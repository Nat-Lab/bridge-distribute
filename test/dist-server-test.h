#include "../distributor.h"
#include "../dist-server.h"

class DistributionServerTest : public DistributionServer {
public:
    virtual ~DistributionServerTest() {};
private:
    void DoStart();
    void DoStop();
};