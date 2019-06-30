#ifndef __SPP_H__
#define __SPP_H__

/*
Expects the following definitions:
types:
    spp_rx_t
functions:
    spp_rx_t spp_rx(void)

preferred definitions for security:
typedef uint8_t spp_rx_t;
*/

#ifndef __SHA256_H__
#define SHA256_ONLY_BE
#include "sha256.h"
#endif

#define SPP_HASH_BLOCK_SIZE SHA256_BLOCK_SIZE

#ifndef SPP_APW_EVEN
#define SPP_APW_EVEN 0xF8
#endif

typedef struct spp_meta_struct_t {
    const void*const start;
    const void*const stop;
    void*const dst;
    uint8_t digest[SPP_HASH_BLOCK_SIZE];
} spp_meta_t;

#ifndef SPP_TYPE_ONLY
static int spp_open(const spp_meta_t*const meta, uint8_t*const red_buf){
	const uint8_t*src=meta->start;
    uint8_t*dst=meta->dst;
    size_t size = meta->stop-meta->start;
    //printf("src = %lx\n",(uint64_t)src);
    //printf("dst = %lx\n",(uint64_t)dst);
    //printf("meta->size = %lu\n",meta->size);
    uint8_t *spp_apw=red_buf;
    for(unsigned int i=0;i<SPP_HASH_BLOCK_SIZE;i+=sizeof(spp_rx_t)){
        spp_rx_t rxdat=spp_rx();
        const uint8_t *const rxdat8=(const uint8_t *const)&rxdat;
        unsigned int offset=2*i;
        for(unsigned int j=0;j<sizeof(spp_rx_t);j++){
            spp_apw[offset+2*j]=SPP_APW_EVEN;
            spp_apw[offset+2*j+1]=rxdat8[j];
        }
    }
    uint8_t dstbuf[SPP_HASH_BLOCK_SIZE+1];
    //print_bytes("spp_apw ",spp_apw,64,"\n");
    sha256_sum(spp_apw,SPP_HASH_BLOCK_SIZE*2,dstbuf);
    //print_bytes("dstbuf  ",dstbuf,32,"\n");
    uint8_t *digest=red_buf;
    dstbuf[SPP_HASH_BLOCK_SIZE]=1;
    sha256_sum(dstbuf,SPP_HASH_BLOCK_SIZE+1,digest);
    //print_bytes("digest  ",digest,32,"\n");
    if(memcmp(digest,meta->digest,sizeof(meta->digest))){return 1;}//password check happens here
    size_t remaining=size;
    while(remaining){
        unsigned int size = remaining > SPP_HASH_BLOCK_SIZE ? SPP_HASH_BLOCK_SIZE : remaining;
        for(unsigned int i=0;i<size;i++){
            dst[i]=dstbuf[i]^src[i];
        }
        remaining -= size;
        if(remaining){
            sha256_sum(dstbuf,SPP_HASH_BLOCK_SIZE,dstbuf);
            dst+=SPP_HASH_BLOCK_SIZE;
            src+=SPP_HASH_BLOCK_SIZE;
        }
    }
    return 0;
}
#endif

#endif
