#!/usr/bin/env python3
import os
import sys
import runpy
import shutil
import subprocess
try:
    from Crypto.Hash import SHA256
except:
    from Cryptodome.Hash import SHA256

def get_section_header_from_elf(elf,section_name):
    objdump = shutil.which('objdump')
    cmd = [ objdump,elf,'-h','--section='+section_name]
    out=""
    res=subprocess.run(cmd, stdout=subprocess.PIPE, check=True)
    out = res.stdout
    fields = out.splitlines()[5].split()
    size=int("0x"+fields[2].decode("utf-8") ,0)
    run=int("0x"+fields[3].decode("utf-8") ,0)
    load=int("0x"+fields[4].decode("utf-8") ,0)
    file_offset=int("0x"+fields[5].decode("utf-8") ,0)
    return {'load':load,'run':run,'size':size,'file_offset':file_offset}

def get_section_headers_from_elf(elf):
    objdump = shutil.which('objdump')
    cmd = [ objdump,elf,'-h']
    out=""
    res=subprocess.run(cmd, stdout=subprocess.PIPE, check=True)
    out = res.stdout
    lines = out.splitlines()[5:]
    #discard all odd lines
    del lines[1::2]
    sections = {}
    for line in lines:
        fields = line.split()
        name = fields[1].decode("utf-8")
        size=int("0x"+fields[2].decode("utf-8") ,0)
        run=int("0x"+fields[3].decode("utf-8") ,0)
        load=int("0x"+fields[4].decode("utf-8") ,0)
        file_offset=int("0x"+fields[5].decode("utf-8") ,0)
        sections[name]={'name':name,'load':load,'run':run,'size':size,'file_offset':file_offset}
    return sections
    
def get_spp_meta_section_from_elf(elf):
    objdump = shutil.which('objdump')
    cmd = [ objdump,elf,'--headers','--section=.rodata']
    out=""
    res=subprocess.run(cmd, stdout=subprocess.PIPE, check=True)
    out = res.stdout
    rodataline = out.splitlines()[5]
    fields = rodataline.split()
    assert(".rodata"==fields[1].decode("utf-8"))
    rodata_load = int("0x"+fields[4].decode("utf-8") ,0)
    rodata_file_offset = int("0x"+fields[5].decode("utf-8") ,0)
    
    objdump = shutil.which('objdump')
    cmd = [ objdump,elf,'--syms']
    out=""
    res=subprocess.run(cmd, stdout=subprocess.PIPE, check=True)
    out = res.stdout
    lines = out.splitlines()[5:]
    #discard all lines not ending by "_spp_meta"
    metalines = list(filter(lambda l: l.decode("utf-8").endswith('_spp_meta'),lines))
    print(metalines)
    sections = {}
    for line in metalines:
        fields = line.split()
        print(fields)
        load=int("0x"+fields[0].decode("utf-8") ,0)
        assert(".rodata"==fields[3].decode("utf-8"))
        size=int("0x"+fields[4].decode("utf-8") ,0)
        name = fields[5].decode("utf-8")
        run=load
        file_offset=rodata_file_offset+load-rodata_load
        sections[name]={'name':name,'load':load,'run':run,'size':size,'file_offset':file_offset}
    return sections
    
def get_spp_protected_section_from_elf(elf,spp_meta_sections):
    all_sections = get_section_headers_from_elf(elf)
    sections = {}
    with open(elf,"rb") as f:
        for meta in spp_meta_sections.values():
            print(meta)
            name = meta['name'].replace('meta','protected')
            f.seek(meta['file_offset'])
            start = int.from_bytes(f.read(4),byteorder='little')
            stop = int.from_bytes(f.read(4),byteorder='little')
            run = int.from_bytes(f.read(4),byteorder='little')
            containing_section=None
            print("%x, %x"%(start,stop))
            for s in all_sections.values():
                s_start = s['load']
                s_stop = s['load']+s['size']
                print("%x, %x"%(s_start,s_stop))
                if (start>=s_start) and (stop<=s_stop):
                    containing_section = s
            assert(containing_section is not None)
            file_offset = containing_section['file_offset']+start-s['load']
            sections[name]={'name':name,'load':load,'run':run,'size':size,'file_offset':file_offset}
    return sections
    
    
