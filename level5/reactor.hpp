#include <thread>
#include <vector>
#include <poll.h>

namespace reactor
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
    int addFdToReactor(void *reactor, int fd, reactorFunc func);
    // removes fd from reactor
    int removeFdFromReactor(void *reactor, int fd);
    // stops reactor
    int stopReactor(void *reactor);
}
