#ifndef CAESAR_H
#define CAESAR_H
#include <stddef.h>
#include <stdint.h>

int caesar_encrypt_mem(const uint8_t *in, size_t in_size, uint8_t **out, size_t *out_size, int shift);
int caesar_decrypt_mem(const uint8_t *in, size_t in_size, uint8_t **out, size_t *out_size, int shift);
int caesar_decrypt_mem_inplace(const uint8_t *in, size_t in_size, uint8_t *out_buf, size_t *out_size, int shift);

#endif
