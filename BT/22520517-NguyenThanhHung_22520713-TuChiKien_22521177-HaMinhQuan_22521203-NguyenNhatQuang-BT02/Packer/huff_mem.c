/*
Huffman algorithm: https://www.geeksforgeeks.org/c/huffman-coding-in-c/
Huffman for compression: https://www.youtube.com/watch?v=NjhJJYHpYsg
*/
#include "huff_mem.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SYMBOLS 256 // Number of possible byte values

// Node for Huffman Tree
typedef struct Node{
    int symbol;     // symbol value (0-255) and -1 for internal nood
    uint64_t freq;  // frequency of the symbol
    struct Node *left, *right; // left and right children
} Node;

// Heap structer for Huffman Tree
typedef struct{ 
    Node **a; // array of Node pointers
    int size; // current number of elements
    int cap; // capacity
} Heap;

// Constructor to create new node
static Node *node_new(int sym, uint64_t f){
    Node *n = (Node*)malloc(sizeof(Node));
    n->symbol = sym; 
    n->freq = f; 
    n->left = n->right = NULL; 
    return n;
}

// Constructor to create a new heap with capacity
static Heap *heap_new(int cap){
    Heap *h = malloc(sizeof(Heap));
    h->a = malloc(sizeof(Node*) * cap);
    h->size = 0; 
    h->cap = cap; 
    return h;
}

// Constructor to push node to heap
static void heap_push(Heap *h, Node *n){
    int i = h->size++;
    while(i && n->freq < h->a[(i-1)/2]->freq){
        h->a[i] = h->a[(i-1)/2];
        i = (i-1)/2;
    }
    h->a[i] = n;
}

// Constructor to pop the smallest-frequency node from heap
static Node *heap_pop(Heap *h) {
    Node *ret = h->a[0];    // root
    Node *last = h->a[--h->size]; // last node
    int i = 0;
    while(1) {
        int l = 2*i+1, r = 2*i+2, s = i;
        if (l < h->size && h->a[l]->freq < h->a[s]->freq) 
            s = l;
        if (r < h->size && h->a[r]->freq < h->a[s]->freq) 
            s = r;
        if (s==i) break;
        h->a[i] = h->a[s];
        i = s;
    }
    if(h->size) 
        h->a[i] = last;
    return ret;
}

// Destructor free the tree
static void free_tree(Node *n) {
    if(!n) 
        return;
    free_tree(n->left);
    free_tree(n->right);
    free(n);
}

// Contructor to codify the byte to combination of bit of 0 and 1 base on their location
static void build_codes(Node *n, char *buf, int depth, char **codes) {
    if(!n->left && !n->right) { // if leaf node
        buf[depth] = '\0';
        codes[(unsigned char)n->symbol] = strdup(buf); // then store code
        return;
    }
    if(n->left){ 
        buf[depth] = '0'; 
        build_codes(n->left, buf, depth+1, codes); 
    }
    if(n->right){ 
        buf[depth] = '1'; 
        build_codes(n->right, buf, depth+1, codes); 
    } 
}

