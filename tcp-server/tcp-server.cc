#include "tcp-dist-server.h"
#include <stdio.h>

void dm_to_str (DistributorMode mode, char *buf, size_t len) {
    switch (mode) {
        case DistributorMode::HUB:
            memcpy (buf, "hub", len);
            break;
        case DistributorMode::STREAM:
            memcpy (buf, "stream", len);
            break;
        case DistributorMode::SWITCH:
            memcpy (buf, "switch", len);
            break;
    }
}

void print_help (const char *me) {
    fprintf (stderr, "usage: %s -h\n", me);
    fprintf (stderr, "usage: %s [-b bind] [-p port] [-m mode]\n", me);
    fprintf (stderr, "where: mode := { switch | hub | stream }\n");
}

int main (int argc, char **argv) {
    DistributorMode dm = DistributorMode::SWITCH;

    char bind_addr[INET_ADDRSTRLEN];

    memset(bind_addr, 0, INET_ADDRSTRLEN);
    memcpy(bind_addr, "0.0.0.0", INET_ADDRSTRLEN);

    in_port_t port = 2672;

    char opt;
    while ((opt = getopt(argc, argv, "hb:p:m:")) != -1) {
        switch (opt) {
            case 'h':
                print_help (argv[0]);
                return 0;
            case 'b':
                strncpy (bind_addr, optarg, INET_ADDRSTRLEN);
                break;
            case 'p':
                port = atoi (optarg);
                break;
            case 'm':
                if (strcmp("hub", optarg) == 0) {
                    dm = DistributorMode::HUB;
                    break;
                }
                if (strcmp("stream", optarg) == 0) {
                    dm = DistributorMode::STREAM;
                    break;
                }
                if (strcmp("switch", optarg) == 0){
                    dm = DistributorMode::SWITCH;
                    break;
                }
            default:
                print_help (argv[0]);
                return 1;
        }
    }

    char *mode_str = (char *) malloc(7);
    memset (mode_str, 0, 7);
    dm_to_str (dm, mode_str, 7);
    fprintf (stderr, "[INFO] starting server at %s:%d, mode %s.\n", bind_addr, port, mode_str);
    TcpDistributionServer server;
    server.SetBindAddress(bind_addr, port);
    server.SetDistributorMode(dm);
    server.Start();
    server.JoinServerThread();

    return 0;
}
