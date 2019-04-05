#include "dist-server.h"

DistributionServer::DistributionServer() {
    running = false;
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
    uint8_t *buffer = (uint8_t *) malloc(4096);
    ssize_t len = read(fd, buffer, 4096);

    if (len < 0) {
        fprintf(stderr, "[WARN] DistributionServer::HandleConnect: error reading from fd %d: %s.\n", fd, strerror(errno));
        delete buffer;
        return false;
    }

    if (len != sizeof(handshake_msg_t)) {
        fprintf(stderr, "[WARN] DistributionServer::HandleConnect: malformed packet from fd %d.\n", fd);
        delete buffer;
        return false;
    }

    handshake_msg_t *handshake = (handshake_msg_t *) buffer;
    fprintf(stderr, "[INFO] DistributionServer::HandleConnect: fd %d joined channel #%d.\n", fd, handshake->id);
    threads.push_back(dis.AddClient(handshake->id, fd));
    delete buffer;
    return true;
}

DistributionServer::~DistributionServer() {}