#include "proactor.hpp"
#include <iostream>
#include <algorithm>
#include <sys/socket.h>
#include <unistd.h>

namespace designPattern
{
    proactorFunc globalHandlerFunc = nullptr;
    void *pollThread(Reactor *r)
    {
        while (r->running)
        {
            int p = poll(r->fds.data(), r->fds.size(), 1000);
            if (p < 0)
            {
                std::cerr << "Poll error" << std::endl;
                continue;
            }
            // בודק אילו file descriptors מוכנים לקריאה
            for (auto &pfd : r->fds)
            {
                if (pfd.revents & POLLIN)
                {
                    auto handler = r->handlers[pfd.fd];
                    if (handler)
                    {
                        handler(pfd.fd);
                    }
                }
            }
        }
        return nullptr;
    }

    void *startReactor()
    {
        Reactor *r = new Reactor();
        r->running = true;
        r->poll_thread = std::thread(pollThread, r);
        return static_cast<void *>(r);
    }

    int addFdToReactor(void *reactor, int fd, reactorFunc func)
    {
        if (!reactor)
            return -1;
        Reactor *r = static_cast<Reactor *>(reactor);
        // הוספת fd חדש למערך ה-poll
        pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;
        pfd.revents = 0;
        r->fds.push_back(pfd);
        r->handlers[fd] = func;
        return 0;
    }

    int removeFdFromReactor(void *reactor, int fd)
    {
        if (!reactor)
            return -1;
        Reactor *r = static_cast<Reactor *>(reactor);
        // מחיקת ה-fd מהמערך
        auto it = std::find_if(r->fds.begin(), r->fds.end(),
                               [fd](const pollfd &pfd)
                               { return pfd.fd == fd; });
        if (it != r->fds.end())
        {
            r->fds.erase(it);
            r->handlers.erase(fd);
            return 0;
        }
        return -1;
    }

    int stopReactor(void *reactor)
    {
        if (!reactor)
            return -1;
        Reactor *r = static_cast<Reactor *>(reactor);
        r->running = false;
        if (r->poll_thread.joinable())
        {
            r->poll_thread.join(); // main thread waits until t finishes
        }
        delete r;
        return 0;
    }

    // proactor
    std::mutex graphMutex;
    std::unordered_map<pthread_t, ProactorData *> proactors;
    std::mutex proactorsMapMutex;

    void *clientHandlerWrapper(void *arg)
    {
        int client_sock = *(int *)arg;
        delete (int *)arg;

        globalHandlerFunc(client_sock);
        close(client_sock);
        return nullptr;
    }

    void *proactorThread(void *arg)
    {
        ProactorData *data = static_cast<ProactorData *>(arg);

        while (data->running)
        {
            int client_sock = accept(data->sockfd, nullptr, nullptr);
            if (client_sock < 0)
            {
                continue;
            }

            int *sock_ptr = new int(client_sock);
            pthread_t client_tid;
            globalHandlerFunc = data->threadFunc; // זמני, לשם הפשטות

            if (pthread_create(&client_tid, nullptr, clientHandlerWrapper, sock_ptr) != 0)
            {
                std::cerr << "Failed to create client thread\n";
                close(client_sock);
                delete sock_ptr;
            }
            else
            {
                pthread_detach(client_tid); // אין צורך ב־join
            }
        }
        return nullptr;
    }


    pthread_t startProactor(int sockfd, proactorFunc threadFunc)
    {
        pthread_t tid;
        ProactorData *data = new ProactorData{
            sockfd,
            threadFunc,
            &graphMutex,
            true};

        if (pthread_create(&tid, nullptr, proactorThread, data) != 0)
        {
            delete data;
            return 0;
        }

        std::lock_guard<std::mutex> lock(proactorsMapMutex);
        proactors[tid] = data;
        return tid;
    }

    int stopProactor(pthread_t tid)
    {
        ProactorData *data = nullptr;
        {
            std::lock_guard<std::mutex> lock(proactorsMapMutex);
            auto it = proactors.find(tid);
            if (it == proactors.end())
                return -1;
            data = it->second;
            proactors.erase(it);
        }

        data->running = false;
        pthread_join(tid, nullptr);
        delete data;
        return 0;
    }

}
