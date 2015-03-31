/*
 * mm_alloc.c
 *
 * Stub implementations of the mm_* routines. Remove this comment and provide
 * a summary of your allocator's design here.
 */

#include "mm_alloc.h"

#include <stdlib.h>
#include <stdbool.h>

s_block_ptr get_block (void *p);

int convert_to_4_aligned(int size) {
    return (((((size)-1)>>2)<<2)+4);
}

bool address_is_valid(void *ptr) {
    if (root && (s_block_ptr)ptr > root && (s_block_ptr)ptr < (s_block_ptr)sbrk(0)) {
        return (ptr == (get_block(ptr))->ptr);
    }
    return NULL;
}

s_block_ptr extend_heap (s_block_ptr last , size_t s) {
    s_block_ptr brk = sbrk(0); // get old break
    size_t block_size = s_block_size;
    if (sbrk(block_size + s) == (void *)-1) {
        // fail case
        return NULL;
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

void* mm_malloc(size_t size) {
    s_block_ptr brk;
    s_block_ptr last;
    size_t s = convert_to_4_aligned(size);
    if (!root) {
        //first time calling
        brk = extend_heap(NULL, s);
        if (!brk) {
            return NULL;
        }
        root = brk;
    } else {
        last = root;
        brk = find_block(&last, s);
        if (brk) {
            if ((brk->size - s) >= (s_block_size + 4)) {
                split_block(brk, s);
            }
            brk->free = false;
        } else {
            brk = extend_heap(last, s);
            if (!brk) {
                return NULL;
            }
        }
    }
    return brk->data;
}

void* mm_realloc(void* ptr, size_t size) {

}

void mm_free(void* ptr) {
    s_block_ptr block;
    if (address_is_valid(ptr)) {
        block = get_block(ptr);
        block->free = true;
        if (block->prev && block->prev->free) {
            block = fusion(block->prev);
        }
        if (block->next) {
            fusion(block);
        } else {
            if (block->prev) {
                block->prev->next = NULL;
            } else {
                root = NULL;
            }
            brk(block);
        }
    }
}

void split_block(s_block_ptr b, size_t s) {
    s_block_ptr new_block;
    new_block = (s_block_ptr)(b->data + s);
    new_block->size = b->size - (s + s_block_size);
    new_block->free = true;
    b->size = s;
    new_block->next = b->next;
    new_block->prev = b;
    b->next = new_block;
}

// private

s_block_ptr fusion(s_block_ptr brk) {
    if (brk->next && brk->next->free) {
        brk->size += s_block_size + brk->next->size;
        brk->next = brk->next->next;
        if (brk->next) {
            brk->next->prev = brk;    
        }
    }
    return brk;
}

s_block_ptr get_block (void *p) {
    return (p - s_block_size);
}

