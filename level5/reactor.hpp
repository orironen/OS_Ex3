namespace reactor {
    typedef void * (* reactorFunc)(int fd);
    typedef void reactor;
    // starts new reactor and returns pointer to it
    void * startReactor();
    // adds fd to Reactor (for reading) ; returns 0 on success.
    int addFdToReactor(void * reactor, int fd, reactorFunc func); int addFdToReactor(void * reactor, int fd, reactorFunc func);
    // removes fd from reactor
    int removeFdFromReactor(void * reactor, int fd);
    // stops reactor
    int stopReactor(void * reactor);
}
