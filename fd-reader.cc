#include "fd-reader.h"

FdReader::~FdReader () {}

ssize_t FdReader::Read (int fd, void *buf, size_t len) {
    return read(fd, buf, len);
}