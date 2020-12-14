/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define INIT_SIZE 16
#define BLOCKSIZE 16
#define WORDSIZE 4
#define DEREF(p) (*(size_t*)(p))
#define NEXT_FREE(block_ptr) (*(void **)block_ptr)
#define PREV_FREE(block_ptr) (*(void **)(block_ptr + WORDSIZE))

/* 
 * mm_init - initialize the malloc package.
 */
static char* heap_ptr = 0;
static char* free_list_ptr = 0;

static inline size_t get_size(void *header_ptr){
    return DEREF(header_ptr) & (~0x1);
}

static inline size_t get_is_alloc(void* header_ptr){
    return DEREF(header_ptr) & (0x1);
}

static inline void *get_header(void* block_ptr){
    return block_ptr - WORDSIZE;
}

static inline void* get_footer(void* block_ptr){
    return block_ptr + (get_size(get_header(block_ptr)) - 2 * WORDSIZE);
}

static inline void* get_next_block(void* block_ptr){
    return block_ptr + get_size(get_header(block_ptr));
}

static inline void* get_prev_block(void* block_ptr){
    return block_ptr - get_size(get_header(block_ptr) - WORDSIZE);
}


static void *find_fit(size_t size);

static void allocate(void* block_ptr, size_t size);

static void remove_block_from_free_list(void *block_ptr);
static void* extend_heap(size_t size);

static void* coalesce(void* ptr);


int mm_init(void)
{
    heap_ptr = mem_sbrk(INIT_SIZE + BLOCKSIZE);
    if(heap_ptr == (void*)-1)
        return -1;
    DEREF(heap_ptr) = BLOCKSIZE | 1;
    DEREF(heap_ptr + WORDSIZE ) = BLOCKSIZE | 0;
    DEREF(heap_ptr + 2 * WORDSIZE) = 0 | 0;
    DEREF(heap_ptr + 3 * WORDSIZE) = 0 | 0;
    DEREF(heap_ptr + 4 * WORDSIZE) = BLOCKSIZE | 0;
    DEREF(heap_ptr + 5 * WORDSIZE) = 0 | 1;
    free_list_ptr = heap_ptr + 2 * WORDSIZE;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t need_size;
    char *free_block_ptr = NULL;
    if(size == 0)
        return NULL;

    need_size = ((ALIGN(size) + 2 * WORDSIZE) > BLOCKSIZE) ? (ALIGN(size) + 2 * WORDSIZE) : BLOCKSIZE;
    free_block_ptr = find_fit(need_size);     
    if(free_block_ptr){
        allocate(free_block_ptr, need_size);
    }
    else{
        free_block_ptr = extend_heap(ALIGN(need_size));
        if(free_block_ptr == NULL)
            return NULL;
        allocate(free_block_ptr, need_size);
    }
    return free_block_ptr;
}

/*
 * find_fit
*/
static void *find_fit(size_t size){
    void* free_block_ptr = free_list_ptr;
    while((free_block_ptr!=NULL) && (size>get_size(free_block_ptr))){
        free_block_ptr = NEXT_FREE(free_block_ptr);
    }
    // for (free_block_ptr = free_list_ptr; free_block_ptr != NULL; free_block_ptr = NEXT_FREE(free_block_ptr) ){
    //     if(size <= get_size(free_block_ptr))
    //         return free_block_ptr;
    // }
    return free_block_ptr;
}

static void allocate(void* block_ptr, size_t size){
    void *ptr;
    size_t total_size = get_size(get_header(block_ptr));
     if(total_size >= size + BLOCKSIZE){
         DEREF(get_header(block_ptr)) = size | 1;
         DEREF(get_footer(block_ptr)) = size | 1;
         remove_block_from_free_list(block_ptr);
         block_ptr = get_next_block(block_ptr);
         DEREF(get_header(block_ptr)) = (total_size - size) | 0;
         DEREF(get_footer(block_ptr)) = (total_size - size) | 0;
     }
     else{
         DEREF(get_header(block_ptr)) = BLOCKSIZE | 1;
         DEREF(get_footer(block_ptr)) = BLOCKSIZE | 1;
         remove_block_from_free_list(block_ptr);
     }
}

