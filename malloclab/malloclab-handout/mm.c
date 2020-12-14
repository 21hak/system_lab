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
#define MAX(x, y) ((x) > (y)? (x) : (y))
#define DEREF(p) (*(size_t*)(p))
#define NEXT_FREE(block_ptr) (*(void **)block_ptr)
#define PREV_FREE(block_ptr) (*(void **)(block_ptr + WORDSIZE))


#define MINBLOCKSIZE 16
#define INITSIZE      16 
#define WORDSIZE       4 
#define PACK(size, alloc) ((size) | (alloc))
#define GET(p)        (*(size_t *)(p))
#define PUT(p, val)   (*(size_t *)(p) = (val))
// #define GET_SIZE(p)  (GET(p) & ~0x1)
// #define GET_ALLOC(p) (GET(p) & 0x1)
// #define HDRP(bp)     ((void *)(bp) - WORDSIZE)
// #define FTRP(bp)     ((void *)(bp) + get_size(get_header(bp)) - 2*WORDSIZE)

// #define NEXT_BLKP(bp) ((void *)(bp) + get_size(get_header(bp)))
// #define PREV_BLKP(bp) ((void *)(bp) - get_size(get_header(bp) - WORDSIZE))

/* 
 * mm_init - initialize the malloc package.
 */
// static char* heap_ptr = 0;
// static char* free_list_ptr = 0;
static char *heap_listp = 0;  /* Points to the start of the heap */
static char *free_listp = 0;  /* Poitns to the frist free block */

static inline size_t get_size(void *header_ptr){
    return DEREF(header_ptr) & (~0x1);
}

static inline size_t get_is_alloc(void* header_ptr){
    return DEREF(header_ptr) & (0x1);
}

static inline void *get_header(void* block_ptr){
    return  ((void *)(block_ptr) - WORDSIZE);
}

static inline void* get_footer(void* block_ptr){
    return  ((void *)(block_ptr) + get_size(get_header(block_ptr)) - 2*WORDSIZE);
}

static inline void* get_next_block(void* block_ptr){
    return ((void *)(block_ptr) + get_size(get_header(block_ptr)));
}

static inline void* get_prev_block(void* block_ptr){
    return ((void *)(block_ptr) - get_size(get_header(block_ptr) - WORDSIZE));
}

static void *extend_heap(size_t words);
static void *find_fit(size_t size);
static void *coalesce(void *bp);
static void place(void *bp, size_t asize);
static void remove_freeblock(void *bp);




// static void *find_fit(size_t size);
// static void allocate(void* block_ptr, size_t size);
// static void remove_block_from_free_list(void *block_ptr);
// static void* extend_heap(size_t size);
// static void* coalesce(void* ptr);


// int mm_init(void)
// {
//     heap_ptr = mem_sbrk(INIT_SIZE + BLOCKSIZE);
//     if(heap_ptr == (void*)-1)
//         return -1;
//     DEREF(heap_ptr) = (BLOCKSIZE | 1);
//     DEREF(heap_ptr + WORDSIZE ) = (BLOCKSIZE | 0);
//     DEREF(heap_ptr + 2 * WORDSIZE) = (0 | 0);
//     DEREF(heap_ptr + 3 * WORDSIZE) = (0 | 0);
//     DEREF(heap_ptr + 4 * WORDSIZE) = (BLOCKSIZE | 0);
//     DEREF(heap_ptr + 5 * WORDSIZE) = (0 | 1);
//     free_list_ptr = heap_ptr +  WORDSIZE;
//     return 0;
// }

// /* 
//  * mm_malloc - Allocate a block by incrementing the brk pointer.
//  *     Always allocate a block whose size is a multiple of the alignment.
//  */
// void *mm_malloc(size_t size)
// {
//     // size_t need_size;
//     // char *free_block_ptr = NULL;
//     // if(size == 0)
//     //     return NULL;

//     // need_size = ((ALIGN(size) + 2 * WORDSIZE) > BLOCKSIZE) ? (ALIGN(size) + 2 * WORDSIZE) : BLOCKSIZE;
//     // free_block_ptr = find_fit(need_size);     
//     // if(free_block_ptr){
//     //     allocate(free_block_ptr, need_size);
//     // }
//     // else{
//     //     free_block_ptr = extend_heap(need_size/WORDSIZE);
//     //     if(free_block_ptr == NULL)
//     //         return NULL;
//     //     allocate(free_block_ptr, need_size);
//     // }
//     // return free_block_ptr;
//     if (size == 0)
//       return NULL;

