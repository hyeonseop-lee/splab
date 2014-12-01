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
	/* Your student ID */
	"20130516",
	/* Your Name */
	"Hyeonseop Lee",
	/* First member's email address */
	"protos37@kaist.ac.kr",
	/* Second member's full name (leave blank) */
	"",
	/* Second member's email address (leave blank) */
	""
};


typedef struct
{
	size_t size;
	void *prev;
	void *next;
} MM_Node;


typedef struct
{
	MM_Node head;
	MM_Node tail;
} MM_Head;


/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))


void *mm_new_node(size_t nsize)
{
	MM_Node *ptr;

	ptr = (MM_Node *)mem_sbrk(nsize);
	ptr->size = nsize;

	return (void *)ptr;
}


void mm_insert_node(MM_Node *at, MM_Node *tg)
{
	MM_Node *p, *h, *n;

	p = (MM_Node *)at->prev;
	h = tg;
	n = at;

	p->next = h;
	h->prev = p;
	h->next = n;
	n->prev = h;
}


void mm_delete_node(MM_Node *at)
{
	MM_Node *p, *h, *n;

	p = (MM_Node *)at->prev;
	h = at;
	n = (MM_Node *)at->next;

	p->next = n;
	n->prev = p;
}


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
	MM_Head *head;

	//printf("mm_init\n");

	head = (MM_Head *)mem_sbrk(ALIGN(sizeof(MM_Head)));
	head->head.size = 0;
	head->head.prev = NULL;
	head->head.next = (void *)&head->tail;
	head->tail.size = 0;
	head->tail.prev = (void *)&head->head;
	head->tail.next = NULL;

	return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *	 Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
	size_t nsize;
	MM_Node *itr, *ntr;
	MM_Head *head;

	//printf("mm_malloc\n");

	head = (MM_Head *)mem_heap_lo();

	nsize = ALIGN(size) + ALIGN(sizeof(size_t));
	if(nsize < ALIGN(sizeof(MM_Node)))
	{
		nsize = ALIGN(sizeof(MM_Node));
	}

	for(itr = (MM_Node *)head->head.next; itr != &head->tail; itr = (MM_Node *)itr->next)
	{
		if(nsize <= itr->size)
		{
			break;
		}
	}

	if(ALIGN(sizeof(MM_Node)) + nsize <= itr->size)
	{
		itr->size -= nsize;
		ntr = (MM_Node *)(((void *)itr) + itr->size);
		ntr->size = nsize;
	}
	else if(nsize <= itr->size)
	{
		mm_delete_node(itr);
		ntr = itr;
	}
	else
	{
		ntr = (MM_Node *)mm_new_node(nsize);
	}

	return (void *)ntr + ALIGN(sizeof(size_t));
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *iptr)
{
	MM_Node *ptr, *itr, *prev, *next;
	MM_Head *head;

	//printf("mm_free\n");

	head = (MM_Head *)mem_heap_lo();

	ptr = (MM_Node *)(iptr - ALIGN(sizeof(size_t)));
	prev = NULL;
	next = NULL;

	for(itr = (MM_Node *)head->head.next; itr != &head->tail; itr = (MM_Node *)itr->next)
	{
		if((void *)itr + itr->size == (void *)ptr)
		{
			prev = itr;
		}
		if((void *)ptr + ptr->size == (void *)itr)
		{
			next = itr;
		}
	}

	if(next != NULL)
	{
		mm_delete_node(next);
		ptr->size += next->size;
	}
	if(prev != NULL)
	{
		prev->size += ptr->size;
	}
	else
	{
		mm_insert_node(&head->tail, ptr);
	}
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
	void *oldptr = ptr;
	void *newptr;
	size_t copySize;

	//printf("mm_realloc\n");
	
	newptr = mm_malloc(size);
	if (newptr == NULL)
	  return NULL;
	copySize = *(size_t *)((char *)oldptr - ALIGN(sizeof(size_t)));
	if (size < copySize)
	  copySize = size;
	memcpy(newptr, oldptr, copySize);
	mm_free(oldptr);
	return newptr;
}
