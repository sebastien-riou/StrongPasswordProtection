#ifndef __MMU_H__
#define __MMU_H__

//on embedded systems a linker file shall place .greeting.spp_protected LOAD address in ROM and RUN address in RAM
//This test is executed on linux/x86, rather than creating a linker file, we just put that section as read/write and overwrite it
//to make the .greetings.spp_protected secrion writable we use the following function
#include <sys/mman.h>
#include <unistd.h>
static int change_page_permissions_of_address(const void *const addr) {
    // Move the pointer to the page boundary
    int page_size = sysconf(_SC_PAGESIZE);
    void* page = (void*)addr - ((unsigned long)addr % page_size);
    if(mprotect(page, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
        return -1;
    }
    return 0;
}


#endif