// Compress memory buffer
int huff_compress_mem(const uint8_t *in, size_t in_size, uint8_t **out, size_t *out_size) {
    if(!in || in_size == 0) 
        return -1;
    
    // Count frequency of each symbol
    uint64_t freq[SYMBOLS] = {0};
    for(size_t i=0;i<in_size;i++) 
        freq[in[i]]++;

    // Build initial heap
    Heap *h = heap_new(SYMBOLS*2);
    for(int s=0;s<SYMBOLS;s++){ 
        if (freq[s]) 
            heap_push(h, node_new(s, freq[s]));
    }
    if(h->size == 0){ 
        free(h->a); free(h); 
        return -1; 
    }

    // Ensure at least 2 nodes for tree
    if(h->size == 1){ 
        int other = (h->a[0]->symbol==0)?1:0; 
        heap_push(h, node_new(other, 0)); 
    }

    // Build Huffman tree
    while(h->size > 1) {
        Node *a = heap_pop(h);
        Node *b = heap_pop(h);
        Node *m = node_new(-1, a->freq + b->freq); // internal node
        m->left = a; 
        m->right = b;
        heap_push(h, m);
    }

    // Generate Huffman codes for each symbol
    Node *root = heap_pop(h);
    char *codes[SYMBOLS]; memset(codes,0,sizeof(codes));
    char tmp[512];
    build_codes(root, tmp, 0, codes);

    // Build header which is the number of symbol used plus freq table
    uint16_t used = 0;
    for(int s=0;s<SYMBOLS;s++){
        if (freq[s]) used++;
    }
    size_t header = 2 + used*(1 + 8); 

    // Compute total bits required
    uint64_t total_bits = 0;
    for (size_t i=0;i<in_size;i++) 
        total_bits += strlen(codes[in[i]]);
    int padding = (8 - (total_bits % 8)) % 8;   // padding for byte alignment
    size_t comp_bytes = (total_bits + padding) / 8;

    size_t max_out = header + comp_bytes + 1; // +1 byte for padding info
    uint8_t *obuf = malloc(max_out);
    if(!obuf){ 
        free_tree(root); 
        free(h->a); 
        free(h); 
        return -1; 
    }

    // Write header
    size_t p = 0;
    memcpy(obuf + p, &used, 2); p += 2;
    for(int s=0;s<SYMBOLS;s++){
        if(freq[s]){
            obuf[p++] = (uint8_t)s;
            uint64_t v = freq[s];
            for(int b=0;b<8;b++) 
                obuf[p++] = (uint8_t)((v >> (8*b)) & 0xFF);
        }
    }

    // Encode input data as bits
    uint8_t bitbuf = 0;
    int bitcnt = 0;
    for(size_t i=0;i<in_size;i++){
        char *c = codes[in[i]];
        for(int k=0; c[k]; ++k){
            bitbuf = (bitbuf << 1) | (c[k]=='1');
            bitcnt++;
            if (bitcnt==8){ 
                obuf[p++] = bitbuf; 
                bitbuf = 0; 
                bitcnt = 0; 
            }
        }
    }
    if(bitcnt > 0){ 
        bitbuf <<= (8 - bitcnt); 
        obuf[p++] = bitbuf; 
    }
    obuf[p++] = (uint8_t)padding;

    // Write padding info
    *out = obuf;
    *out_size = p;

    // cleanup
    for(int s=0;s<SYMBOLS;s++){ 
        if(codes[s]) 
            free(codes[s]);
    }
    free_tree(root);
    free(h->a); 
    free(h);
    return 0;
}

// Decompress memory buffer
int huff_decompress_mem(const uint8_t *in, size_t in_size, uint8_t **out, size_t *out_size) {
    if(!in || in_size < 3) 
        return -1;

    // Read header
    size_t p = 0;
    uint16_t used = 0;
    memcpy(&used, in + p, 2); p += 2;
    uint64_t freq[SYMBOLS] = {0};
    for(int i=0;i<used;i++){
        if(p + 1 + 8 > in_size) 
            return -1;
        uint8_t sym = in[p++];
        uint64_t v = 0;
        for(int b=0;b<8;b++) 
            v |= ((uint64_t)in[p++]) << (8*b);
        freq[sym] = v;
    }

    // Rebuild Huffman Tree
    Heap *h = heap_new(SYMBOLS*2);
    for(int s=0;s<SYMBOLS;s++){ 
        if(freq[s]) 
            heap_push(h, node_new(s, freq[s]));
    }
    if(h->size == 0){ 
        free(h->a); 
        free(h); 
        return -1; 
    }
    if(h->size == 1){ 
        int other = (h->a[0]->symbol==0) ? 1:0; 
        heap_push(h, node_new(other, 0)); 
    }

    while(h->size > 1){
        Node *a = heap_pop(h); 
        Node *b = heap_pop(h);
        Node *m = node_new(-1, a->freq + b->freq);
        m->left = a; m->right = b; heap_push(h, m);
    }
    Node *root = heap_pop(h);

    // Read padding byte
    if(in_size < 1){ 
        free_tree(root); 
        free(h->a); 
        free(h); 
        return -1; 
    }
    uint8_t padding = in[in_size - 1];
    size_t bitlen = (in_size - p - 1) * 8;
    if (padding) 
        bitlen -= padding;

    // compute total symbols expected
    uint64_t total_symbols = 0;
    for(int s=0;s<SYMBOLS;s++) 
        total_symbols += freq[s];
    uint8_t *obuf = malloc(total_symbols ? (size_t)total_symbols : 1);
    if(!obuf){ 
        free_tree(root); 
        free(h->a); 
        free(h); 
        return -1; 
    }

    // Decode bit-by-bit
    size_t outp = 0;
    Node *cur = root;
    for(size_t bi=0; bi<bitlen; ++bi){
        size_t byte_idx = p + (bi/8);
        int bit_in_byte = 7 - (bi%8); // MSB first
        int bit = (in[byte_idx] >> bit_in_byte) & 1;
        cur = bit ? cur->right : cur->left;
        if(!cur->left && !cur->right){ // leaf node
            if(outp < total_symbols) 
                obuf[outp++] = (uint8_t)cur->symbol;
            cur = root;
        }
    }
    *out = obuf;
    *out_size = outp;

    // Cleanup
    free_tree(root);
    free(h->a); 
    free(h);
    return 0;
}
