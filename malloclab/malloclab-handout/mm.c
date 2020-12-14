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
#define WORDSIZE       4 
#define MAX(x, y) ((x) > (y)? (x) : (y))
#define DEREF(p) (*(size_t*)(p))

#define PACK(size, alloc) ((size) | (alloc))
#define NEXT_FREE(block_ptr) (*(void **)block_ptr)
#define PREV_FREE(block_ptr) (*(void **)(block_ptr + WORDSIZE))


static char *heap_list_ptr = 0;  
static char *free_list_ptr = 0;  

// get size of the block
static inline size_t get_size(void *header_ptr){
    return DEREF(header_ptr) & (~0x1);
}

// get alloc bit of the block
static inline size_t get_is_alloc(void* header_ptr){
    return DEREF(header_ptr) & (0x1);
}

// get header pointer of the block
static inline void *get_header(void* block_ptr){
    return  ((void *)(block_ptr) - WORDSIZE);
}

// get footer pointer of the block
static inline void* get_footer(void* block_ptr){
    return  ((void *)(block_ptr) + get_size(get_header(block_ptr)) - 2*WORDSIZE);
}

// get next block pointer
static inline void* get_next_block(void* block_ptr){
    return ((void *)(block_ptr) + get_size(get_header(block_ptr)));
}

// get previous block pointer
static inline void* get_prev_block(void* block_ptr){
    return ((void *)(block_ptr) - get_size(get_header(block_ptr) - WORDSIZE));
}

static void *extend_heap(size_t words);
static void *find_fit(size_t size);
static void *coalesce(void *block_ptr);
static void allocate(void *block_ptr, size_t asize);
static void remove_block_from_free_list(void *block_ptr);
static void insert_block_to_free_list(void *block_ptr);


