#include "reactor.hpp"
#include <iostream>
#include <algorithm>

namespace reactor {
    void *pollThread(Reactor *r)
    {
        while (r->running)
        {
            int p = poll(r->fds.data(), r->fds.size(), 1000);//מחזיר כמות הפעמים שה-fd מוכן לקריאה
            if (p < 0)
            {
                std::cerr << "Poll error" << std::endl;
                continue;
            }
            // בודק אילו file descriptors מוכנים לקריאה
            for (auto &pfd : r->fds)
            {
                if (pfd.revents & POLLIN)//אם ה-fd מוכן לקריאה 
                {
                    auto handler = r->handlers[pfd.fd];
                    if (handler)
                    {
                        handler(pfd.fd);//מפעילים את הפונקציה המתאימה מהמפה
                    }
                }
            }
        }
        return nullptr;
    }

    void* startReactor()
    {
        Reactor *r = new Reactor();
        r->running = true;
        r->poll_thread = std::thread(pollThread, r);//מפעילים תרד חדש
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
        //מוסיפים את ה-fd למערך ולמפה
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
        //צריך להמיר ממצביע כללי לreactor
        Reactor *r = static_cast<Reactor *>(reactor);
        r->running = false;
        if (r->poll_thread.joinable())
        {
            r->poll_thread.join(); // main thread waits until t finishes
        }
        delete r;
        return 0;
    }
}
