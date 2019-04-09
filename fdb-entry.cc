#include "fdb-entry.h"

FdbEntry::FdbEntry (int fd, const struct ether_addr *addr) {
    this->fd = fd;
    memcpy(&(this->addr), addr, sizeof(struct ether_addr));
}

bool FdbEntry::DestinationIs (const struct ether_addr *addr) const {
    return memcmp(addr, &(this->addr), sizeof(struct ether_addr)) == 0;
}

int FdbEntry::GetFd (void) const {
    return fd;
}

const struct ether_addr* FdbEntry::GetAddr (void) const {
	return &addr;
}

bool FdbEntry::operator== (const FdbEntry &other) const {
    return fd == other.fd && DestinationIs(&(other.addr));
}