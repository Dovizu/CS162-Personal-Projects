/*
 * mm_alloc.c
 *
 * Stub implementations of the mm_* routines. Remove this comment and provide
 * a summary of your allocator's design here.
 */

#include "mm_alloc.h"

#include <stdlib.h>

int convert_to_4_aligned(int size) {
    return (((((size)-1)>>2)<<2)+4);
}

void* mm_malloc(size_t size) {

}

void* mm_realloc(void* ptr, size_t size) {

}

void mm_free(void* ptr) {

}

void split_block(s_block_ptr b, size_t s) {

}

// private

s_block_ptr fusion(s_block_ptr b) {

}

s_block_ptr get_block (void *p) {

}

s_block_ptr extend_heap (s_block_ptr last , size_t s) {
    s_block_ptr brk = sbrk(0); // get old break
    size_t block_size = sizeof(struct s_block);
    if (sbrk(block_size + s) == (void *)-1) {
        // fail case
        return NULL
    }
    brk->size = s;
    brk->next = NULL;
    if (last) {
        brk->prev = last;
        last->next = brk;
    }
    brk->free = 0;
    return brk;
}

s_block_ptr find_block(s_block_ptr *last, size_t size) {
    s_block_ptr b = root;
    while (b && !(b->free && b->size >= size)) {
        *last = b;
        b = b->next;
    }
    return b;
}