//     size_t asize;       // Adjusted block size 
//     size_t extendsize;  // Amount to extend heap by if no fit 
//     char *block_ptr;

//     /* The size of the new block is equal to the size of the header and footer, plus
//     * the size of the payload. Or BLOCKSIZE if the requested size is smaller.
//     */
//     asize = MAX(ALIGN(size) + 2*WORDSIZE, BLOCKSIZE);
    
//     // Search the free list for the fit 
//     if ((block_ptr = find_fit(asize))) {
//         allocate(block_ptr, asize);
//         return block_ptr;
//     }

//     // Otherwise, no fit was found. Grow the heap larger. 
//     extendsize = MAX(asize, BLOCKSIZE);
//     if ((block_ptr = extend_heap(extendsize/WORDSIZE)) == NULL)
//         return NULL;

//     // Place the newly allocated block
//     allocate(block_ptr, asize);

//     return block_ptr;
// }

// /*
//  * find_fit
// */
// static void *find_fit(size_t size){
//     // void* free_block_ptr = free_list_ptr;
//     // while((free_block_ptr!=NULL) && (size>get_size(free_block_ptr))){
//     //     free_block_ptr = NEXT_FREE(free_block_ptr);
//     // }
//     // // for (free_block_ptr = free_list_ptr; free_block_ptr != NULL; free_block_ptr = NEXT_FREE(free_block_ptr) ){
//     // //     if(size <= get_size(free_block_ptr))
//     // //         return free_block_ptr;
//     // // }
//     // return free_block_ptr;
//     void *block_ptr;

//     /* Iterate through the free list and try to find a free block
//     * large enough */
//     for (block_ptr = free_list_ptr; get_is_alloc(get_header(block_ptr)) == 0; block_ptr = NEXT_FREE(block_ptr)) {
//         if (size <= get_size(get_header(block_ptr))) 
//         return block_ptr; 
//     }
//     // Otherwise no free block was large enough
//     return NULL; 
// }

// static void allocate(void* block_ptr, size_t size){
//     // void *ptr;
//     // size_t total_size = get_size(get_header(block_ptr));
//     //  if(total_size >= size + BLOCKSIZE){
//     //      DEREF(get_header(block_ptr)) = size | 1;
//     //      DEREF(get_footer(block_ptr)) = size | 1;
//     //      remove_block_from_free_list(block_ptr);
//     //      block_ptr = get_next_block(block_ptr);
//     //      DEREF(get_header(block_ptr)) = (total_size - size) | 0;
//     //      DEREF(get_footer(block_ptr)) = (total_size - size) | 0;
//     //  }
//     //  else{
//     //      DEREF(get_header(block_ptr)) = BLOCKSIZE | 1;
//     //      DEREF(get_footer(block_ptr)) = BLOCKSIZE | 1;
//     //      remove_block_from_free_list(block_ptr);
//     //  }

//     size_t fsize = get_size(get_header(block_ptr));

//     // Case 1: Splitting is performed 
//     if((fsize - size) >= (BLOCKSIZE)) {

//         DEREF(get_header(block_ptr)) =  (size | 1);
//         DEREF(get_footer(block_ptr)) = (size | 1);
//         remove_block_from_free_list(block_ptr);
//         block_ptr = get_next_block(block_ptr);
//         DEREF(get_header(block_ptr)) = (fsize-size) | 0;
//         DEREF(get_footer(block_ptr)) = (fsize-size) | 0;
//         coalesce(block_ptr);
//     }

//     // Case 2: Splitting not possible. Use the full free block 
//     else {

//         DEREF(get_header(block_ptr)) = (fsize |1);
//         DEREF(get_footer(block_ptr)) = (fsize |1);
//         remove_block_from_free_list(block_ptr);
//     }
// }

// static void remove_block_from_free_list(void *block_ptr){
//     if(block_ptr) {
//         if (PREV_FREE(block_ptr))
//             NEXT_FREE(PREV_FREE(block_ptr)) = NEXT_FREE(block_ptr);
//         else
//             free_list_ptr = NEXT_FREE(block_ptr);
//     if(NEXT_FREE(block_ptr) != NULL)
//         PREV_FREE(NEXT_FREE(block_ptr)) = PREV_FREE(block_ptr);
//   }
// }
// static void* extend_heap(size_t words){
//     // char *free_block_ptr = NULL;
//     // size_t size;
//     // if(size == 0)
//     //     size = BLOCKSIZE;