static void remove_block_from_free_list(void *block_ptr){
    if(block_ptr==NULL)
        return;
    if(PREV_FREE(block_ptr) != NULL){
        NEXT_FREE(PREV_FREE(block_ptr)) = NEXT_FREE(block_ptr);
    }
    else
    {
      free_list_ptr = NEXT_FREE(block_ptr);
    } 
    if(NEXT_FREE(block_ptr)!=NULL){
        if(PREV_FREE(block_ptr))
            PREV_FREE(NEXT_FREE(block_ptr)) = PREV_FREE(block_ptr);
    }
}
static void* extend_heap(size_t size){
    char *free_block_ptr = NULL;
    if(size == 0)
        size = BLOCKSIZE;

    free_block_ptr = mem_sbrk(size);
    if(free_block_ptr == (void*)-1)
        return NULL;

    // DEREF(free_block_ptr) = size | 0;
    // DEREF(free_block_ptr+size - WORDSIZE) = size | 0;
    // DEREF(free_block_ptr+size) = 0 | 1;

    DEREF(get_header(free_block_ptr)) = size | 0;
    DEREF(get_footer(free_block_ptr)) = size | 0;
    DEREF(get_next_block(free_block_ptr)) = 0 | 1;

    coalesce(free_block_ptr);
    return free_block_ptr;
}
static void* coalesce(void* block_ptr){
    void *ptr;
    size_t prev_alloc = get_is_alloc(get_footer(get_next_block(block_ptr))) || get_prev_block(block_ptr) == block_ptr;
    size_t next_alloc = get_is_alloc(get_header(get_next_block(block_ptr)));

    // Get the size of the current free block
    size_t size = get_size(get_header(block_ptr));
    /* If the next block is free, then coalesce the current block
    * (block_ptr) and the next block */
    if (prev_alloc && !next_alloc) {           // Case 2 (in text) 
        size += get_size(get_header(get_next_block(block_ptr)));  
        remove_block_from_free_list(get_next_block(block_ptr));
        DEREF(get_header(block_ptr)) =  size | 0;
        DEREF(get_footer(block_ptr)) =  size | 0;
    }

    /* If the previous block is free, then coalesce the current
    * block (block_ptr) and the previous block */
    else if (!prev_alloc && next_alloc) {      // Case 3 (in text) 
        size += get_size(get_header(get_prev_block(block_ptr)));
        block_ptr = get_prev_block(block_ptr); 
        remove_freeblock(block_ptr);
        DEREF(get_header(block_ptr)) =  size | 0;
        DEREF(get_footer(block_ptr)) = size |0;
    } 

    /* If the previous block and next block are free, coalesce
    * both */
    else if (!prev_alloc && !next_alloc) {     // Case 4 (in text) 
        size += get_size(get_header(get_prev_block(block_ptr))) + 
                get_size(get_header(NEXT_BLKP(block_ptr)));
        remove_freeblock(get_prev_block(block_ptr));
        remove_freeblock(NEXT_BLKP(block_ptr));
        block_ptr = get_prev_block(block_ptr);
        DEREF(get_header(block_ptr)) =  size | 0;
        DEREF(get_footer(block_ptr)) = size | 0;
    }

    // Insert the coalesced block at the front of the free list 
    NEXT_FREE(block_ptr) = free_list_ptr;
    PREV_FREE(free_list_ptr) = block_ptr;
    PREV_FREE(block_ptr) = NULL;
    free_list_ptr = block_ptr;

    // Return the coalesced block 
    return block_ptr;
    
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    if(ptr == NULL)
        return;
    size_t size = DEREF(get_header(ptr)) & (~0x1);
    DEREF(get_header(ptr)) = size | 0;
    DEREF(get_footer(ptr)) = size | 0;

    coalesce(ptr);
}


/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    if(ptr == NULL)
        return mm_malloc(size);
    if (size == 0){
        mm_free(ptr);
        return NULL;
    }
}














