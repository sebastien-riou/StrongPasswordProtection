#include "test_helpers.h"

__attribute__((__section__(".spp_meta"))) const spp_meta_t meta = {0};//members will be filled out at elf file post processing step
#define PROTECTED __attribute__((__section__(".spp_protected")))

PROTECTED void greetings(void){
    //WARNING: even though we protect the whole function, the string may still appear in clear in the binary
    //If you want to encrypt the string, refer to the "string" test
    printf("protected greetings\n");
}

PROTECTED void goodbye(void){
    //WARNING: even though we protect the whole function, the string may still appear in clear in the binary
    //If you want to encrypt the string, refer to the "string" test
    printf("protected goodbye\n");
}

int main(int argc, char **argv){
    int force = argc!=1;//add a dummy argument to force execution after a wrong password has been detected
    ask_for_password("protected code",&meta,force);
    greetings();
    goodbye();
    return 0;
}
