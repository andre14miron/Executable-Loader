/*
 * Loader Implementation
 *
 * 2022, Operating Systems
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "exec_parser.h"

static so_exec_t *exec;
static struct sigaction default_handler;
static int fd;

static void segv_handler(int signum, siginfo_t *info, void *context)
{
	int i;
	int page_size = getpagesize();
	uintptr_t seg_fault_addr = (uintptr_t) info->si_addr;

	// Traversing the segments 
	for (i = 0; i < exec->segments_no; i++) {
		uintptr_t segment_end_addr = exec->segments[i].vaddr + (uintptr_t) exec->segments[i].mem_size;

		if (exec->segments[i].vaddr <= seg_fault_addr && seg_fault_addr < segment_end_addr) {
			uintptr_t segm_file_size = exec->segments[i].file_size;
			unsigned int page_index = (seg_fault_addr - exec->segments[i].vaddr)/page_size;
			uintptr_t page_addr = exec->segments[i].vaddr + page_index * page_size;
			uintptr_t page_offset = exec->segments[i].offset + page_index * page_size;
			uintptr_t length = 0;

			// CASE 3: The page has already been mapped
			if (msync((void *)page_addr, page_size, 0) == 0)
				default_handler.sa_sigaction(signum, info, context);

			// CASE 2: The page was not mapped
			mmap((void *)page_addr, page_size, 0b110, MAP_FIXED | MAP_SHARED | MAP_ANONYMOUS, 0, 0);

			// Filling the entire page with 0
			memset((void *)page_addr, 0, (size_t)page_size);

			// Calculating the length of data to be read
			if (page_index * page_size < segm_file_size && (page_index+1) * page_size >= segm_file_size)
				length = segm_file_size - page_index * page_size;
			else if ((page_index + 1) * page_size < segm_file_size)
				length = page_size;

			// Reading the data from page
			lseek(fd, page_offset, SEEK_SET);
			read(fd, page_addr, length);

			// Setting permissions
			mprotect(page_addr, page_size, exec->segments[i].perm);

			return;
		}
	}

	// CASE 1: No segment found
	default_handler.sa_sigaction(signum, info, context);
}

int so_init_loader(void)
{
	int rc;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = segv_handler;
	sa.sa_flags = SA_SIGINFO;
	rc = sigaction(SIGSEGV, &sa, &default_handler);
	if (rc < 0) {
		perror("sigaction");
		return -1;
	}
	return 0;
}

int so_execute(char *path, char *argv[])
{
	fd = open(path, 0b100);

	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	so_start_exec(exec, argv);

	return -1;
}