if __name__ == "__main__":
    debug=0
    if len(sys.argv)<2:
        print("ERROR: need at least 1 argument")
        print("usage:")
        print("\t%s <src elf path> [dst elf path] [secrets path] [SPP_APW_EVEN]"%os.path.basename(__file__))
        print("\t",sys.argv)
        exit(-1)
    elfpath = sys.argv[1]
    outpath = elfpath+".protected"
    if len(sys.argv) > 2:
        outpath = sys.argv[2]
    secrets_path = "spp_apw.py"
    if len(sys.argv) > 3:
        secrets = sys.argv[3]
    SPP_APW_EVEN=0xF8
    if len(sys.argv) > 4:
        SPP_APW_EVEN = int(sys.argv[4],0)
    shutil.copy(elfpath,outpath)
    sections = get_section_headers_from_elf(outpath)
    protected_sections = {k: v for k, v in sections.items() if k.endswith('.spp_protected')}
    protected_meta = {k: v for k, v in sections.items() if k.endswith('.spp_meta')}
    if 0==len(protected_meta):
        protected_meta = get_spp_meta_section_from_elf(outpath)
        protected_sections = get_spp_protected_section_from_elf(outpath,protected_meta)
        
    with open(secrets_path, 'w') as secrets:
        secrets.write('secrets=[]\n')
        for name,section in protected_sections.items():
            print(name)
            if debug:
                print(section)
            meta_name = name.replace('.spp_protected','.spp_meta')
            meta = protected_meta[meta_name]
            if debug:
                print(meta)
            try:
                with open(outpath, 'r+b') as f:
                    f.seek(meta['file_offset'])
                    state = f.read(meta['size'])
                    if debug:
                        print(state)
                    assert(0 == sum(state))
                    ptrsize = (meta['size']-32) // 3
                    assert(meta['size'] == ptrsize*3+32)
                    f.seek(meta['file_offset'])
                    load = section['load'].to_bytes(ptrsize,byteorder='little')
                    run = section['run'].to_bytes(ptrsize,byteorder='little')
                    size = section['size'].to_bytes(ptrsize,byteorder='little')
                    load_stop = (section['load']+section['size']).to_bytes(ptrsize,byteorder='little')
                    f.write(load)
                    f.write(load_stop)
                    f.write(run)
                    #generate SPP_APW
                    spp_seed = os.urandom(32)
                    spp_seed_iterations = 1
                    spp_otp_state = spp_seed
                    def spp_otp():
                        global spp_otp_state
                        spp_otp_state = SHA256.new(spp_otp_state).digest()
                        return spp_otp_state
                    for i in range(0,spp_seed_iterations):
                        spp_otp()
                    SPP_APW_odd = spp_otp()
                    if debug:
                        print("\tSPP_APW_odd")
                        print("\t%064x"%int.from_bytes(SPP_APW_odd, byteorder='big'))
                    SPP_APW=bytearray()
                    for b in SPP_APW_odd:
                        SPP_APW.append(0xF8)
                        SPP_APW.append(b)
                    spp_otp_state = SHA256.new(SPP_APW).digest()
                    bypass_pw=spp_otp_state
                    if debug:
                        print("\tSPP_SEC_BYPASS_PW")
                        print("\t%064x"%int.from_bytes(bypass_pw, byteorder='big'))
                    spp_otp_state = SHA256.new(bypass_pw).digest()
                    buf=spp_otp_state
                    b=(1).to_bytes(1,byteorder='little')
                    buf+=b
                    SPP_DIGEST = SHA256.new(buf).digest()
                    if debug:
                        print("\tSPP_DIGEST")
                        print("\t%064x"%int.from_bytes(SPP_DIGEST, byteorder='big'))
                    f.write(SPP_DIGEST)

                    f.seek(section['file_offset'])
                    dat=f.read(section['size'])

                    SPP_BLOCKS = (len(dat)+31)//32

                    SPP_OTP = bytearray()
                    SPP_OTP+=spp_otp_state
                    for i in range(1,SPP_BLOCKS):
                        SPP_OTP+=spp_otp()

                    protected_dat = bytearray()

                    for i in range(0,len(dat)):
                        protected_dat.append(SPP_OTP[i] ^ dat[i])

                    f.seek(section['file_offset'])
                    f.write(protected_dat)

                    spp_seed_hex = "0x%032x"%int.from_bytes(spp_seed, byteorder='big')
                    spp_apw_hex = "0x%032x"%int.from_bytes(SPP_APW_odd, byteorder='big')
                    spp_bpw_hex = "0x%032x"%int.from_bytes(bypass_pw, byteorder='big')
                    secrets.write('#%s %s\n'%(name,spp_apw_hex[2:]))
                    secrets.write('secrets.append({')
                    secrets.write('"name":"%s",'%name)
                    secrets.write('"SPP_APW_EVEN":0x%02X,'%(SPP_APW_EVEN))
                    secrets.write('"seed_iterations":%d,'%(spp_seed_iterations))
                    secrets.write('"seed":%s.to_bytes(32,byteorder="big"),'%(spp_seed_hex))
                    secrets.write('"SPP_APW_odd":%s.to_bytes(32,byteorder="big"),'%(spp_apw_hex))
                    secrets.write('"SPP_SEC_BYPASS_PW":%s.to_bytes(32,byteorder="big")})\n\n'%(spp_bpw_hex))
            except:
                if debug:
                    raise
                os.remove(outpath)
                #os.remove(secrets_path)
                print("ERROR: %s has been already modified, we need the original one"%elfpath)
                exit(-1)
