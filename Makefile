CFLAGS = -O3 -std=c23 -Wall -Wextra -Wconversion -I./

ifeq ($(PLATFORM),linux)
	CFLAGS += -DPOSIX -DPAGED
	KERNEL_PATH = platform/linux/kernel.c
else
$(warning PLATFORM not provided or unimplemented, provide POSIX/PAGED defines and KERNEL_PATH to kernel specific implementation file by hand. For non-posix paged systems PAGE_SIZE also has to be defined)
endif

ifndef KERNEL_PATH
$(error KERNEL_PATH not provided)
endif

ifdef PAGED
	CFLAGS += -DPAGED

	ifndef POSIX
		ifndef PAGE_SIZE
$(error PAGE_SIZE not provided)
		else
			CFLAGS += -DPAGE_SIZE=$(PAGE_SIZE)
		endif
	endif
endif

ifdef POSIX
	CFLAGS += -DPOSIX
endif

ifdef DEBUG
	CFLAGS += -DDEBUG_PRINT_EXTRA -g
endif

test_alloc:
	gcc -o test *.c $(KERNEL_PATH) $(CFLAGS) -DALLOCATOR_TEST

