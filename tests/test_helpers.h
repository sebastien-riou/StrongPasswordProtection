#ifndef __TEST_HELPERS_H__
#define __TEST_HELPERS_H__
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <termios.h>

#include "mmu.h"
#include "bytes_utils.h"

char getch(){
    char buf=0;
    struct termios old={0};
    fflush(stdout);
    if(tcgetattr(0, &old)<0)
        perror("tcsetattr()");
    old.c_lflag&=~ICANON;
    old.c_lflag&=~ECHO;
    old.c_cc[VMIN]=1;
    old.c_cc[VTIME]=0;
    if(tcsetattr(0, TCSANOW, &old)<0)
        perror("tcsetattr ICANON");
    if(read(0,&buf,1)<0)
        perror("read()");
    old.c_lflag|=ICANON;
    old.c_lflag|=ECHO;
    if(tcsetattr(0, TCSADRAIN, &old)<0)
        perror ("tcsetattr ~ICANON");
    //printf("%c",buf);
    return buf;
 }


typedef uint8_t spp_rx_t;
spp_rx_t spp_rx(void){
    int c = getch( );
    spp_rx_t out = hexdigit_value(c);
    out = (out<<4) | hexdigit_value(getch());
    //printf("got %02X\n",out);
    return out;
}

#include "spp.h"

#include <termios.h>
#include <unistd.h>

static void ask_for_password(const char* msg, const spp_meta_t *const meta, int force){
    printf("enter the password for accessing '%s'\n",msg);
    assert(0==change_page_permissions_of_address(meta->dst));
    uint8_t red_buffer[SPP_HASH_BLOCK_SIZE*2];
    //term_raw_mode(1);
    int status = spp_open(meta,red_buffer);
    //term_raw_mode(0);
    if(status){
        printf("spp_open failed with error code %d\n",status);
        if(0==force) exit(status);
        printf("try to continue anyway\n");
    }
}

#endif
