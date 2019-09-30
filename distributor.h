#ifndef DIST_H
#define DIST_H

#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <thread>
#include <shared_mutex>
#include <algorithm>

#include "dist-types.h"
#include "fd-reader.h"
#include "switch.h"

typedef struct client_t {
    uint8_t id;
    int fd;
} client_t;

typedef enum DistributorMode {
    STREAM,
    HUB,
    SWITCH
} DistributorMode;

template <typename T>
class Distributor {
public:
    Distributor (DistributorMode mode);
    Distributor ();
    std::thread AddClient(uint8_t id, int client_fd);
    bool RemoveClient(int client_fd);
    const std::vector<client_t>& GetClients();
    void SetReader (FdReader<T> reader);
    void SetMode (DistributorMode mode);
private:
    void ClientHandler(const client_t &client);
    int DoDistribute(int fd_src, uint8_t id, const uint8_t *buffer, size_t len);
    std::vector<client_t> clients;
    std::shared_mutex mtx;
    FdReader<T> reader;
    DistributorMode mode;
    Switch m_switch;
};

template <typename T>
Distributor<T>::Distributor (DistributorMode mode) {
    this->mode = mode;
}

template <typename T>
Distributor<T>::Distributor () {
    mode = DistributorMode::STREAM;
}

template <typename T>
std::thread Distributor<T>::AddClient(uint8_t id, int client_fd) {
    if (mode == DistributorMode::SWITCH)
        m_switch.FdRegister (id, client_fd);
    client_t new_client;
    new_client.id = id;
    new_client.fd = client_fd;
    mtx.lock();
    clients.push_back(new_client);
    mtx.unlock();
    return std::thread (&Distributor::ClientHandler, this, new_client);
}

template <typename T>
bool Distributor<T>::RemoveClient(int client_fd) {
    mtx.lock();
    for (auto client = clients.begin(); client != clients.end(); client++) {
        if (client->fd == client_fd) {
            if (mode == DistributorMode::SWITCH)
                m_switch.FdUnregister (client->id, client_fd);
            clients.erase(client);
            mtx.unlock();
            return true;
        }
    }
    mtx.unlock();
    return false;
}

template <typename T>
const std::vector<client_t>& Distributor<T>::GetClients() {
    return clients;
}

template <typename T>
void Distributor<T>::ClientHandler(const client_t &client) {
    int fd = client.fd;
    uint8_t id = client.id;
    payload_t payload;

    while (1) {
        ssize_t len = reader.Read(fd, &payload, sizeof(payload_t));
        
        if (len < 0) {
            fprintf(stderr, "[WARN] Distributor::ClientHandler: error reading from fd %d: %s, the client will be remove.\n", fd, strerror(errno));
            break;
        }
        
        if (len == 0) {
            fprintf(stderr, "[WARN] Distributor::ClientHandler: read() from fd %d returned 0, the client will be remove.\n", fd);
            break;
        }

        if (mode == DistributorMode::HUB || mode == DistributorMode::SWITCH) {
            if (len < 2) {
                fprintf(stderr, "[WARN] Distributor::ClientHandler: invalid packet from fd %d (packet too small).\n", fd);
                continue;
            }

            if (payload.payload_len != len - 2) {
                fprintf(stderr, "[WARN] Distributor::ClientHandler: invalid packet from fd %d (malformed packet: payload_len=%d, but pkt_len=%li).\n", fd, payload.payload_len, len);
                continue;
            }

            if (payload.payload_len < sizeof(struct ether_header)) {
                fprintf(stderr, "[WARN] Distributor::ClientHandler: invalid packet from fd %d (payload too small).\n", fd);
                continue;
            }

            m_switch.DoPortIn(id, fd, (struct ether_header *) payload.payload);
        }

        if (DoDistribute(fd, id, (uint8_t *) &payload, len) < 0) {
            break;
        }
    }

    if(RemoveClient(fd)) {
        fprintf(stderr, "[WARN] Distributor::ClientHandler: closing fd %d.\n", fd);
        close(fd);
    } else {
        fprintf(stderr, "[CRIT] Distributor::ClientHandler: failed to remove client with fd %d, something is not right.\n", fd);
        std::terminate();
    }
}

template <typename T>
int Distributor<T>::DoDistribute(int fd_src, uint8_t id, const uint8_t *buffer, size_t len) {
    // FIXME: mutex?
    mtx.lock_shared();
    if (mode != DistributorMode::SWITCH) {
        for (auto &client : clients) {
            if (client.id == id && fd_src != client.fd) {
                ssize_t ret = write(client.fd, buffer, len);
                if (ret < 0) {
                    fprintf(stderr, "[WARN] Distributor::DoDistribute: error writing to fd %d: %s.\n", client.fd, strerror(errno));
                    mtx.unlock_shared();
                    return -1;
                }
                if ((size_t) ret != len) {
                    fprintf(stderr, "[WARN] Distributor::DoDistribute: error writing to fd %d: len (%li) != wrote (%lu).\n", client.fd, len, ret);
                    mtx.unlock_shared();
                    return -1;
                }
            }
        }
        mtx.unlock_shared();
        return 0;
    }

    const uint8_t *payload = ((const payload_t *) buffer)->payload;
    std::vector<int> ports = m_switch.GetOutPorts(id, (struct ether_header *) payload);
    for (auto fd : ports) {
        ssize_t ret = write(fd, buffer, len);
        if (ret < 0) {
            fprintf(stderr, "[WARN] Distributor::DoDistribute: error writing to fd %d: %s.\n", fd, strerror(errno));
            mtx.unlock_shared();
            return -1;
        }
        if ((size_t) ret != len) {
            fprintf(stderr, "[WARN] Distributor::DoDistribute: error writing to fd %d: len (%li) != wrote (%lu).\n", fd, len, ret);
            mtx.unlock_shared();
            return -1;
        }
    }
    mtx.unlock_shared();
    return 0;
}

template <typename T>
void Distributor<T>::SetReader (FdReader<T> reader) {
    this->reader = reader;
}

template <typename T>
void Distributor<T>::SetMode (DistributorMode mode) {
    this->mode = mode;
}

#endif // DIST_H