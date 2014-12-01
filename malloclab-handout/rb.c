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

#define ALIGNMENT 	8
#define ALIGN(size)	(((size) + (0x7)) & (~0x7))

#define MM_ALIGN	3
#define MM_ARCH		32

#define MM_ALLOC_FREE	0
#define MM_ALLOC_ALLOC	1
#define MM_COLOR_BLACK	0
#define MM_COLOR_RED	1
#define MM_NULL_NOTNULL	0
#define MM_NULL_NULL	1

#define MM_HEADER_SIZE	8
#define MM_NODE_SIZE	16
#define MM_DATA_SIZE	24

struct MM_Node
{
	struct MM_Node *	prev;
	size_t				size:	MM_ARCH - MM_ALIGN;
	unsigned int		alloc:	1;
	unsigned int		color:	1;
	unsigned int		null:	1;
	struct MM_Node *	left;
	struct MM_Node *	right;
};

struct MM_Data
{
	struct MM_Node *	root;
	struct MM_Node 		null;
};

struct MM_Stack
{
	struct MM_Node *	node;
	struct MM_Stack *	parent;
};


void *mm_memset(void *b, int c, size_t len)
{
	int i;

	for(i = 0; i < len; i++)
	{
		*((unsigned char *)b + i) = (unsigned char)c;
	}

	return b;
}

bool mm_cmp_node(MM_Node *x, MM_Node *y)
{
	if(x->size == y->size)
	{
		return x < y;
	}
	return x->size < y->size;
}

struct MM_Node *mm_rotate_left(struct MM_Node *now)
{
	MM_Node *tmp;

	tmp = now->right;
	now->right = tmp->left;
	tmp->left = now;
	return tmp;
}

struct MM_Node *mm_rotate_right(struct MM_Node *now)
{
	MM_Node *tmp;

	tmp = now->left;
	now->left = tmp->right;
	tmp->right = now;
	return tmp;
}

struct MM_Node *mm_search_node(struct MM_Node *now, size_t size)
{
	if(now->null == MM_NULL_NULL)
	{
		return NULL;
	}
	if(size <= now->size)
	{
		return now;
	}
	return mm_search_node(now->right);
}

struct MM_Node *mm_insert_node(struct MM_Node *now, struct MM_Node *new)
{
	if(now->null == MM_NULL_NULL)
	{
		new->left = new->right = now;
		new->color = MM_COLOR_RED;
		return new;
	}

	if(mm_cmp_node(new, now))
	{
		now->left = mm_insert_node(now->left, new);

		if(now->left->color == MM_COLOR_RED)
		{
			if(now->left->right->color == MM_COLOR_RED)
			{
				now->left = mm_rotate_left(now->left);
			}
			if(now->left->left->color == MM_COLOR_RED)
			{
				if(now->right->color == MM_COLOR_RED)
				{
					now->left->color = now->right->color = MM_COLOR_BLACK;
					now->color = MM_COLOR_RED;
				}
				else
				{
					now = mm_rotate_right(now);
					now->left->color = now->right->color = MM_COLOR_RED;
					now->color = MM_COLOR_BLACK;
				}
			}
		}
	}
	else
	{
		now->right = mm_insert_node(now->right, new);

		if(now->right->color == MM_COLOR_RED)
		{
			if(now->right->left->color == MM_COLOR_RED)
			{
				now->right = mm_rotate_right(now->right);
			}
			if(now->right->right->color == MM_COLOR_RED)
			{
				if(now->left->color == MM_COLOR_RED)
				{
					now->left->color = now->right->color = MM_COLOR_BLACK;
					now->color = MM_COLOR_RED;
				}
				else
				{
					now = mm_rotate_left(now);
					now->left->color = now->right->color = MM_COLOR_RED;
					now->color = MM_COLOR_BLACK;
				}
			}
		}
	}

	return now;
}

struct MM_Node *mm_delete_delete_node(struct MM_Node *par, MM_Node *now)
{
}

struct MM_Node *mm_delete_search_node(struct MM_Node *now)
{
	if(now->left->null == MM_NULL_NULL)
	{
		return now;
	}
	return mm_delete_search_node(now->left);
}

struct MM_Node *mm_delete_node(struct MM_Node *now, struct MM_Node *del, struct MM_Node *par)
{
	MM_Node *ptr;
	if(now == del)
	{
		if(now->right->null == MM_NULL_NULL)
		{
			now = mm_delete_delete_node(par, now);
		}
		else
		{
			ptr = mm_delete_search_node(now->right);
		}
	}
	else if(cmp(now, del))
	{
		now->left = mm_delete_node(now->left, del, now);
	}
	else
	{
		now->right = mm_delete_node(now->right, del, now);
	}
	return now;
}

struct MM_Node *mm_get_prev(struct MM_Node *ptr)
{
	return ptr->prev;
}

struct MM_Node *mm_get_next(struct MM_Node *ptr)
{
	return (MM_Node *)(((void *)ptr) + ptr->size);
}

int mm_init()
{
	struct MM_Node *node;
	struct MM_Data *data;

	data = (struct MM_Data *)mem_sbrk(MM_DATA_SIZE);
	memset(data, 0, MM_DATA_SIZE);

	data->root = &data->null;
	data->null.color = MM_COLOR_BLACK;
	data->null.null = MM_NULL_NULL;

	node = (struct MM_Node *)mem_sbrk(MM_HEADER_SIZE);
	node->prev = NULL;

	return 0;
}

void *mm_malloc(size_t size)
{
}

void mm_free(void *ptr)
{
	struct MM_Node *now, *prev, *next;

	now = (struct MM_Node *)(ptr - MM_HEADER_SIZE);
	prev = mm_get_prev(now);
	next = mm_get_next(now);

	if(prev != NULL && prev->alloc == MM_ALLOC_FREE)
	{
		prev->size += now->size;
		now = prev;
	}
	if(next->size != 0 && next->alloc == MM_ALLOC_FREE)
	{
		now->size += next->size;
	}

	mm_get_next(now)->prev = now;
}

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
