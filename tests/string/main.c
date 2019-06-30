#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "test_helpers.h"

extern const char greetings_str[];//we have to put the string in a separate file otherwise GCC is duplicating it and the useful copy is not encrypted
extern const spp_meta_t greetings_meta;
void greetings(void){
    printf("%s",greetings_str);
}

int main(int argc, char **argv){
    int force = argc!=1;//add a dummy argument to force execution after a wrong password has been detected
    ask_for_password("greetings",&greetings_meta,force);
    greetings();
    return 0;
}
