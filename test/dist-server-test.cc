#include "dist-server-test.h"
#include <sys/socket.h>
#include <chrono>

int cs1[2];
int cs2[2];
int cs3[2];

void DistributionServerTest::DoStart() {
    socketpair (AF_UNIX, SOCK_DGRAM, 0, cs1);
    socketpair (AF_UNIX, SOCK_DGRAM, 0, cs2);
    socketpair (AF_UNIX, SOCK_DGRAM, 0, cs3);

    HandleConnect(cs1[1]);
    HandleConnect(cs2[1]);
    HandleConnect(cs3[1]);
}

void DistributionServerTest::DoStop() { }

int main () {
    // server init
    DistributionServerTest test;
    std::thread server_thread(&DistributionServerTest::Start, &test);
    printf("[INFO] main: started, wait 1s for server to be ready.\n");
    std::this_thread::sleep_for (std::chrono::seconds(1));

    // handshake
    handshake_msg_t hs;
    hs.id = 114;
    write(cs1[0], &hs, sizeof(handshake_msg_t));
    write(cs2[0], &hs, sizeof(handshake_msg_t));
    write(cs3[0], &hs, sizeof(handshake_msg_t));
    server_thread.join();

    // publish from cs1
    payload_t payload;
    printf("[INFO] main: client0: writing payload = test, len = 4.\n");
    memcpy(payload.payload, "test", 4);
    payload.payload_len = 4;
    ssize_t len = write(cs1[0], &payload, 6);
    printf("[INFO] main: client0: wrote %d bytes.\n", len);

    // cs2: read
    printf("[INFO] main: client1: reading payload...\n");
    payload_t payload_read;
    len = read(cs2[0], &payload_read, sizeof(payload_t));
    printf("[INFO] main: client1: read %d bytes, payload length = %d btyes.\n", len, payload_read.payload_len);

    if (payload_read.payload_len + 2 != len) {
        printf("[WARN] main: client1: error, payload length should be %d bytes.\n", len - 2);
    } else {
        char content[payload_read.payload_len];
        memcpy(&content, payload_read.payload, payload_read.payload_len);
        printf("[INFO] main: client1: payload content: '%s'\n", content);
    }

    // cs3: read
    printf("[INFO] main: client2: reading payload...\n");
    payload_t payload_read_2;
    len = read(cs3[0], &payload_read_2, sizeof(payload_t));
    printf("[INFO] main: client2: read %d bytes, payload length = %d btyes.\n", len, payload_read_2.payload_len);

    if (payload_read_2.payload_len + 2 != len) {
        printf("[WARN] main: client2: error, payload length should be %d bytes.\n", len - 2);
    } else {
        char content[payload_read_2.payload_len];
        memcpy(&content, payload_read_2.payload, payload_read_2.payload_len);
        printf("[INFO] main: client2: payload content: '%s'\n", content);
    }

    // stop
    test.Stop();

    return 0;
}