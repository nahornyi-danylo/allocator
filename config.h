
#ifdef PAGED

#ifdef POSIX

#include <unistd.h>
#define PAGE_SIZE (unsigned)sysconf(_SC_PAGESIZE)

#endif

#define DEFAULT_ARENA_SIZE (4 * PAGE_SIZE)
#define TO_PAGE_ALIGNED(s) (((s) + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1))

#else

#define DEFAULT_ARENA_SIZE 4096u

#endif

#define TO_DEFAULT_ARENA_ALIGNED(s) (((s) + (DEFAULT_ARENA_SIZE - 1)) & ~(DEFAULT_ARENA_SIZE - 1))

#define MAX_SINGLE_ARENA_ALLOCATION_SIZE (DEFAULT_ARENA_SIZE - ALIGNED_BLOCK_SIZE)