//     // size = (words % 2) ? (words + 1) * WORDSIZE : words * WORDSIZE;
//     // if (size < BLOCKSIZE)
//     // size = BLOCKSIZE;

//     // free_block_ptr = mem_sbrk(size);
//     // if(free_block_ptr == (void*)-1)
//     //     return NULL;

//     // // DEREF(free_block_ptr) = size | 0;
//     // // DEREF(free_block_ptr+size - WORDSIZE) = size | 0;
//     // // DEREF(free_block_ptr+size) = 0 | 1;

//     // DEREF(get_header(free_block_ptr)) = size | 0;
//     // DEREF(get_footer(free_block_ptr)) = size | 0;
//     // DEREF(get_header(get_next_block(free_block_ptr))) = 0 | 1;

//     // coalesce(free_block_ptr);
//     // return free_block_ptr;
//     char *bp;
//     size_t asize;

//     /* Adjust the size so the alignment and minimum block size requirements
//     * are met. */ 
//     asize = (words % 2) ? (words + 1) * WORDSIZE : words * WORDSIZE;
//     if (asize < BLOCKSIZE)
//         asize = BLOCKSIZE;
    
//     // Attempt to grow the heap by the adjusted size 
//     if ((bp = mem_sbrk(asize)) == (void *)-1)
//         return NULL;

//     /* Set the header and footer of the newly created free block, and
//     * push the epilogue header to the back */
//     DEREF(get_header(bp)) =  (asize | 0);
//     DEREF(get_footer(bp)) = (asize | 0);
//     DEREF(get_header(get_next_block(bp))) = (0 | 1); /* Move the epilogue to the end */

//     // Coalesce any partitioned free memory 
//     return coalesce(bp); 
// }
// static void* coalesce(void* block_ptr){
//     void *ptr;
//     size_t prev_alloc = get_is_alloc(get_footer(get_next_block(block_ptr))) || get_prev_block(block_ptr) == block_ptr;
//     size_t next_alloc = get_is_alloc(get_header(get_next_block(block_ptr)));

//     // Get the size of the current free block
//     size_t size = get_size(get_header(block_ptr));
//     /* If the next block is free, then coalesce the current block
//     * (block_ptr) and the next block */
//     if (prev_alloc && !next_alloc) {           // Case 2 (in text) 
//         size += get_size(get_header(get_next_block(block_ptr)));  
//         remove_block_from_free_list(get_next_block(block_ptr));
//         DEREF(get_header(block_ptr)) =  size | 0;
//         DEREF(get_footer(block_ptr)) =  size | 0;
//     }

//     /* If the previous block is free, then coalesce the current
//     * block (block_ptr) and the previous block */
//     else if (!prev_alloc && next_alloc) {      // Case 3 (in text) 
//         size += get_size(get_header(get_prev_block(block_ptr)));
//         block_ptr = get_prev_block(block_ptr); 
//         remove_block_from_free_list(block_ptr);
//         DEREF(get_header(block_ptr)) =  size | 0;
//         DEREF(get_footer(block_ptr)) = size |0;
//     } 

//     /* If the previous block and next block are free, coalesce
//     * both */
//     else if (!prev_alloc && !next_alloc) {     // Case 4 (in text) 
//         size += get_size(get_header(get_prev_block(block_ptr))) + 
//                 get_size(get_header(get_next_block(block_ptr)));
//         remove_block_from_free_list(get_prev_block(block_ptr));
//         remove_block_from_free_list(get_next_block(block_ptr));
//         block_ptr = get_prev_block(block_ptr);
//         DEREF(get_header(block_ptr)) =  size | 0;
//         DEREF(get_footer(block_ptr)) = size | 0;
//     }

//     // Insert the coalesced block at the front of the free list 
//     NEXT_FREE(block_ptr) = free_list_ptr;
//     PREV_FREE(free_list_ptr) = block_ptr;
//     PREV_FREE(block_ptr) = NULL;
//     free_list_ptr = block_ptr;

//     // Return the coalesced block 
//     return block_ptr;
    
// }

// /*
//  * mm_free - Freeing a block does nothing.
//  */
// void mm_free(void *block_ptr)
// {
//       // Ignore spurious requests 
//     if (!block_ptr)
//         return;

//     size_t size = get_size(get_header(block_ptr));

