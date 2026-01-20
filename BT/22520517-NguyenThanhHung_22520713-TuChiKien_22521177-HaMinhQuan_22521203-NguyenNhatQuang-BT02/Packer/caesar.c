/*
Thuật toán Caesar: https://www.geeksforgeeks.org/ethical-hacking/caesar-cipher-in-cryptography/
*/
#include "caesar.h"
#include <stdlib.h>
#include <string.h>

int caesar_encrypt_mem(const uint8_t *in, size_t in_size, uint8_t **out, size_t *out_size, int shift) {
    if (!in || in_size == 0){ // Make sure input size not empty
        return -1;
    }
    uint8_t *obuf = malloc(in_size); // Allocate output buffer same size as input
    if (!obuf){
        return -1;
    }
    for (size_t i=0; i<in_size; i++){ 
        // shift forward input via shift variable.
        obuf[i] = (uint8_t)((in[i] + shift) & 0xFF); // Use 0xFF or mod 256 to not overflow
    }
    *out = obuf; // save output
    *out_size = in_size; // save output size
    return 0;
}

int caesar_decrypt_mem(const uint8_t *in, size_t in_size, uint8_t **out, size_t *out_size, int shift) {
    if (!in || in_size == 0){ // Make sure input size not empty
        return -1;
    }
    uint8_t *obuf = malloc(in_size); // Allocate output buffer same size as input
    if (!obuf){ 
        return -1;
    }
    for (size_t i=0; i<in_size; i++){
        // shift back input via shift variable.
        obuf[i] = (uint8_t)((in[i] - shift) & 0xFF); // Use 0xFF or mod 256 to not overflow
    }
    *out = obuf; // save output
    *out_size = in_size; // save output size
    return 0;
}

// Function for in memory decryption
int caesar_decrypt_mem_inplace(const uint8_t *in, size_t in_size, uint8_t *out_buf, size_t *out_size, int shift) {
    if (!in || in_size == 0 || !out_buf){ // Make sure input size not empty
        return -1;
    }
    for (size_t i=0; i<in_size; i++){
        out_buf[i] = (uint8_t)((in[i] - shift) & 0xFF);
    }
    *out_size = in_size;
    return 0;
}
