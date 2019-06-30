# Strong Password Protection

Library and tool to protect code (and data) sections by a password. The password is not stored anywhere in the resulting executable, so security level is optimal.
This is mainly intended for use in embedded systems, so the threat model takes physical attacks into account (glitches, laser fault attacks, side channels attacks...)
Nevertheless it is usable on Linux systems as well.

## Concept
In this section we use the word ROM to refer to the memory that holds the code while the device is powered off, it may be actually Flash, MRAM, OTP...
- At implementation time:
    - Include spp.h in the software
    - Put the code/data to protect in a dedicated section \*.spp_protected
    - Put place holders in a metadata section \*.spp_metadata
    - Call spp_open function before the protected code/data is used
- At build time:
    - Elf file is built as usual but it is not usable
    - spp.py does the following:
        - Find the section to protect and the associated metadata section
        - Generate a 32 bytes password at random
        - Derive random bit stream using SHA256
        - Xor it with the section to protect
        - Compute reference value SHA256(password,0x01), write it in metadata section
        - Write the resulting Elf file, which is usable
- At run time:
    - spp_open function does the following:
        - Get the password from user
        - Check SHA256(password,0x01) against reference value in ROM
        - Derive random bit stream from password using SHA256
        - Xor it with the protected section
        - Write result in RAM: the protected code/data is usable from there

Possible evolutions:
    - Compress the protected section using lz4
    - Support update of the password by using a tweak value xored to SHA256(password)

## Demo

### The test program
It is a Linux program which does the following:
- Ask for a password (It expects a 32 bytes hexadecimal number)
- If the password does not match, exit
- Execute the *greetings* function: it prints 'protected greetings'
- Execute the *goodbye* function: it prints 'protected goodbye'

The code:

    #include "test_helpers.h"

    __attribute__((__section__(".spp_meta"))) const spp_meta_t meta = {0};//members will be filled out at elf file post processing step
    #define PROTECTED __attribute__((__section__(".spp_protected")))

    PROTECTED void greetings(void){
        printf("protected greetings\n");
    }

    PROTECTED void goodbye(void){
        printf("protected goodbye\n");
    }

    int main(int argc, char **argv){
        int force = argc!=1;//add a dummy argument to force execution after a wrong password has been detected
        ask_for_password("protected code",&meta,force);
        greetings();
        goodbye();
        return 0;
    }


### Build it
This step produces a.out which is not usable as is.

    user@lafite:~/Downloads/StrongPasswordProtection$ cd tests/code
    user@lafite:~/Downloads/StrongPasswordProtection/tests/code$ ./build

### Password-protect it
This step produces:
- spp_pwd.py: a file containing the generated password
- a.out.protected: the final binary, usable only with the knowledge of the password

    user@lafite:~/Downloads/StrongPasswordProtection/tests/code$ ../../spp.py a.out

### Try it with a wrong password

    user@lafite:~/Downloads/StrongPasswordProtection/tests/code$ ./a.out.protected
    enter the password for accessing 'protected code'   --> type 0000000000000000000000000000000000000000000000000000000000000000
    spp_open failed with error code 1

### Get the expected password
By default, ssp.py writes the password in the file spp_apw.py

    user@lafite:~/Downloads/StrongPasswordProtection/tests/code$ cat spp_apw.py | grep ^#
    #.spp_protected b679d54220a7f77674a41025ab5b82652c5001ffbe9f31d727e81b954f67d43e

### Try it with the right password

    user@lafite:~/Downloads/StrongPasswordProtection/tests/code$ ./a.out.protected
    enter the password for accessing 'protected code'   --> type b679d54220a7f77674a41025ab5b82652c5001ffbe9f31d727e81b954f67d43e
    protected greetings
    protected goodbye