//     /* Set the header and footer allocated bits to 0, thus
//     * freeing the block */
//     DEREF(get_header(block_ptr)) =  (size | 0);
//     DEREF(get_footer(block_ptr)) = (size | 0);

//     // Coalesce to merge any free blocks and add them to the list 
//     coalesce(block_ptr);
// }


// /*
//  * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
//  */
// void *mm_realloc(void *ptr, size_t size)
// {
//     if(ptr == NULL)
//         return mm_malloc(size);
//     if (size == 0){
//         mm_free(ptr);
//         return NULL;
//     }
// }

int mm_init(void)
{
  // Initialize the heap with freelist prologue/epilogoue and space for the
  // initial free block. (32 bytes total)
  if ((heap_listp = mem_sbrk(INITSIZE + MINBLOCKSIZE)) == (void *)-1)
      return -1; 
  PUT(heap_listp,             PACK(MINBLOCKSIZE, 1));           // Prologue header 
  PUT(heap_listp +    WORDSIZE,  PACK(MINBLOCKSIZE, 0));           // Free block header 

  PUT(heap_listp + (2*WORDSIZE), PACK(0,0));                       // Space for next pointer 
  PUT(heap_listp + (3*WORDSIZE), PACK(0,0));                       // Space for prev pointer 
  
  PUT(heap_listp + (4*WORDSIZE), PACK(MINBLOCKSIZE, 0));           // Free block footer 
  PUT(heap_listp + (5*WORDSIZE), PACK(0, 1));                      // Epilogue header 

  // Point free_list to the first header of the first free block
  free_listp = heap_listp + (WORDSIZE);

  return 0;
}

/*
 * mm_malloc - Allocates a block of memory of memory of the given size aligned to 8-byte
 * boundaries.
 *
 * A block is allocated according to this strategy:
 * (1) If a free block of the given size is found, then allocate that free block and return
 * a pointer to the payload of that block.
 * (2) Otherwise a free block could not be found, so an extension of the heap is necessary.
 * Simply extend the heap and place the allocated block in the new free block.
 */
void *mm_malloc(size_t size)
{  
  
  if (size == 0)
      return NULL;

  size_t asize;       // Adjusted block size 
  size_t extendsize;  // Amount to extend heap by if no fit 
  char *bp;

  /* The size of the new block is equal to the size of the header and footer, plus
   * the size of the payload. Or MINBLOCKSIZE if the requested size is smaller.
   */
  asize = MAX(ALIGN(size) + 2*WORDSIZE, MINBLOCKSIZE);
  
  // Search the free list for the fit 
  if ((bp = find_fit(asize))) {
    place(bp, asize);
    return bp;
  }

  // Otherwise, no fit was found. Grow the heap larger. 
  extendsize = MAX(asize, MINBLOCKSIZE);
  if ((bp = extend_heap(extendsize/WORDSIZE)) == NULL)
    return NULL;

  // Place the newly allocated block
  place(bp, asize);

  return bp;
}

/*
 * mm_free - Frees the block being pointed to by bp.
 *
 * Freeing a block is as simple as setting its allocated bit to 0. After
 * freeing the block, the free blocks should be coalesced to ensure high
 * memory utilization. 
 */
