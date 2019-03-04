#ifndef TINYALLOC_H
#define TINYALLOC_H

#include <stdbool.h>
#include <stddef.h>

#define TA_DISABLE_COMPACT

bool ta_init();
void *ta_alloc(size_t num);
void *ta_calloc(size_t num, size_t size);
bool ta_free(void *ptr);

size_t ta_num_free();
size_t ta_num_used();
size_t ta_num_fresh();
bool ta_check();

typedef struct Block Block;

struct Block {
    void *addr;
    Block *next;
    size_t size;
};

#define TA_SPLIT_THRESH 16
#define TA_HEAP_BLOCKS 32
typedef struct {
    Block *free;   // first free block
    Block *used;   // first used block
    Block *fresh;  // first available blank block
    unsigned int top;    // top free addr
    Block blocks[TA_HEAP_BLOCKS];
} Heap;

extern Heap ta_heap;

extern unsigned char *ta_heap_start;
extern unsigned char *ta_heap_limit;

#define TA_HEAP_START ta_heap_start
#define TA_HEAP_LIMIT ta_heap_limit


#endif
