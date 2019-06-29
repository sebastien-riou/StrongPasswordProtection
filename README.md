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
It is a Linux program which display 'protected greetings' when the correct password is entered.
It expects a 32 bytes hexadecimal number as password.

### Build it

    cd test
    ~/dev/StrongPasswordProtection$ cd test
    ~/dev/StrongPasswordProtection/test$ ./build

### Password-protect it

    ~/dev/StrongPasswordProtection/test$ ../spp.py a.out

### Try it with a wrong password

    ~/dev/StrongPasswordProtection/test$ ./a.out
    enter the password
    0000000000000000000000000000000000000000000000000000000000000000
    spp_open failed with error code 1

### Get the expected password
By default, ssp.py writes the password in the file spp_apw.py

    ~/dev/StrongPasswordProtection/test$ head -2 spp_apw.py
    secrets=[]
    #.greetings.spp_protected fcd52e456880ecbe6cc525a5f7f3df6b2088a688f0a66ce856693bd668f1427d

### Try it with the right password

    ~/dev/StrongPasswordProtection/test$ ./a.out
    enter the password
    fcd52e456880ecbe6cc525a5f7f3df6b2088a688f0a66ce856693bd668f1427d
    protected greetings
