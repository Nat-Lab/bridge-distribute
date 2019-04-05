#ifndef DIST_H
#define DIST_H
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#define MAX_PAYLOAD 65536

typedef struct payload_t {
    uint16_t payload_len;
    uint8_t payload[MAX_PAYLOAD];
} payload_t;

typedef struct handshake_msg_t {
    uint8_t id;
} handshake_msg_t;

typedef struct client_t {
    uint8_t id;
    int fd;
} client_t;

class Distributor {
public:
    std::thread AddClient(uint8_t id, int client_fd);
    bool RemoveClient(int client_fd);
    const std::vector<client_t>& GetClients();
private:
    void ClientHandler(const client_t &client);
    void DoDistribute(int fd_src, uint8_t id, const uint8_t *buffer, size_t len);
    std::vector<client_t> clients;
    std::mutex mtx;
};

#endif // DIST_H