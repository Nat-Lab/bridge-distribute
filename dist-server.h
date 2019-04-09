#ifndef DIST_S_H
#define DIST_S_H

#include "distributor.h"
#include "fd-reader.h"
#include <thread>
#include <vector>

class DistributionServer {
public:
    DistributionServer();

    void JoinThreads();
    void Start();
    void Stop();
    virtual ~DistributionServer();
    void SetDistributorMode (DistributorMode mode);

protected:
    bool HandleConnect (int fd);

private:
    virtual ssize_t HandleRead (int fd, void *buf, size_t len);
    virtual void DoStart() = 0;
    virtual void DoStop() = 0;
    Distributor<DistributionServer> dis;
    std::vector<std::thread> threads;
    bool running;
};

#endif // DIST_S_H