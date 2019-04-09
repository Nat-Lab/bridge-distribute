#include "switch.h"

std::vector<int>& Switch::GetFdsByContext (uint8_t context) {
    mtx_fds.lock();
    if (!fds_map.count(context)) {
        fprintf(stderr, "[INFO] Switch::GetFdsByContext: fds for context %d does not exist, create.\n", context);
        fds_map.insert({context, std::vector<int> ()});
    }
    mtx_fds.unlock();
    return fds_map[context];
}

std::vector<FdbEntry>& Switch::GetFdbByContext (uint8_t context) {
    mtx_fdb.lock();
    if (!fdb_map.count(context)) {
        fprintf(stderr, "[INFO] Switch::GetFdbByContext: fdb for context %d does not exist, create.\n", context);
        fdb_map.insert({context, std::vector<FdbEntry> ()});
    }
    mtx_fdb.unlock();
    return fdb_map[context];
}

void Switch::FdRegister (uint8_t context, int fd) {
    auto &fds = GetFdsByContext(context);

    mtx_fds.lock();
    if (std::find(fds.begin(), fds.end(), fd) == fds.end()) {
        fds.push_back(fd);
        fprintf(stderr, "[INFO] Switch::FdRegister: registered: fd %d @ context %d.\n", fd, context);
    }
    mtx_fds.unlock();
}

void Switch::FdUnregister (uint8_t context, int fd) {
    fprintf(stderr, "[INFO] Switch::FdRegister: unregistering: fd %d @ context %d.\n", fd, context);
    auto &fds = GetFdsByContext(context);
    auto &fdb = GetFdbByContext(context);

    mtx_fds.lock();
    auto fds_it = fds.begin();
    while (fds_it != fds.end()) {
        if(*fds_it == fd) {
            fds.erase (fds_it);
        } else fds_it++;
    } 
    mtx_fds.unlock();

    mtx_fdb.lock();
    auto fdb_it = fdb.begin();
    while (fdb_it != fdb.end()) {
        if(fdb_it->GetFd() == fd) {
            auto addr_str = ether_ntoa(fdb_it->GetAddr());
            fprintf(stderr, "[INFO] Switch::FdUnregister: remove fdb: %s @ fd %d (context %d).\n", addr_str, fd, context);
            fdb.erase (fdb_it);
        } else fdb_it++;
    } 
    mtx_fdb.unlock();

    fprintf(stderr, "[INFO] Switch::FdUnregister: unregistered: fd %d @ context %d.\n", fd, context);
}

void Switch::DoPortIn (uint8_t context, int fd, const struct ether_header *eth) {
    auto &fdb = GetFdbByContext(context);

    FdbEntry new_entry (fd, (struct ether_addr *) eth->ether_shost);
    for (auto &fdb_entry : fdb) {
        if (fdb_entry == new_entry) return;
    }

    mtx_fdb.lock();
    fdb.push_back(new_entry);
    mtx_fdb.unlock();

    auto addr_str = ether_ntoa((struct ether_addr *) eth->ether_shost);
    fprintf(stderr, "[INFO] Switch::DoPortIn: learned: %s -> fd %d (context %d).\n", addr_str, fd, context);
}

std::vector<int> Switch::GetOutPorts (uint8_t context, const struct ether_header *eth) {
    const uint16_t *ea = (const uint16_t *) eth->ether_dhost;
    if (ea[0] == 0xFFFF && ea[1] == 0xFFFF && ea[2] == 0xFFFF)
        return GetFdsByContext(context);

    if (!fdb_map.count(context)) return std::vector<int> ();

    auto &fdb = GetFdbByContext(context);
    std::vector<int> ports;

    mtx_fdb.lock_shared();
    for (auto &fdb_entry : fdb) {
        if (fdb_entry.DestinationIs((struct ether_addr *) eth->ether_dhost))
            ports.push_back(fdb_entry.GetFd());
            
    }
    mtx_fdb.unlock_shared();

    return ports;
}