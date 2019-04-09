#ifndef FD_READ_H
#define FD_READ_H

#include <unistd.h>

template <typename T>
class FdReader {
public:
    typedef ssize_t (T::* ReadHandler)(int, void*, size_t);

    FdReader ();
    FdReader (ReadHandler handler, T *handler_object);
    ssize_t Read (int fd, void *buf, size_t len);
    void SetReaderHandler (ReadHandler handler, T *handler_object);

private:
    ReadHandler handler;
    T *handler_object;
};

template <typename T>
FdReader<T>::FdReader () {
    handler_object = 0;
    handler = 0;
}

template <typename T>
FdReader<T>::FdReader (ReadHandler handler, T *handler_object) {
    this->handler = handler;
    this->handler_object = handler_object;
}

template <typename T>
ssize_t FdReader<T>::Read (int fd, void *buf, size_t len) {
    if (handler == 0 || handler_object == 0) return read (fd, buf, len);
    return (handler_object->*handler)(fd, buf, len);
}

template <typename T>
void FdReader<T>::SetReaderHandler (ReadHandler handler, T *handler_object) {
    this->handler = handler;
    this->handler_object = handler_object;
}

#endif // FD_READ_H