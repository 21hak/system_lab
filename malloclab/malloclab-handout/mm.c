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

/* 
 * mm_init - initialize the malloc package.
 */
static char* heap_ptr = 0;
static char* free_list_ptr = 0;

int mm_init(void)
{
    heap_ptr = mem_sbrk(INIT_SIZE+BLOCKSIZE);
    if(heap_ptr == (void*)-1)
        return -1;
    (*(size_t*))heap_ptr = BLOCKSIZE | 1;
    (*(size_t*))(heap_ptr + WORDSIZE ) = BLOCKSIZE | 0;
    (*(size_t*))(heap_ptr + 2*WORDSIZE) = 0 | 0;
    (*(size_t*))(heap_ptr + 3*WORDSIZE) = 0 | 0;
    (*(size_t*))(heap_ptr + 4*WORDSIZE) = BLOCKSIZE | 0;
    (*(size_t*))(heap_ptr + 5*WORDSIZE) = BLOCKSIZE | 1;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