int mm_init(void)
{
//   if ((heap_list_ptr = mem_sbrk(INIT_SIZE + BLOCKSIZE)) == (void *)-1)
//       return -1; 
//   DEREF(heap_list_ptr) = (BLOCKSIZE | 1);           // Prologue header 
//   DEREF(heap_list_ptr + WORDSIZE)=  (BLOCKSIZE | 0);           // Free block header 

//   DEREF(heap_list_ptr + (2*WORDSIZE))= (0 | 0);                       // Space for next pointer 
//   DEREF(heap_list_ptr + (3*WORDSIZE))= (0 | 0);                       // Space for prev pointer 
//   DEREF(heap_list_ptr + (4*WORDSIZE))= (BLOCKSIZE | 0);           // Free block footer 
//   DEREF(heap_list_ptr + (5*WORDSIZE))= (0 |  1);                      // Epilogue header 

//   free_list_ptr = heap_list_ptr + (WORDSIZE);

    if ((heap_list_ptr = mem_sbrk(8*WORDSIZE)) == NULL) 
        return -1;

    DEREF(heap_list_ptr) =  0;                            /* Alignment padding */
    DEREF(heap_list_ptr + (1 * WORDSIZE))= (2*WORDSIZE | 1); /* Prologue header */ 
    DEREF(heap_list_ptr + (2 * WORDSIZE))= (2*WORDSIZE | 1); /* Prologue footer */ 
    DEREF(heap_list_ptr + (3 * WORDSIZE))= (0 | 1);     /* Epilogue header */
    free_list_ptr = heap_list_ptr + 2*WORDSIZE; 

    /* Extend the empty heap with a free block of minimum possible block size */
    if (extend_heap(4) == NULL){ 
        return -1;
    }

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
 * Simply extend the heap and allocate the allocated block in the new free block.
 */
void *mm_malloc(size_t size)
{  
  
  
  size_t asize;       // Adjusted block size 
  size_t extendsize;  // Amount to extend heap by if no fit 
  char *block_ptr;

  if (size == 0)
      return NULL;


  /* The size of the new block is equal to the size of the header and footer, plus
   * the size of the payload. Or BLOCKSIZE if the requested size is smaller.
   */
  asize = MAX(ALIGN(size) + 2*WORDSIZE, BLOCKSIZE);
  
  // Search the free list for the fit 
  if ((block_ptr = find_fit(asize))) {
    allocate(block_ptr, asize);
    return block_ptr;
  }

  // Otherwise, no fit was found. Grow the heap larger. 
  extendsize = MAX(asize, BLOCKSIZE);
  if ((block_ptr = extend_heap(extendsize/WORDSIZE)) == NULL)
    return NULL;

  // Place the newly allocated block
  allocate(block_ptr, asize);

  return block_ptr;
}

/*
 * mm_free - Frees the block being pointed to by block_ptr.
 *
 * Freeing a block is as simple as setting its allocated bit to 0. After
 * freeing the block, the free blocks should be coalesced to ensure high
 * memory utilization. 
 */
void mm_free(void *block_ptr)
{ 
  
  // Ignore spurious requests 
  if (!block_ptr)
      return;

  size_t size = get_size(get_header(block_ptr));

  /* Set the header and footer allocated bits to 0, thus
   * freeing the block */
  DEREF(get_header(block_ptr)) = (size | 0);
  DEREF(get_footer(block_ptr)) = (size | 0);

  // Coalesce to merge any free blocks and add them to the list 
  coalesce(block_ptr);
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
  size_t asize = MAX(ALIGN(size) + 2*WORDSIZE, BLOCKSIZE);
  size_t current_size = get_size(get_header(ptr));

  void *block_ptr;
//   char *next = get_header(get_next_block(ptr));
//   size_t newsize = current_size + get_size(next);

  /* Case 1: Size is equal to the current payload size */
  if (asize == current_size)
    return ptr;

  // Case 2: Size is less than the current payload size 
  if ( asize <= current_size ) {

    if( asize > BLOCKSIZE && (current_size - asize) > BLOCKSIZE) {  

      DEREF(get_header(ptr))= (asize | 1);
      DEREF(get_footer(ptr))= (asize | 1);
      block_ptr = get_next_block(ptr);
      DEREF(get_header(block_ptr))=((current_size - asize) | 1);
      DEREF(get_footer(block_ptr))=((current_size - asize) | 1);
      mm_free(block_ptr);
      return ptr;

    //    DEREF(get_header(block_ptr))= (asize | 1);
    // DEREF(get_footer(block_ptr))= (asize | 1);
    // remove_block_from_free_list(block_ptr);
    // block_ptr = get_next_block(block_ptr);
    // DEREF(get_header(block_ptr))= ((fsize-asize) | 0);
    // DEREF(get_footer(block_ptr))= ((fsize-asize) | 0);
    }

    // allocate a new block of the requested size and release the current block
    block_ptr = mm_malloc(asize);
    memcpy(block_ptr, ptr, asize);
    mm_free(ptr);
    return block_ptr;
  }

  // Case 3: Requested size is greater than the current payload size 
  else {

    // next block is unallocated and is large enough to complete the request
    // merge current block with next block up to the size needed and free the 
    // remaining block.
    // if ( !get_is_alloc(next) && newsize >= asize ) {

    //   // merge, split, and release
    //   remove_block_from_free_list(get_next_block(ptr));
    //   DEREF(get_header(ptr))= (asize | 1);
    //   DEREF(get_footer(ptr))= (asize | 1);
    //   block_ptr = get_next_block(ptr);
    //   DEREF(get_header(block_ptr))=((newsize-asize) | 1);
    //   DEREF(get_footer(block_ptr))=((newsize-asize) | 1);
    //   mm_free(block_ptr);
    //   return ptr;
    // }  
    
    // otherwise allocate a new block of the requested size and release the current block
    block_ptr = mm_malloc(asize); 
    memcpy(block_ptr, ptr, current_size);
    mm_free(ptr);
    return block_ptr;
  }

}


/*
 * extend_heap - Extends the heap by the given number of words rounded up to the 
 * nearest even integer.
 */
static void *extend_heap(size_t words)
{
  char *block_ptr;
  size_t asize;

  /* Adjust the size so the alignment and minimum block size requirements
   * are met. */ 
  asize = (words % 2) ? (words + 1) * WORDSIZE : words * WORDSIZE;
  if (asize < BLOCKSIZE)
    asize = BLOCKSIZE;
  
  // Attempt to grow the heap by the adjusted size 
  if ((block_ptr = mem_sbrk(asize)) == (void *)-1)
    return NULL;

  /* Set the header and footer of the newly created free block, and
   * push the epilogue header to the back */
  DEREF(get_header(block_ptr))= (asize | 0);
  DEREF(get_footer(block_ptr))=(asize | 0);
  DEREF(get_header(get_next_block(block_ptr)))=(0 | 1); /* Move the epilogue to the end */

  // Coalesce any partitioned free memory 
  return coalesce(block_ptr); 
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
  void *block_ptr;

  /* Iterate through the free list and try to find a free block
   * large enough */
  for (block_ptr = free_list_ptr; get_is_alloc(get_header(block_ptr)) == 0; block_ptr = NEXT_FREE(block_ptr)) {
    if (size <= get_size(get_header(block_ptr))) 
      return block_ptr; 
  }
  // Otherwise no free block was large enough
  return NULL; 
}

/*
 * remove_block_from_free_list - Removes the given free block pointed to by block_ptr from the free list.
 * 
 * The explicit free list is simply a doubly linked list. This function performs a removal
 * of the block from the doubly linked free list.
 */
static void remove_block_from_free_list(void *block_ptr)
{
  if(block_ptr) {
    if (PREV_FREE(block_ptr))
      NEXT_FREE(PREV_FREE(block_ptr)) = NEXT_FREE(block_ptr);
    else
      free_list_ptr = NEXT_FREE(block_ptr);
    if(NEXT_FREE(block_ptr) != NULL)
      PREV_FREE(NEXT_FREE(block_ptr)) = PREV_FREE(block_ptr);
  }
}

static void insert_block_to_free_list(void *block_ptr){
  NEXT_FREE(block_ptr) = free_list_ptr;
  PREV_FREE(free_list_ptr) = block_ptr;
  PREV_FREE(block_ptr) = NULL;
  free_list_ptr = block_ptr;
}

/*
 * coalesce - Coalesces the memory surrounding block block_ptr using the Boundary Tag strategy
 * proposed in the text (Page 851, Section 9.9.11).
 * 
 * Adjancent blocks which are free are merged together and the aggregate free block
 * is added to the free list. Any individual free blocks which were merged are removed
 * from the free list.
 */
static void *coalesce(void *block_ptr)
{
  // Determine the current allocation state of the previous and next blocks 
  size_t prev_alloc = get_is_alloc(get_footer(get_prev_block(block_ptr))) || get_prev_block(block_ptr) == block_ptr;
  size_t next_alloc = get_is_alloc(get_header(get_next_block(block_ptr)));

  // Get the size of the current free block
  size_t size = get_size(get_header(block_ptr));

  /* If the next block is free, then coalesce the current block
   * (block_ptr) and the next block */
  if (prev_alloc && !next_alloc) {           // Case 2 (in text) 
    size += get_size(get_header(get_next_block(block_ptr)));  
    remove_block_from_free_list(get_next_block(block_ptr));
    DEREF(get_header(block_ptr))= (size | 0);
    DEREF(get_footer(block_ptr))= (size | 0);
  }

  /* If the previous block is free, then coalesce the current
   * block (block_ptr) and the previous block */
  else if (!prev_alloc && next_alloc) {      // Case 3 (in text) 
    size += get_size(get_header(get_prev_block(block_ptr)));
    block_ptr = get_prev_block(block_ptr); 
    remove_block_from_free_list(block_ptr);
    DEREF(get_header(block_ptr))= (size | 0);
    DEREF(get_footer(block_ptr))= (size | 0);
  } 

  /* If the previous block and next block are free, coalesce
   * both */
  else if (!prev_alloc && !next_alloc) {     // Case 4 (in text) 
    size += get_size(get_header(get_prev_block(block_ptr))) + 
            get_size(get_header(get_next_block(block_ptr)));
    remove_block_from_free_list(get_prev_block(block_ptr));
    remove_block_from_free_list(get_next_block(block_ptr));
    block_ptr = get_prev_block(block_ptr);
    DEREF(get_header(block_ptr))= (size | 0);
    DEREF(get_footer(block_ptr))= (size | 0);
  }

  // Insert the coalesced block at the front of the free list 
  insert_block_to_free_list(block_ptr);

  // Return the coalesced block 
  return block_ptr;
}

/*
 * allocate - Places a block of the given size in the free block pointed to by the given
 * pointer block_ptr.
 *
 * This placement is done using a split strategy. If the difference between the size of block 
 * being allocated (asize) and the total size of the free block (fsize) is greater than or equal
 * to the mimimum block size, then the block is split into two parts. The first block is the 
 * allocated block of size asize, and the second block is the remaining free block with a size
 * corresponding to the difference between the two block sizes.  
 */
static void allocate(void *block_ptr, size_t asize)
{  
  // Gets the total size of the free block 
  size_t fsize = get_size(get_header(block_ptr));

  // Case 1: Splitting is performed 
  if((fsize - asize) >= (BLOCKSIZE)) {

    DEREF(get_header(block_ptr))= (asize | 1);
    DEREF(get_footer(block_ptr))= (asize | 1);
    remove_block_from_free_list(block_ptr);
    block_ptr = get_next_block(block_ptr);
    DEREF(get_header(block_ptr))= ((fsize-asize) | 0);
    DEREF(get_footer(block_ptr))= ((fsize-asize) | 0);
    coalesce(block_ptr);
  }

  // Case 2: Splitting not possible. Use the full free block 
  else {

    DEREF(get_header(block_ptr))= (fsize | 1);
    DEREF(get_footer(block_ptr))= (fsize | 1);
    remove_block_from_free_list(block_ptr);
  }
}
