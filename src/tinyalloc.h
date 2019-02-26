#ifndef TINYALLOC_H
#define TINYALLOC_H

#include <stdbool.h>
#include <stddef.h>

#define TINYALLOC_HEAP 8192
extern char tinyalloc_heap[TINYALLOC_HEAP];

bool ta_init();
void *ta_alloc(size_t num);
void *ta_calloc(size_t num, size_t size);
bool ta_free(void *ptr);

size_t ta_num_free();
size_t ta_num_used();
size_t ta_num_fresh();
bool ta_check();

#endif
