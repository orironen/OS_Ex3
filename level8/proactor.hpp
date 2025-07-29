#include <thread>
#include <unordered_map>
#include <vector>
#include <poll.h>
#include <pthread.h>
#include <mutex>

namespace designPattern
{
    typedef void *(*reactorFunc)(int fd);
    struct Reactor
    {
        std::vector<pollfd> fds;
        std::unordered_map<int, reactorFunc> handlers;
        bool running;
        std::thread poll_thread;
    };
    typedef void reactor;
    // starts new reactor and returns pointer to it
    void *startReactor();
    // adds fd to Reactor (for reading) ; returns 0 on success.
    int addFdToReactor(void *reactor, int fd, reactorFunc func);
    // removes fd from reactor
    int removeFdFromReactor(void *reactor, int fd);
    // stops reactor
    int stopReactor(void *reactor);

    // proactor
    typedef void *(*proactorFunc)(int sockfd);
    struct ProactorData
    {
        int sockfd;
        proactorFunc threadFunc;
        std::mutex *graphMutex;
        bool running;
    };
    // Global mutex for graph protection
    extern std::mutex graphMutex;
    // starts new proactor and returns proactor thread id
    pthread_t startProactor(int sockfd, proactorFunc threadFunc);
    // stops proactor by threadid
    int stopProactor(pthread_t tid);
}
