#ifndef FDB_ENTRY_H
#define FDB_ENTRY_H

#include <net/ethernet.h>
#include <string.h>

class FdbEntry {
public:
    FdbEntry (int fd, const struct ether_addr *addr);
    bool DestinationIs (const struct ether_addr *addr) const;
    int GetFd (void) const;
    const struct ether_addr* GetAddr (void) const;
    bool operator== (const FdbEntry &other) const;

private:
    int fd;
    struct ether_addr addr;
};

#endif // FDB_ENTRY_H 