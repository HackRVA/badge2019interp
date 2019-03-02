#include <stdint.h>
#include "tinyalloc.h"

unsigned char *ta_heap_start=NULL;
unsigned char *ta_heap_limit=NULL;


#define TA_HEAP_START ta_heap_start
#define TA_HEAP_LIMIT ta_heap_limit


#ifndef TA_ALIGN
#define TA_ALIGN 4
#endif


#ifndef TA_HEAP_BLOCKS
#define TA_HEAP_BLOCKS 64
#endif

#ifndef TA_SPLIT_THRESH
#define TA_SPLIT_THRESH 16
#endif

#ifdef TA_DEBUG
extern void print_s(char *);
extern void print_i(size_t);
#else
#define print_s(X)
#define print_i(X)
#endif

Heap ta_heap;

#ifdef DOMAIN
#include <memory.h>

/*

cc -m32 -g -o ta -DDOMAIN tinyalloc.c

*/
#define TEXTSZ (2048+1024)
#define DATASZ 256
#define STACKSZ 256
#define SYMBOLSZ 4096

#define INTERP_RAM interpreter_ram
#define INTERP_RAM_SIZE (TEXTSZ+DATASZ+STACKSZ+SYMBOLSZ)
char interpreter_ram[INTERP_RAM_SIZE];

int main(int argc, char**argv)
{
    int textsection=TEXTSZ;
    int datasection=DATASZ;
    int stacksection=STACKSZ;
    int symbolsection=SYMBOLSZ;
    int *text, *stack, *data, *symbols;

    /* init tinyalloc */
    ta_heap_start = INTERP_RAM;
    ta_heap_limit = &INTERP_RAM[INTERP_RAM_SIZE];

    ta_init();

    text = ta_alloc(textsection);
    memset(text, 0, textsection);

    data = ta_alloc(datasection);
    memset(data, 0, datasection);

    stack = ta_alloc(stacksection);
    memset(stack, 0, stacksection);

    symbols = ta_alloc(symbolsection);
    memset(symbols, 0, symbolsection);

    printf("text %x\n", text);
    printf("data %x\n", data);
    printf("stack %x\n", stack);
    printf("symbols %x\n", symbols);

    exit(0);
}

#endif

/**
 * If compaction is enabled, inserts block
 * into free list, sorted by addr.
 * If disabled, add block has new head of
 * the free list.
 */
static void insert_block(Block *block) {
#ifndef TA_DISABLE_COMPACT
    Block *ptr  = ta_heap.free;
    Block *prev = NULL;
    while (ptr != NULL) {
        if ((size_t)block->addr <= (size_t)ptr->addr) {
            print_s("insert");
            print_i((size_t)ptr);
            break;
        }
        prev = ptr;
        ptr  = ptr->next;
    }
    if (prev != NULL) {
        if (ptr == NULL) {
            print_s("new tail");
        }
        prev->next = block;
    } else {
        print_s("new head");
        ta_heap.free = block;
    }
    block->next = ptr;
#else
    block->next = ta_heap.free;
    ta_heap.free  = block;
#endif
}

#ifndef TA_DISABLE_COMPACT
static void release_blocks(Block *scan, Block *to) {
    Block *scan_next;
    while (scan != to) {
        print_s("release");
        print_i((size_t)scan);
        scan_next   = scan->next;
        scan->next  = ta_heap.fresh;
        ta_heap.fresh = scan;
        scan->addr  = 0;
        scan->size  = 0;
        scan        = scan_next;
    }
}

static void compact() {
    Block *ptr = ta_heap.free;
    Block *prev;
    Block *scan;
    while (ptr != NULL) {
        prev = ptr;
        scan = ptr->next;
        while (scan != NULL &&
               (unsigned int)prev->addr + prev->size == (unsigned int)scan->addr) {
            print_s("merge");
            print_i((size_t)scan);
            prev = scan;
            scan = scan->next;
        }
        if (prev != ptr) {
            unsigned int new_size =
                (unsigned int)prev->addr - (unsigned int)ptr->addr + prev->size;
            print_s("new size");
            print_i(new_size);
            ptr->size   = new_size;
            Block *next = prev->next;
            // make merged blocks available
            release_blocks(ptr->next, prev->next);
            // relink
            ptr->next = next;
        }
        ptr = ptr->next;
    }
}
#endif

