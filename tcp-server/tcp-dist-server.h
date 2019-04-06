#include "../distributor.h"
#include "../dist-server.h"
#define DEFAULT_PORT 2672 // 2672 = CSMA

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <thread>
#include <errno.h>
#include <string.h>

class TcpDistributionServer : public DistributionServer {
public:
    TcpDistributionServer();

    virtual ~TcpDistributionServer() {};
    int SetBindAddress(const char *addr, in_port_t port);
    void SetQueueLength (int len);
    void JoinServerThread();
private:
    void DoStart();
    void DoStop();
    void TcpListener();
    void TcpAcceptHandler(int fd);
    ssize_t HandleRead (int fd, void *buf, size_t len);

    std::thread server_thread;
    struct sockaddr_in local_addr;
    bool running;
    int queue_len;
};