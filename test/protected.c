#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#define SPP_TYPE_ONLY
#include "spp.h"
__attribute__((__section__(".greetings.spp_protected"))) const char greetings_str[] = "protected greetings\n";
__attribute__((__section__(".greetings.spp_meta"))) const spp_meta_t greetings_meta = {.size = 0};//members will be filled out at elf file post processing step