bool ta_init() {
    ta_heap.free   = NULL;
    ta_heap.used   = NULL;
    ta_heap.fresh  = ta_heap.blocks;
    ta_heap.top    = (unsigned int)TA_HEAP_START;
    Block *block = ta_heap.blocks;
    unsigned int i = TA_HEAP_BLOCKS - 1;
    while (i--) {
        block->next = block + 1;
        block->addr = NULL;
        block->size = 0;
        block++;
    }
    block->next = NULL;
    block->addr = NULL;
    block->size = 0;
    return true;
}

bool ta_free(void *free) {
    Block *block = ta_heap.used;
    Block *prev  = NULL;
    while (block != NULL) {
        if (free == block->addr) {
            if (prev) {
                prev->next = block->next;
            } else {
                ta_heap.used = block->next;
            }
            insert_block(block);
#ifndef TA_DISABLE_COMPACT
            compact();
#endif
            return true;
        }
        prev  = block;
        block = block->next;
    }
    return false;
}

static Block *alloc_block(size_t num) {
    Block *ptr  = ta_heap.free;
    Block *prev = NULL;
    unsigned int top  = ta_heap.top;
    num         = (num + TA_ALIGN - 1) & -TA_ALIGN;
    while (ptr != NULL) {
        int is_top = ((unsigned int)ptr->addr + ptr->size >= top) && ((unsigned int)ptr->addr + num <= (unsigned int)TA_HEAP_LIMIT);
        if (is_top || ptr->size >= num) {
            if (prev != NULL) {
                prev->next = ptr->next;
            } else {
                ta_heap.free = ptr->next;
            }
            ptr->next  = ta_heap.used;
            ta_heap.used = ptr;
            if (is_top) {
                print_s("resize top block");
                ptr->size = num;
                ta_heap.top = (unsigned int)ptr->addr + num;
#ifndef TA_DISABLE_SPLIT
            } else if (ta_heap.fresh != NULL) {
                unsigned int excess = ptr->size - num;
                if (excess >= TA_SPLIT_THRESH) {
                    ptr->size    = num;
                    Block *split = ta_heap.fresh;
                    ta_heap.fresh  = split->next;
                    split->addr  = (void *)((unsigned int)ptr->addr + num);
                    print_s("split");
                    print_i((unsigned int)split->addr);
                    split->size = excess;
                    insert_block(split);
#ifndef TA_DISABLE_COMPACT
                    compact();
#endif
                }
#endif
            }
            return ptr;
        }
        prev = ptr;
        ptr  = ptr->next;
    }
    // no matching free blocks
    // see if any other blocks available
    unsigned int new_top = top + num;
    if (ta_heap.fresh != NULL && new_top <= (unsigned int)TA_HEAP_LIMIT) {
        ptr         = ta_heap.fresh;
        ta_heap.fresh = ptr->next;
        ptr->addr   = (void *)top;
        ptr->next   = ta_heap.used;
        ptr->size   = num;
        ta_heap.used  = ptr;
        ta_heap.top   = new_top;
        return ptr;
    }
    return NULL;
}

void *ta_alloc(size_t num) {
    Block *block = alloc_block(num);
    if (block != NULL) {
        return block->addr;
    }
    return NULL;
}

static void memclear(void *ptr, size_t num) {
    unsigned int *ptrw = (unsigned int *)ptr;
    unsigned int numw  = (num & -sizeof(int)) / sizeof(int);
    while (numw--) {
        *ptrw++ = 0;
    }
    num &= (sizeof(unsigned int) - 1);
    uint8_t *ptrb = (uint8_t *)ptrw;
    while (num--) {
        *ptrb++ = 0;
    }
}

void *ta_calloc(size_t num, size_t size) {
    num *= size;
    Block *block = alloc_block(num);
    if (block != NULL) {
        memclear(block->addr, num);
        return block->addr;
    }
    return NULL;
}

static size_t count_blocks(Block *ptr) {
    size_t num = 0;
    while (ptr != NULL) {
        num++;
        ptr = ptr->next;
    }
    return num;
}

size_t ta_num_free() {
    return count_blocks(ta_heap.free);
}

size_t ta_num_used() {
    return count_blocks(ta_heap.used);
}

size_t ta_num_fresh() {
    return count_blocks(ta_heap.fresh);
}

bool ta_check() {
    return TA_HEAP_BLOCKS == ta_num_free() + ta_num_used() + ta_num_fresh();
}