void mm_free(void *bp)
{ 
  
  // Ignore spurious requests 
  if (!bp)
      return;

  size_t size = get_size(get_header(bp));

  /* Set the header and footer allocated bits to 0, thus
   * freeing the block */
  PUT(get_header(bp), PACK(size, 0));
  PUT(get_footer(bp), PACK(size, 0));

  // Coalesce to merge any free blocks and add them to the list 
  coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
  // If ptr is NULL, realloc is equivalent to mm_malloc(size)
  if (ptr == NULL)
    return mm_malloc(size);

  // If size is equal to zero, realloc is equivalent to mm_free(ptr)
  if (size == 0) {
    mm_free(ptr);
    return NULL;
  }
    
  /* Otherwise, we assume ptr is not NULL and was returned by an earlier malloc or realloc call.
   * Get the size of the current payload */
  size_t asize = MAX(ALIGN(size) + 2*WORDSIZE, MINBLOCKSIZE);
  size_t current_size = get_size(get_header(ptr));

  void *bp;
  char *next = get_header(get_next_block(ptr));
  size_t newsize = current_size + get_size(next);

  /* Case 1: Size is equal to the current payload size */
  if (asize == current_size)
    return ptr;

  // Case 2: Size is less than the current payload size 
  if ( asize <= current_size ) {

    if( asize > MINBLOCKSIZE && (current_size - asize) > MINBLOCKSIZE) {  

      PUT(get_header(ptr), PACK(asize, 1));
      PUT(get_footer(ptr), PACK(asize, 1));
      bp = get_next_block(ptr);
      PUT(get_header(bp), PACK(current_size - asize, 1));
      PUT(get_footer(bp), PACK(current_size - asize, 1));
      mm_free(bp);
      return ptr;
    }

    // allocate a new block of the requested size and release the current block
    bp = mm_malloc(asize);
    memcpy(bp, ptr, asize);
    mm_free(ptr);
    return bp;
  }

  // Case 3: Requested size is greater than the current payload size 
  else {

    // next block is unallocated and is large enough to complete the request
    // merge current block with next block up to the size needed and free the 
    // remaining block.
    if ( !get_is_alloc(next) && newsize >= asize ) {

      // merge, split, and release
      remove_freeblock(get_next_block(ptr));
      PUT(get_header(ptr), PACK(asize, 1));
      PUT(get_footer(ptr), PACK(asize, 1));
      bp = get_next_block(ptr);
      PUT(get_header(bp), PACK(newsize-asize, 1));
      PUT(get_footer(bp), PACK(newsize-asize, 1));
      mm_free(bp);
      return ptr;
    }  
    
    // otherwise allocate a new block of the requested size and release the current block
    bp = mm_malloc(asize); 
    memcpy(bp, ptr, current_size);
    mm_free(ptr);
    return bp;
  }

}


/*
 * extend_heap - Extends the heap by the given number of words rounded up to the 
 * nearest even integer.
 */
static void *extend_heap(size_t words)
{
  char *bp;
  size_t asize;

  /* Adjust the size so the alignment and minimum block size requirements
   * are met. */ 
  asize = (words % 2) ? (words + 1) * WORDSIZE : words * WORDSIZE;
  if (asize < MINBLOCKSIZE)
    asize = MINBLOCKSIZE;
  
  // Attempt to grow the heap by the adjusted size 
  if ((bp = mem_sbrk(asize)) == (void *)-1)
    return NULL;

  /* Set the header and footer of the newly created free block, and
   * push the epilogue header to the back */
  PUT(get_header(bp), PACK(asize, 0));
  PUT(get_footer(bp), PACK(asize, 0));
  PUT(get_header(get_next_block(bp)), PACK(0, 1)); /* Move the epilogue to the end */

  // Coalesce any partitioned free memory 
  return coalesce(bp); 
}

/*
 * find_fit - Attempts to find a free block of at least the given size in the free list.
 *
 * This function implements a first-fit search strategy for an explicit free list, which 
 * is simply a doubly linked list of free blocks.
 */
static void *find_fit(size_t size)
{
  // First-fit search 
  void *bp;

  /* Iterate through the free list and try to find a free block
   * large enough */
  for (bp = free_listp; get_is_alloc(get_header(bp)) == 0; bp = NEXT_FREE(bp)) {
    if (size <= get_size(get_header(bp))) 
      return bp; 
  }
  // Otherwise no free block was large enough
  return NULL; 
}

/*
 * remove_freeblock - Removes the given free block pointed to by bp from the free list.
 * 
 * The explicit free list is simply a doubly linked list. This function performs a removal
 * of the block from the doubly linked free list.
 */
static void remove_freeblock(void *bp)
{
  if(bp) {
    if (PREV_FREE(bp))
      NEXT_FREE(PREV_FREE(bp)) = NEXT_FREE(bp);
    else
      free_listp = NEXT_FREE(bp);
    if(NEXT_FREE(bp) != NULL)
      PREV_FREE(NEXT_FREE(bp)) = PREV_FREE(bp);
  }
}



/*
 * coalesce - Coalesces the memory surrounding block bp using the Boundary Tag strategy
 * proposed in the text (Page 851, Section 9.9.11).
 * 
 * Adjancent blocks which are free are merged together and the aggregate free block
 * is added to the free list. Any individual free blocks which were merged are removed
 * from the free list.
 */
