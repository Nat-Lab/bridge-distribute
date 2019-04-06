#ifndef FD_READ_H
#define FD_READ_H

#include <unistd.h>

class FdReader {
public:
    virtual ssize_t Read (int fd, void *buf, size_t len);
    virtual ~FdReader ();
};

#endif // FD_READ_H