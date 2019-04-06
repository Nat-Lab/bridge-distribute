#include "distributor.h"

std::thread Distributor::AddClient(uint8_t id, int client_fd) {
    client_t new_client;
    new_client.id = id;
    new_client.fd = client_fd;
    mtx.lock();
    clients.push_back(new_client);
    mtx.unlock();
    return std::thread (&Distributor::ClientHandler, this, new_client);
}

bool Distributor::RemoveClient(int client_fd) {
    mtx.lock();
    for (auto client = clients.begin(); client != clients.end(); client++) {
        if (client->fd == client_fd) {
            clients.erase(client);
            mtx.unlock();
            return true;
        }
    }
    mtx.unlock();
    return false;
}

const std::vector<client_t>& Distributor::GetClients() {
    return clients;
}

void Distributor::ClientHandler(const client_t &client) {
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
        if (len < 2) {
            fprintf(stderr, "[WARN] Distributor::ClientHandler: invalid packet from fd %d (packet too small).\n", fd);
            continue;
        }

        if (payload.payload_len != len - 2) {
            fprintf(stderr, "[WARN] Distributor::ClientHandler: invalid packet from fd %d (malformed packet: payload_len=%d, but pkt_len=%li).\n", fd, payload.payload_len, len);
            continue;
        }

        DoDistribute(fd, id, (uint8_t *) &payload, len);
    }

    if(RemoveClient(fd)) {
        fprintf(stderr, "[WARN] Distributor::ClientHandler: closing fd %d.\n", fd);
        close(fd);
    } else {
        fprintf(stderr, "[CRIT] Distributor::ClientHandler: failed to remove client with fd %d, something is not right.\n", fd);
        std::terminate();
    }
}

void Distributor::DoDistribute(int fd_src, uint8_t id, const uint8_t *buffer, size_t len) {
    // FIXME: mutex?
    for (auto &client : clients) {
        if (client.id == id && fd_src != client.fd) {
            ssize_t ret = write(client.fd, buffer, len);
            if (ret < 0)
                fprintf(stderr, "[WARN] Distributor::DoDistribute: error writing to fd %d: %s.\n", client.fd, strerror(errno));
            if ((size_t) ret != len)
                fprintf(stderr, "[WARN] Distributor::DoDistribute: error writing to fd %d: len (%li) != wrote (%lu).\n", client.fd, len, ret);
        }
    }
}

void Distributor::SetReader (FdReader reader) {
    this->reader = reader;
}