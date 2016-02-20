#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <iostream>

#include "ion_wrapper.hpp"

int ion_free(IonBuffer& ionBuf)
{
    int ion_device_fd = -1, rc = 0;

    ion_device_fd = open("/dev/ion", O_RDONLY);

    if (ion_device_fd < 0) {
        ERR("ERROR: ION Device open() Failed %d", ion_device_fd);
        return -2;
    }

    munmap(ionBuf.vaddr, ionBuf.alloc_data.len);

    // struct ion_handle_data data;
    // data.handle = ionBuf.alloc_data.handle;
    // rc = ioctl(ion_device_fd, ION_IOC_FREE, &data);

    close(ion_device_fd);

    return rc;
}

int ion_allocate(int size, IonBuffer& ionBuf)
{
    int ion_device_fd = -1, rc = 0;

    if ((size <= 0)) {
        ERR("Invalid input to alloc_map_ion_memory");
        rc = -1;
        goto ret;
    }

    ion_device_fd = open("/dev/ion", O_RDONLY);

    if (ion_device_fd < 0) {
        ERR("ERROR: ION Device open() Failed");
        rc = -2;
        goto ret;
    }

    /* to make it page size aligned */
    ionBuf.alloc_data.len       = (size + 4095) & (~4095);
    ionBuf.alloc_data.align     = 4096;
    ionBuf.alloc_data.flags     = ION_FLAG_CACHED;
    ionBuf.alloc_data.heap_mask = 1 << ION_IOMMU_HEAP_ID;
    INFO("ION ALLOC unsec buf: size %d align %d flags %x",
         (int)ionBuf.alloc_data.len,
         (int)ionBuf.alloc_data.align, ionBuf.alloc_data.flags);

    rc = ioctl(ion_device_fd, ION_IOC_ALLOC, &ionBuf.alloc_data);

    if (rc || !ionBuf.alloc_data.handle) {
        ERR("ION ALLOC memory failed 0x%x", rc);
        ionBuf.alloc_data.handle = NULL;
        rc                       = -3;
        goto close_ion;
    }
    ionBuf.fd_data.handle = ionBuf.alloc_data.handle;
    rc                    = ioctl(ion_device_fd, ION_IOC_SHARE, &ionBuf.fd_data);

    if (rc) {
        ERR("ION SHARE failed");
        rc = -4;
        goto close_ion;
    }

    ionBuf.vaddr = mmap(NULL, ionBuf.alloc_data.len, PROT_READ | PROT_WRITE,
                        MAP_SHARED, ionBuf.fd_data.fd, 0);

    if (ionBuf.vaddr == MAP_FAILED) {
        ERR("mmap failed for ion!");
        rc = -5;
        goto close_ion;
    }
close_ion:
    close(ion_device_fd);
ret:
    return rc;
}
