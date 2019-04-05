#ifndef DIST_S_H
#define DIST_S_H

#include "distributor.h"
#include <thread>
#include <vector>

class DistributionServer {
public:
    DistributionServer();

    void JoinThreads();
    void Start();
    void Stop();
    virtual ~DistributionServer();

protected:
    bool HandleConnect(int fd);

private:
    virtual void DoStart() = 0;
    virtual void DoStop() = 0;
    Distributor dis;
    std::vector<std::thread> threads;
    bool running;
};

#endif // DIST_S_H