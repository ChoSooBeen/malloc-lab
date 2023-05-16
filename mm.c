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

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ChoSooBeen",
    /* First member's full name */
    "SOOOOS",
    /* First member's email address */
    "cba0613060@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* -------------- 상수 정의 -------------------------------------------- */
/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8 // 2워드
#define WSIZE 4 // 1워드
#define CHUNKSIZE (1 << 12) //초기 가용 블록과 힙 확장을 위한 기본 크기

/* -------------- 매크로 정의 ------------------------------------------ */
#define MAX(x, y) ((x) > (y) ? (x) : (y)) //큰 값 반환

#define PACK(size, alloc) ((size) | (alloc)) //Header or Footer 에 저장할 값 반환

#define GET(p) (*(unsigned int *)(p)) //p가 참조하는 word 반환
#define PUT(p, val) (*(unsigned int *)(p) = (val)) //p가 가리키는 word에 val 저장

#define GET_SIZE(p) (GET(p) & ~0x7) //p가 가리키는 block size 반환
#define GET_ALLOC(p) (GET(p) & 0x1) //p가 가리키는 block의 할당 비트 반환

#define HDRP(bp) ((char *)(bp) - WSIZE) //bp가 가리키는 block의 Header를 가리키는 포인터 반환
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - ALIGNMENT) //bp가 가리키는 block의 Footer를 가리키는 포인터 반환

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE))) //다음 block pointer 반환
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - ALIGNMENT))) //이전 block pointer 반환

/* -------------- 함수 정의 -------------------------------------------- */
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);

/* -------------전역 변수 선언 --------------------------------------------*/
static void *heap_listp;
static char *nextfit;

/* 
 * mm_init - initialize the malloc package.
 * 할당기를 초기화한다.
 * 성공이면 0 / 아니면 -1 반환
 */
int mm_init(void){
    if ((heap_listp = mem_sbrk(2 * ALIGNMENT)) == (void *)-1) {
        return -1;
    }
    PUT(heap_listp, 0); //Padding Word
    PUT(heap_listp + (1*WSIZE), PACK(ALIGNMENT, 1)); //Prolog Header
    PUT(heap_listp + (2*WSIZE), PACK(ALIGNMENT, 1)); //Prolog Footer
    PUT(heap_listp + (3*WSIZE), PACK(0, 1)); //Epilogue Header - 크기를 0으로 할당
    heap_listp += (2 * WSIZE);

    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) {
        return -1;
    }
    nextfit = heap_listp; //마지막 탐색지점을 저장할 변수 초기화
    return 0;
}

/*
 * 힙을 확장하기 위한 함수
 * 1. 힙을 초기화할 때
 * 2. mm_malloc()이 적당한 fit을 찾지 못했을 때
 * 요청한 크기를 정렬을 유지하기 위해 8(double words)의 배수로 반올림하여 요청
 */
static void *extend_heap(size_t words) {
    char *bp;
    // words가 홀수일 경우 1을 더한 후 4만큼 곱해준다.
    size_t size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

    //실패했을 경우
    if ((long)(bp = mem_sbrk(size)) == -1) {
        return NULL;
    }

    //새로운 가용 블록 생성
    PUT(HDRP(bp), PACK(size, 0)); //가용 블록의 HEADER
    PUT(FTRP(bp), PACK(size, 0)); //가용 블록의 FOOTER
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); //새로운 Epilogue

    //새로 할당 받은 가용 블록 앞에 또다른 가용 블록이 존재할 수 있다.
    return coalesce(bp);
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size){
    size_t asize;
    size_t extendsize;
    char *bp;

    //size가 0이면 필요 없다.
    if (size == 0) {
        return NULL;
    }

    //size가 2 word 보다 작을 경우
    if (size <= ALIGNMENT) {
        asize = 2 * ALIGNMENT;
    }
    else { //클 경우
        asize = ALIGNMENT * ((size + (ALIGNMENT) + (ALIGNMENT - 1)) / ALIGNMENT);
    }

    //할당할 수 있는 가용 영역이 있으면
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    //할당할 영역이 없으므로 힙 영역 확장하기
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL) {
        return NULL;
    }
    place(bp, asize);
    return bp;
}

/*
 * next-fit 검색 수행
 * 83점
 */
static void *find_fit(size_t asize) {
    char *bp = nextfit;
    // epilogue 의 크기는 0이다.
    while (GET_SIZE(HDRP(bp)) > 0) {
        if (!GET_ALLOC(HDRP(bp)) && (GET_SIZE(HDRP(bp)) >= asize)) {
            nextfit = bp;
            return bp;
        }
        bp = NEXT_BLKP(bp);
    }

    //epilogue에서 while문이 끝나면 처음부터 검색
    //단, nextfit보다 전까지만 검색
    bp = heap_listp; //처음으로 초기화
    while (bp < nextfit){
        if (!GET_ALLOC(HDRP(bp)) && (GET_SIZE(HDRP(bp)) >= asize)) {
            nextfit = bp;
            return bp;
        }
        bp = NEXT_BLKP(bp);
    }
    return NULL;
}

static void place(void *bp, size_t asize) {
    size_t size = GET_SIZE(HDRP(bp));
    //최소 블록 크기 = 16byte
    if ((size - asize) >= (2 * (ALIGNMENT))) {
        //앞에서부터 할당하기
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));

        //나머지는 다음 블록에 가용 영역 할당
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(size - asize, 0));
        PUT(FTRP(bp), PACK(size - asize, 0));
        nextfit = bp; //남은 가용 영역을 마지막으로 탐색했다.
    }
    else { //현재 영역 전부 사용
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    //연결할 수 있는 가용 블록이 있으면 연결하기
    coalesce(ptr);
}

static void *coalesce(void *bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))); //이전 블록의 할당 비트
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp))); //다음 블록의 할당 비트
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) { //둘다 할당되어 있는 상태일 때
        return bp;
    }
    else if (prev_alloc && !next_alloc) { //이전 블록만 할당되어 있을 때
        size += GET_SIZE(HDRP(NEXT_BLKP(bp))); //다음 블록 사이즈만큼 증가
        //현재 가용 블록과 다음 가용 블록 더하기
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc) { //다음 블록만 할당되어 있을 때
        size += GET_SIZE(HDRP(PREV_BLKP(bp))); //이전 블록 서이즈만큼 증가
        //현재 가용 블록과 이전 가용 블록 더하기
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        //블록의 앞을 가리켜야 하므로 이전의 블록 포인터가 현재 블록 포인터
        bp = PREV_BLKP(bp);
    }
    else { //둘다 할당 안되어 있을 때
        size += (GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp))));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        //블록의 앞을 가리켜야 하므로 이전의 블록 포인터가 현재 블록 포인터
        bp = PREV_BLKP(bp);
    }
    nextfit = bp; //현재 탐색한 노드 저장
    return bp;
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
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}