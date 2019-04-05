#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <thread>

#include "../dist-types.h"

bool run = true;

void reader (int fd) {
    payload_t pl;
    
    while (run) {
        ssize_t len = read(fd, &pl, sizeof(payload_t));
        if (len <= 0) {
            fprintf(stderr, "[WARN] lost connection to server.\n");
            run = false;
            break;
        }
        if (len != pl.payload_len + 2) {
            fprintf(stderr, "[WARN] got malfromed packet (len = %li, plen = %d)\n", len, pl.payload_len);
            continue;
        }
        char content[pl.payload_len];
        memcpy(content, pl.payload, pl.payload_len);
        fprintf(stderr, "%s", content);
    }
}

int main (int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "usage: %s <server> <port> <channel>\n", argv[0]);
        return 1;
    }

    struct sockaddr_in local_addr;
    struct sockaddr_in server_addr;

    memset(&local_addr, 0, sizeof(struct sockaddr_in));
    memset(&server_addr, 0, sizeof(struct sockaddr_in));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) < 0) { 
        fprintf(stderr, "[CRIT] inet_pton(): %s.\n", strerror(errno));
        return 1; 
    } 

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        fprintf(stderr, "[CRIT] socket(): %s.\n", strerror(errno));
        return 1;
    }

    if (connect(fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in)) < 0) {
        fprintf(stderr, "[CRIT] connect(): %s.\n", strerror(errno));
        return 1; 
    }

    handshake_msg_t hs;
    hs.id = atoi(argv[3]);
    write(fd, &hs, sizeof(handshake_msg_t));
    
    std::thread reader_thread (&reader, fd);
    reader_thread.detach();

    char *data = (char *) malloc(4096);

    payload_t pl;

    fprintf(stderr, "[INFO] connected to %s:%s at channel #%s.\n", argv[1], argv[2], argv[3]);

    while (run) {
        fgets(data, 4096, stdin);
        ssize_t len = strlen(data) + 1;
        memcpy(pl.payload, data, len);
        pl.payload_len = len;
        write(fd, &pl, len + 2);
    }

    close(fd);

    return 0;
}