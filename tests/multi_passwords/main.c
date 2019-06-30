#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "test_helpers.h"

__attribute__((__section__(".greetings.spp_meta"))) const spp_meta_t greetings_meta = {0};//members will be filled out at elf file post processing step
__attribute__((__section__(".greetings.spp_protected"))) void greetings(void){
    //WARNING: even though we protect the whole function, the string may still appear in clear in the binary
    //If you want to encrypt the string, refer to the "string" test
    printf("protected greetings\n");
}

__attribute__((__section__(".goodbye.spp_meta"))) const spp_meta_t goodbye_meta = {0};//members will be filled out at elf file post processing step
__attribute__((__section__(".goodbye.spp_protected"))) void goodbye(void){
    //WARNING: even though we protect the whole function, the string may still appear in clear in the binary
    //If you want to encrypt the string, refer to the "string" test
    printf("protected goodbye\n");
}

int main(int argc, char **argv){
    int force = argc!=1;//add a dummy argument to force execution after a wrong password has been detected
    ask_for_password("greetings",&greetings_meta,force);
    greetings();
    ask_for_password("goodbye",&goodbye_meta,force);
    goodbye();
    return 0;
}
