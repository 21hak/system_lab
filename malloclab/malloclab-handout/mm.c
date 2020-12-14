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
#define BLOCKSIZE 16
#define WORDSIZE 4 
#define MAX(x, y) ((x) > (y)? (x) : (y))
#define DEREF(p) (*(size_t*)(p))
#define NEXT_FREE(block_ptr) (*(void **)block_ptr)
#define PREV_FREE(block_ptr) (*(void **)(block_ptr + WORDSIZE))


static char *heap_list_ptr = 0;  
static char *free_list_ptr = 0;  

// block size 반환
static inline size_t get_size(void *header_ptr){
    return DEREF(header_ptr) & (~0x1);
}

// alloc bit 반환
static inline size_t get_is_alloc(void* header_ptr){
    return DEREF(header_ptr) & (0x1);
}

// header pointer 반환
static inline void *get_header(void* block_ptr){
    return  ((void *)(block_ptr) - WORDSIZE);
}

// footer pointer 반환
static inline void* get_footer(void* block_ptr){
    return  ((void *)(block_ptr) + get_size(get_header(block_ptr)) - 2*WORDSIZE);
}

// 다음 block pointer 반환
static inline void* get_next_block(void* block_ptr){
    return ((void *)(block_ptr) + get_size(get_header(block_ptr)));
}

// 이전 block pointer 반환
static inline void* get_prev_block(void* block_ptr){
    return ((void *)(block_ptr) - get_size(get_header(block_ptr) - WORDSIZE));
}

static void *extend_heap(size_t adjust_size);
static void *find_fit(size_t size);
static void *coalesce(void *block_ptr);
static void allocate(void *block_ptr, size_t adjust_size);
static void remove_block_from_free_list(void *block_ptr);
static void insert_block_to_free_list(void *block_ptr);

/* 
 * textbook 894p.
 * dynamic memory allocation을 위한 heap list 초기화
 */
int mm_init(void){

    if ((heap_list_ptr = mem_sbrk(8*WORDSIZE)) == NULL) 
        return -1;

    DEREF(heap_list_ptr) =  0;                                // Alignment padding
    DEREF(heap_list_ptr + (1 * WORDSIZE))= (2*WORDSIZE | 1);  // Prologue header 
    DEREF(heap_list_ptr + (2 * WORDSIZE))= (2*WORDSIZE | 1);  // Prologue footer 
    DEREF(heap_list_ptr + (3 * WORDSIZE))= (0 | 1);           // Epilogue header 
    free_list_ptr = heap_list_ptr + 2*WORDSIZE; 

    // 최소 크기 block 생성
    if (extend_heap(BLOCKSIZE) == NULL){ 
        return -1;
    }
  return 0;
}

/*
 * 8byte로 align 되게 memory를 allocate
 */
void *mm_malloc(size_t size){  
    size_t adjust_size;       
    char *block_ptr;

    if (size == 0)
        return NULL;
    // header, footer를 위해 2*WORDSIZE 추가
    adjust_size = MAX(ALIGN(size) + 2*WORDSIZE, BLOCKSIZE);
    
    // free list에서 free block을 찾음
    if ((block_ptr = find_fit(adjust_size))) {
        allocate(block_ptr, adjust_size);
        return block_ptr;
    }

    // 해당 size의 free block이 없는 경우 extend heap
    if ((block_ptr = extend_heap(adjust_size)) == NULL)
        return NULL;

    allocate(block_ptr, adjust_size);
    return block_ptr;
}

/*
 * mm_free - 입력 받은 block pointer가 가리키는 block을 free
 */
void mm_free(void *block_ptr)
{ 
  if (!block_ptr)
      return;
  size_t size = get_size(get_header(block_ptr));

  // allocate bit을 0로 변경
  DEREF(get_header(block_ptr)) = (size | 0);  
  DEREF(get_footer(block_ptr)) = (size | 0);

  // free 한 후 생성된 free block을 coalesce를 통해 다른 free block과 merge
  coalesce(block_ptr);
}

/*
 * mm_realloc - 구현한 mm_malloc과 mm_free를 사용해서 구현
 */
void *mm_realloc(void *ptr, size_t size){
    // ptr이 NULL이면, realloc = mm_malloc(size)
    if (ptr == NULL)
        return mm_malloc(size);

    // size가 0면, realloc is same as mm_free(ptr)
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
        
    size_t adjust_size = MAX(ALIGN(size) + 2*WORDSIZE, BLOCKSIZE); // adjust size
    size_t current_size = get_size(get_header(ptr)); // 현재 block size

    void *block_ptr;
    
    // 현재 size와 adjust size가 같으면 추가 작업 없이 입력받은 ptr 반환
    if (adjust_size == current_size)
        return ptr;

    // adjust size가 현재 size보다 작으면
    if ( adjust_size <= current_size ) {    
        // splitting이 가능한지 확인 후 가능하면 진행.
        if( adjust_size > BLOCKSIZE && (current_size - adjust_size) > BLOCKSIZE) {  
        DEREF(get_header(ptr))= (adjust_size | 1);
        DEREF(get_footer(ptr))= (adjust_size | 1);
        block_ptr = get_next_block(ptr);
        DEREF(get_header(block_ptr))=((current_size - adjust_size) | 1);
        DEREF(get_footer(block_ptr))=((current_size - adjust_size) | 1);
        mm_free(block_ptr);
        return ptr;
        }

        // mm_malloc을 통해 새로운 block을 할당하고 기존 block은 free
        block_ptr = mm_malloc(adjust_size);
        memcpy(block_ptr, ptr, adjust_size);
        mm_free(ptr);
        return block_ptr;
    }
    // if adjust size가 현재 size보다 크면
    else {
        // mm_malloc을 통해 새로운 block을 할당하고 기존 block은 free
        block_ptr = mm_malloc(adjust_size); 
        memcpy(block_ptr, ptr, current_size);
        mm_free(ptr);
        return block_ptr;
    }

}


