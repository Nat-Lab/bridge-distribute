#ifndef SWITCH_H
#define SWITCH_H

#include <net/ethernet.h>
#ifdef __GLIBC__
#include <netinet/ether.h>
#endif
#include <stdio.h>

#include <vector>
#include <map>
#include <algorithm>
#include <shared_mutex>

#include "fdb-entry.h"

class Switch {
public:
    void FdRegister (uint8_t context, int fd);
    void FdUnregister (uint8_t context, int fd);
    void DoPortIn (uint8_t context, int fd, const struct ether_header *eth);
    std::vector<int> GetOutPorts (uint8_t context, const struct ether_header *eth);

private:
    std::vector<int>& GetFdsByContext (uint8_t context);
    std::vector<FdbEntry>& GetFdbByContext (uint8_t context);
    std::map<uint8_t, std::vector<int>> fds_map;
    std::map<uint8_t, std::vector<FdbEntry>> fdb_map;
    std::shared_mutex mtx_fds;
    std::shared_mutex mtx_fdb;
};

#endif // SWITCH_H