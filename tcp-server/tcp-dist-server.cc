#include "tcp-dist-server.h"

TcpDistributionServer::TcpDistributionServer() : DistributionServer() {
    memset(&local_addr, 0, sizeof(struct sockaddr_in));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY; 
    local_addr.sin_port = htons(DEFAULT_PORT);
    running = false;
    queue_len = 16;
    SetDistributorMode (DistributorMode::SWITCH);
}

int TcpDistributionServer::SetBindAddress(const char *addr, in_port_t port) {
    local_addr.sin_port = htons(port);
    return inet_pton(AF_INET, addr, &(local_addr.sin_addr));
}

void TcpDistributionServer::SetQueueLength(int len) {
    queue_len = len;
}

void TcpDistributionServer::JoinServerThread () {
    if (server_thread.joinable()) server_thread.join();
}

void TcpDistributionServer::DoStart() {
    running = true;
    server_thread = std::thread (&TcpDistributionServer::TcpListener, this);
}

void TcpDistributionServer::DoStop() {
    running = false;
}

void TcpDistributionServer::TcpListener() {
    char ip_str[INET_ADDRSTRLEN];

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int nodelay = 1;
    struct sockaddr_in remote_addr;
    socklen_t remote_addr_len = sizeof(struct sockaddr_in);

    if (fd < 0) {
        fprintf(stderr, "[CRIT] TcpDistributionServer::TcpListener: socket(): %s.\n", strerror(errno));
        Stop();
        goto listener_exit;
    }
    
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(int)) < 0) {
        fprintf(stderr, "[CRIT] TcpDistributionServer::TcpListener: setsockopt(): %s.\n", strerror(errno));

        Stop();
        goto listener_exit;
    }


    if (bind(fd, (struct sockaddr *) &local_addr, sizeof(struct sockaddr_in)) < 0) {
        fprintf(stderr, "[CRIT] TcpDistributionServer::TcpListener: bind(): %s.\n", strerror(errno));
        Stop();
        goto listener_exit;
    }

    if (listen(fd, queue_len) < 0) {
        fprintf(stderr, "[CRIT] TcpDistributionServer::TcpListener: listen(): %s.\n", strerror(errno));
        Stop();
        goto listener_exit;
    }

    inet_ntop(AF_INET, &(local_addr.sin_addr), ip_str, INET_ADDRSTRLEN);

    fprintf(stderr, "[INFO] TcpDistributionServer::TcpListener: listening %s:%d.\n", ip_str, ntohs(local_addr.sin_port));

    while (running) {
        int client_fd = accept(fd, (struct sockaddr *) &remote_addr, &remote_addr_len);
        if (client_fd < 0) {
            fprintf(stderr, "[CRIT] TcpDistributionServer::TcpListener: accept(): %s.\n", strerror(errno));
            Stop();
            goto listener_exit;
        }
        inet_ntop(AF_INET, &(remote_addr.sin_addr), ip_str, INET_ADDRSTRLEN);
        fprintf(stderr, "[INFO] TcpDistributionServer::TcpListener: new connection: src: %s:%d (fd = %d).\n", ip_str, ntohs(remote_addr.sin_port), client_fd);

        std::thread(&TcpDistributionServer::TcpAcceptHandler, this, client_fd).detach();
    }

listener_exit:
    fprintf(stderr, "[INFO] TcpDistributionServer::TcpListener: now exiting.\n");

}

void TcpDistributionServer::TcpAcceptHandler (int fd) {
    if(!HandleConnect(fd))
        close(fd);
}

ssize_t TcpDistributionServer::HandleRead (int fd, void *buf, size_t len) {
    // TODO buffer if not in STREAM mode.
    return read(fd, buf, len);
}