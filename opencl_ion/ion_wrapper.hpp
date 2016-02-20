#ifndef __ION_WRAPPER_HPP__
#define __ION_WRAPPER_HPP__

#include <stdlib.h>
#include <linux/msm_ion.h>
#include <syslog.h>

#define ERR(fmt, args ...)                                                \
    do {                                                                  \
        syslog(LOG_ERR, "E %d, " fmt, __LINE__, ## args); \
    } while (0)

#define INFO(fmt, args ...)                                                \
    do {                                                                   \
        syslog(LOG_INFO, "I %d, " fmt, __LINE__, ## args); \
    } while (0)

struct IonBuffer {
    int ion_device_fd;
    struct ion_fd_data fd_data;
    struct ion_allocation_data alloc_data;
    void *vaddr;
};

int ion_allocate(int size, IonBuffer &ionBuf);

int ion_free(IonBuffer &ionBuf);

#endif // ifndef __ION_WRAPPER_HPP__