/*
 * extend_heap - adjust size만큼 heap을 확장
 */
static void *extend_heap(size_t adjust_size)
{
    char *block_ptr;

    if (adjust_size < BLOCKSIZE)
        adjust_size = BLOCKSIZE;

    if ((block_ptr = mem_sbrk(adjust_size)) == (void *)-1)
        return NULL;

    //  새로 할당한 block의 allocate bit을 0으로 setting, epilogue header을 맨 뒤에 위치
    DEREF(get_header(block_ptr))= (adjust_size | 0);
    DEREF(get_footer(block_ptr))=(adjust_size | 0);
    DEREF(get_header(get_next_block(block_ptr)))=(0 | 1);
    return coalesce(block_ptr); 
}

/*
 * find_fit - first-fit을 사용해서 free list에서 free block 찾음.
 */
static void *find_fit(size_t size)
{
    void *block_ptr;
    for (block_ptr = free_list_ptr; get_is_alloc(get_header(block_ptr)) == 0; block_ptr = NEXT_FREE(block_ptr)) {
        if (size <= get_size(get_header(block_ptr))) 
        return block_ptr; 
    }
    return NULL; 
}

/*
 * remove_block_from_free_list - free list에서 해당 block pointer가 가리키는 free block을 삭제
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

/*
 * insert_block_to_free_list - free list의 맨 앞에 해당 block pointer가 가리키는 free block을 추가
 */
static void insert_block_to_free_list(void *block_ptr){
    NEXT_FREE(block_ptr) = free_list_ptr;
    PREV_FREE(free_list_ptr) = block_ptr;
    PREV_FREE(block_ptr) = NULL;
    free_list_ptr = block_ptr;
}

/*
 * coalesce - 인접한 free block을 merge함
 */
static void *coalesce(void *block_ptr)
{
    size_t prev_alloc = get_is_alloc(get_footer(get_prev_block(block_ptr))) || get_prev_block(block_ptr) == block_ptr;
    size_t next_alloc = get_is_alloc(get_header(get_next_block(block_ptr)));

    size_t size = get_size(get_header(block_ptr));

    /* 다음 block만 free한 경우
     * total size = current block size + next block size 
     */
    if (prev_alloc && !next_alloc) {
        size += get_size(get_header(get_next_block(block_ptr)));
        remove_block_from_free_list(get_next_block(block_ptr));
        DEREF(get_header(block_ptr))= (size | 0);
        DEREF(get_footer(block_ptr))= (size | 0);
    }

    /* 이전 block만 free한 경우
    *  total size = current block size + previous block size 
    */
    else if (!prev_alloc && next_alloc) {      
        size += get_size(get_header(get_prev_block(block_ptr)));
        block_ptr = get_prev_block(block_ptr); // block pointer을 업데이트
        remove_block_from_free_list(block_ptr);
        DEREF(get_header(block_ptr))= (size | 0);
        DEREF(get_footer(block_ptr))= (size | 0);
    } 

    /* 이전, 다음 모두 free한 경우
    * both */
    else if (!prev_alloc && !next_alloc) {
        size += get_size(get_header(get_prev_block(block_ptr))) + 
                get_size(get_header(get_next_block(block_ptr)));
        remove_block_from_free_list(get_prev_block(block_ptr));
        remove_block_from_free_list(get_next_block(block_ptr));
        block_ptr = get_prev_block(block_ptr); // block pointer을 업데이트
        DEREF(get_header(block_ptr))= (size | 0);
        DEREF(get_footer(block_ptr))= (size | 0);
    }

    insert_block_to_free_list(block_ptr);
    return block_ptr;
}

/*
 * allocate - 입력 받은 size만큼 block pointer가 가리키는 block을 할당시킴 (free에서 non free로)
 */
static void allocate(void *block_ptr, size_t adjust_size)
{  
  size_t free_size = get_size(get_header(block_ptr));

  // 빈 공간이 BLOCKSIZE보다 큰 경우 splitting을 통해 나머지 공간은 free list에 넣음
  if((free_size - adjust_size) >= (BLOCKSIZE)) {
    DEREF(get_header(block_ptr))= (adjust_size | 1);
    DEREF(get_footer(block_ptr))= (adjust_size | 1);
    remove_block_from_free_list(block_ptr);
    block_ptr = get_next_block(block_ptr);
    DEREF(get_header(block_ptr))= ((free_size-adjust_size) | 0);
    DEREF(get_footer(block_ptr))= ((free_size-adjust_size) | 0);
    coalesce(block_ptr);
  }
  else {

    DEREF(get_header(block_ptr))= (free_size | 1);
    DEREF(get_footer(block_ptr))= (free_size | 1);
    remove_block_from_free_list(block_ptr);
  }
}
