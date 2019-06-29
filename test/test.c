#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "bytes_utils.h"

typedef uint8_t spp_rx_t;
spp_rx_t spp_rx(void){
    int c = getchar( );
    spp_rx_t out = hexdigit_value(c);
    out = (out<<4) | hexdigit_value(getchar());
    //printf("got %02X\n",out);
    return out;
}
#include "spp.h"


extern const char greetings_str[];
extern const spp_meta_t greetings_meta;
void greetings(void){
    printf("%s",greetings_str);
}

//on embedded systems a linker file shall place .greeting.spp_protected LOAD address in ROM and RUN address in RAM
//This test is executed on linux/x86, rather than creating a linker file, we just put that section as read/write and overwrite it
//to make the .greetings.spp_protected secrion writable we use the following function
#include <sys/mman.h>
#include <unistd.h>
int change_page_permissions_of_address(const void *const addr) {
    // Move the pointer to the page boundary
    int page_size = sysconf(_SC_PAGESIZE);
    void* page = (void*)addr - ((unsigned long)addr % page_size);
    if(mprotect(page, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
        return -1;
    }
    return 0;
}

int main(int argc, char **argv){
    assert(0==change_page_permissions_of_address((void*)greetings_str));
    printf("enter the password\n");
    uint8_t red_buffer[SPP_HASH_BLOCK_SIZE*2];
    int status = spp_open(&greetings_meta,red_buffer);
    if(status){
        printf("spp_open failed with error code %d\n",status);
        if(argc==1) return status;
        printf("try to call protected_function anyway\n");
    }
    greetings();
    return 0;
}
