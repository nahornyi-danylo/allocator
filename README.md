# General purpose allocator
A general purpose platform agnostic tagged heap allocator, using best-fit strategy through an intrusive rbtree.
## Building
For now, only a linux build is available. To build with a randomized tester use
```
make PLATFORM=linux
```
It is also possible to compile for other platforms by adding a kernel.c for the specific platform, fulfilling the kernel.h API.
The makefile allows for this by providing an ability to enter the needed data manually. For example a build for windows would be something like
```
make PAGED=1 PAGE_SIZE=4096 KERNEL_PATH=platform/windows/kernel.c
```
On POSIX systems PAGE_SIZE is retrieved via sysconf(_SC_PAGESIZE).

Default arena size is defined as 4 pages on paged systems, or 4096 for non-paged.
Of course, if possible, it's better to edit config.h and Makefile to add a new platform. Other then that the core of the allocator is fully platform independent.

The current platform/linux/kernel.c is not fully POSIX compliant, because it uses MAP_ANONYMOUS for mmap, and MADV_FREE for madvise.

It is also possible to add DEBUG=1 to the make command which will make the program print a lot of debug information per each iteration, enough to find most bugs. 
