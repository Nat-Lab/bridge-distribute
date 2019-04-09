#include "dist-server.h"

DistributionServer::DistributionServer() {
    running = false;
    dis.SetReader(FdReader<DistributionServer>(&DistributionServer::HandleRead, this));
}

void DistributionServer::Start () {
    if (!running) {
        DoStart();
        running = true;
    }
}

void DistributionServer::Stop () {
    DoStop();
    for (auto &thread : threads) {
        thread.detach(); 
        thread.~thread();
    } // TODO kill threads
    running = false;
}

void DistributionServer::JoinThreads () {
    for (auto &thread : threads) {
        if (thread.joinable()) thread.join();
    }
}

bool DistributionServer::HandleConnect(int fd) {
    handshake_msg_t hs;
    ssize_t len = read(fd, &hs, sizeof(handshake_msg_t));

    if (len < 0) {
        fprintf(stderr, "[WARN] DistributionServer::HandleConnect: error reading from fd %d: %s.\n", fd, strerror(errno));
        return false;
    }

    if (len != sizeof(handshake_msg_t)) {
        fprintf(stderr, "[WARN] DistributionServer::HandleConnect: malformed packet from fd %d.\n", fd);
        return false;
    }

    fprintf(stderr, "[INFO] DistributionServer::HandleConnect: fd %d joined channel #%d.\n", fd, hs.id);
    threads.push_back(dis.AddClient(hs.id, fd));
    return true;
}

void DistributionServer::SetDistributorMode (DistributorMode mode) {
    dis.SetMode(mode);
}

DistributionServer::~DistributionServer() {}

ssize_t DistributionServer::HandleRead (int fd, void *buf, size_t len) {
    return read (fd, buf, len);
}