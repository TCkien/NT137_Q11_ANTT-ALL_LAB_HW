/*
Self extract program: https://cplusplus.com/forum/general/56128/
CreateProcessA: https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessa
Nhúng các blob nhị phân bằng gcc mingw: https://stackoverflow.com/questions/2627004/embedding-binary-blobs-using-gcc-mingw
*/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "huff_mem.h"
#include "caesar.h"

// Symbols created by linker when embedding blob.bin
// Using objcopy/lld or `ld -r` style embedding
extern const unsigned char _binary_blob_bin_start[];
extern const unsigned char _binary_blob_bin_end[];

// Launch the unpack exe after unpacking
static int launch_exe_and_wait(const char *path){
    STARTUPINFOA si; 
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    // Build mutable command line string
    char cmd[MAX_PATH+1];
    snprintf(cmd, sizeof(cmd), "\"%s\"", path); // Use cmd to run the program

    BOOL ok = CreateProcessA(
        NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    if(!ok){
        fprintf(stderr, "CreateProcess failed: %lu\n", GetLastError());
        return -1;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hThread); 
    CloseHandle(pi.hProcess);
    return 0;
}

int main(void){
    // Pointer to embedded blob
    const uint8_t *blob = _binary_blob_bin_start;
    size_t blob_sz = (size_t)(_binary_blob_bin_end - _binary_blob_bin_start);
    if(blob_sz == 0){ 
        fprintf(stderr,"No embedded blob\n"); 
        return 1; 
    }

    // Decrypt in memory using Caesar cipher (shift=3)
    uint8_t *dec = NULL; size_t dec_sz = 0;
    if(caesar_decrypt_mem(blob, blob_sz, &dec, &dec_sz, 3) != 0){
        fprintf(stderr,"decryption failed\n"); 
        return 1;
    }

     // Decompress using Huffman
    uint8_t *out = NULL; size_t out_sz = 0;
    if(huff_decompress_mem(dec, dec_sz, &out, &out_sz) != 0){
        fprintf(stderr,"decompress failed\n"); 
        free(dec); 
        return 1;
    }
    free(dec);

    // write unpacked.exe to current directory
    const char *outname = "unpacked.exe";
    FILE *f = fopen(outname, "wb");
    if(!f){ 
        perror("fopen"); 
        free(out); 
        return 1; 
    }
    if(fwrite(out, 1, out_sz, f) != out_sz){ 
        perror("fwrite"); 
        fclose(f); 
        free(out); 
        return 1; 
    }
    fclose(f);
    free(out);

    // run and wait
    if(launch_exe_and_wait(outname) != 0){
        fprintf(stderr,"failed to launch\n");
        return 1;
    }

    return 0;
}
