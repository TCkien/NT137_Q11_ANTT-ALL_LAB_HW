
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "huff_mem.h"
#include "caesar.h"
#include "file_util.h"

int main(int argc, char **argv){
    if(argc != 4){
        fprintf(stderr, "Usage: %s <input.exe> <shift> <out_blob.bin>\n", argv[0]);
        return 1;
    }
    const char *inpath = argv[1];
    int shift = atoi(argv[2]);
    const char *outpath = argv[3];

    // Check that the input is a valid PE file
    if(check_validPEfile(inpath)){
        printf("Not a valid PE file");
        return 1;
    }

    // Open the input file for reading in binary
    FILE *f = fopen(inpath, "rb"); 

    // Seek to end to measure file size
    fseek(f,0,SEEK_END); 
    long sz = ftell(f); 
    fseek(f,0,SEEK_SET);

    if(sz <= 0){ 
        // empty or invalid size
        fclose(f); 
        fprintf(stderr,"Empty file\n"); 
        return 1; 
    }

    // Allocate buffer to read the entire file
    uint8_t *buf = malloc(sz); 
    if(!buf){ 
        fclose(f); 
        return 1; 
    }

    // Read file into buf
    if(fread(buf,1,sz,f) != (size_t)sz){ 
        perror("read"); 
        free(buf); 
        fclose(f); 
        return 1; 
    }
    fclose(f);

    // Compress the buffer in-memory using Huffman
    uint8_t *comp = NULL; size_t comp_sz = 0;
    if(huff_compress_mem(buf, (size_t)sz, &comp, &comp_sz) != 0){ 
        fprintf(stderr,"compress failed\n"); 
        free(buf); 
        return 1; 
    }
    free(buf);

    // Encrypt the compressed data with Caesar cipher (shift cipher)
    uint8_t *enc = NULL; size_t enc_sz = 0;
    if(caesar_encrypt_mem(comp, comp_sz, &enc, &enc_sz, shift) != 0){ 
        fprintf(stderr,"encrypt failed\n"); 
        free(comp); 
        return 1; 
    }
    free(comp);

    // Write the encrypted blob out to a file
    FILE *out = fopen(outpath, "wb"); 
    if(!out){ perror("open out"); 
        free(enc); 
        return 1; 
    }
    if(fwrite(enc,1,enc_sz,out) != enc_sz){ 
        perror("write"); 
        free(enc); 
        fclose(out); 
        return 1; 
    }
    fclose(out);
    free(enc);

    printf("Wrote blob %s (%zu bytes)\n", outpath, enc_sz);
    return 0;
}