static void *coalesce(void *bp)
{
  // Determine the current allocation state of the previous and next blocks 
  size_t prev_alloc = get_is_alloc(get_footer(get_prev_block(bp))) || get_prev_block(bp) == bp;
  size_t next_alloc = get_is_alloc(get_header(get_next_block(bp)));

  // Get the size of the current free block
  size_t size = get_size(get_header(bp));

  /* If the next block is free, then coalesce the current block
   * (bp) and the next block */
  if (prev_alloc && !next_alloc) {           // Case 2 (in text) 
    size += get_size(get_header(get_next_block(bp)));  
    remove_freeblock(get_next_block(bp));
    PUT(get_header(bp), PACK(size, 0));
    PUT(get_footer(bp), PACK(size, 0));
  }

  /* If the previous block is free, then coalesce the current
   * block (bp) and the previous block */
  else if (!prev_alloc && next_alloc) {      // Case 3 (in text) 
    size += get_size(get_header(get_prev_block(bp)));
    bp = get_prev_block(bp); 
    remove_freeblock(bp);
    PUT(get_header(bp), PACK(size, 0));
    PUT(get_footer(bp), PACK(size, 0));
  } 

  /* If the previous block and next block are free, coalesce
   * both */
  else if (!prev_alloc && !next_alloc) {     // Case 4 (in text) 
    size += get_size(get_header(get_prev_block(bp))) + 
            get_size(get_header(get_next_block(bp)));
    remove_freeblock(get_prev_block(bp));
    remove_freeblock(get_next_block(bp));
    bp = get_prev_block(bp);
    PUT(get_header(bp), PACK(size, 0));
    PUT(get_footer(bp), PACK(size, 0));
  }

  // Insert the coalesced block at the front of the free list 
  NEXT_FREE(bp) = free_listp;
  PREV_FREE(free_listp) = bp;
  PREV_FREE(bp) = NULL;
  free_listp = bp;

  // Return the coalesced block 
  return bp;
}

/*
 * place - Places a block of the given size in the free block pointed to by the given
 * pointer bp.
 *
 * This placement is done using a split strategy. If the difference between the size of block 
 * being allocated (asize) and the total size of the free block (fsize) is greater than or equal
 * to the mimimum block size, then the block is split into two parts. The first block is the 
 * allocated block of size asize, and the second block is the remaining free block with a size
 * corresponding to the difference between the two block sizes.  
 */
static void place(void *bp, size_t asize)
{  
  // Gets the total size of the free block 
  size_t fsize = get_size(get_header(bp));

  // Case 1: Splitting is performed 
  if((fsize - asize) >= (MINBLOCKSIZE)) {

    PUT(get_header(bp), PACK(asize, 1));
    PUT(get_footer(bp), PACK(asize, 1));
    remove_freeblock(bp);
    bp = get_next_block(bp);
    PUT(get_header(bp), PACK(fsize-asize, 0));
    PUT(get_footer(bp), PACK(fsize-asize, 0));
    coalesce(bp);
  }

  // Case 2: Splitting not possible. Use the full free block 
  else {

    PUT(get_header(bp), PACK(fsize, 1));
    PUT(get_footer(bp), PACK(fsize, 1));
    remove_freeblock(bp);
  }
}

// consistency checker

// static int mm_check() {

//   // Is every block in the free list marked as free?
//   void *next;
//   for (next = free_listp; get_is_alloc(get_header(next)) == 0; next = NEXT_FREE(next)) {
//     if (get_is_alloc(get_header(next))) {
//       printf("Consistency error: block %p in free list but marked allocated!", next);
//       return 1;
//     }
//   }

//   // Are there any contiguous free blocks that escaped coalescing?
//   for (next = free_listp; get_is_alloc(get_header(next)) == 0; next = NEXT_FREE(next)) {

//     char *prev = PREV_FREE(get_header(next));
//       if(prev != NULL && get_header(next) - get_footer(prev) == 2*WORDSIZE) {
//         printf("Consistency error: block %p missed coalescing!", next);
//         return 1;
//       }
//   }

//   // Do the pointers in the free list point to valid free blocks?
//   for (next = free_listp; get_is_alloc(get_header(next)) == 0; next = NEXT_FREE(next)) {

//     if(next < mem_heap_lo() || next > mem_heap_hi()) {
//       printf("Consistency error: free block %p invalid", next);
//       return 1;
//     }
//   }

//   // Do the pointers in a heap block point to a valid heap address?
//   for (next = heap_listp; get_next_block(next) != NULL; next = get_next_block(next)) {

//     if(next < mem_heap_lo() || next > mem_heap_hi()) {
//       printf("Consistency error: block %p outside designated heap space", next);
//       return 1;
//     }
//   }

//   return 0;
// }














