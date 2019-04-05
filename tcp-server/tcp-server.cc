#include "tcp-dist-server.h"

int main (int argc, char **argv) {
    TcpDistributionServer server;
    server.Start();
    server.JoinServerThread();

    return 0;
}