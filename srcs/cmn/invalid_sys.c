#include "common.h"

bool invalid_read(int fd, void *ptr, size_t len)
{
    return (read(fd, ptr, len) != (ssize_t)len);
}

bool invalid_write(int fd, void *ptr, size_t len)
{
    return (write(fd, ptr, len) != (ssize_t)len);
}
