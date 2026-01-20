#ifndef HUFF_MEM_H
#define HUFF_MEM_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int huff_compress_mem(const uint8_t *in, size_t in_size, uint8_t **out, size_t *out_size);
int huff_decompress_mem(const uint8_t *in, size_t in_size, uint8_t **out, size_t *out_size);

#ifdef __cplusplus
}
#endif

#endif
