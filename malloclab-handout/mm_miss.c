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

typedef struct MM_Node * PNODE;
typedef struct MM_Data * PDATA;

void mm_print_node(PNODE now, int depth)
{
	int i;

	if(now->null == MM_NULL_NULL)
	{
		printf("%9d\n", -now->size);
		return;
	}

	printf("%3d%6d", now->color, now->size);
	mm_print_node(now->left, depth + 1);
	if(now->left->null == MM_NULL_NOTNULL && now->size < now->left->size)
	{
		printf("ERROR\n");
	}

	for(i = 0; i < depth; i++)
	{
		printf("         ");
	}
	printf("        -");
	mm_print_node(now->right, depth + 1);
	if(now->right->null == MM_NULL_NOTNULL && now->right->size < now->size)
	{
		printf("ERROR\n");
	}
}

void mm_print()
{
	PDATA data;

	printf("--------------------------------------------------------------------------------\n");

	data = (PDATA)mem_heap_lo();
	mm_print_node(data->root, 0);

	//getchar();

	printf("--------------------------------------------------------------------------------\n\n");
}

void *mm_memset(void *b, int c, size_t len)
{
	int i;

	for(i = 0; i < len; i++)
	{
		*((unsigned char *)b + i) = (unsigned char)c;
	}

	return b;
}

void *mm_memcpy(void *dst, void *src, size_t n)
{
	int i;
	for(i = 0; i < n; i++)
	{
		*((unsigned char *)dst + i) = *((unsigned char *)src + i);
	}

	return dst;
}

char mm_cmp_node(PNODE x, PNODE y)
{
	if(x->size == y->size)
	{
		return x < y;
	}
	return x->size < y->size;
}

PNODE mm_rotate_left(PNODE now)
{
	PNODE tmp;

	tmp = now->right;
	now->right = tmp->left;
	tmp->left = now;
	return tmp;
}

PNODE mm_rotate_right(PNODE now)
{
	PNODE tmp;

	tmp = now->left;
	now->left = tmp->right;
	tmp->right = now;
	return tmp;
}

PNODE mm_search_node(PNODE now, size_t size)
{
	PNODE ptr;
	if(now->null == MM_NULL_NULL)
	{
		return NULL;
	}
	if(size <= now->size)
	{
		ptr = mm_search_node(now->left, size);
		if(ptr == NULL)
		{
			ptr = now;
		}
		return ptr;
	}
	return mm_search_node(now->right, size);
}

PNODE mm_insert_node(PNODE now, PNODE new)
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

PNODE mm_delete_left(PNODE now, char *balance)
{
	//printf("enter_ %d %d %d\n", now->size, now->left->size, now->right->size);
	if(now->right->color == MM_COLOR_RED)
	{
		//printf("case 2\n");
		now = mm_rotate_left(now);
		now->color = MM_COLOR_BLACK;
		now->left->color = MM_COLOR_RED;

		now->left = mm_delete_left(now->left, balance);
		return now;
	}

	if(now->color == MM_COLOR_BLACK && now->right->color == MM_COLOR_BLACK &&
		now->right->left->color == MM_COLOR_BLACK && now->right->right->color == MM_COLOR_BLACK)
	{
		//printf("case 3\n");
		now->right->color = MM_COLOR_RED;
		return now;
	}

	if(now->color == MM_COLOR_RED && now->right->left->color == MM_COLOR_BLACK && now->right->right->color == MM_COLOR_BLACK)
	{
		//printf("case 4\n");
		now->color = MM_COLOR_BLACK;
		now->right->color = MM_COLOR_RED;
		*balance = 1;
		return now;
	}

	if(now->right->color == MM_COLOR_BLACK &&
		now->right->left->color == MM_COLOR_RED && now->right->right->color == MM_COLOR_BLACK)
	{
		//printf("case 5\n");
		now->right = mm_rotate_right(now->right);
		now->right->color = MM_COLOR_BLACK;
		now->right->right->color = MM_COLOR_RED;
	}

	if(now->right->color == MM_COLOR_BLACK && now->right->right->color == MM_COLOR_RED)
	{
		//printf("case 6\n");
		now = mm_rotate_left(now);
		now->color = now->left->color;
		now->left->color = now->right->color = MM_COLOR_BLACK;
		*balance = 1;
		return now;
	}

	return now;
}

PNODE mm_delete_right(PNODE now, char *balance)
{
	//printf("enter_ %d %d %d\n", now->size, now->left->size, now->right->size);
	//printf("%p %p %p\n", now, now->left, now->right);
	if(now->left->color == MM_COLOR_RED)
	{
		//printf("case 2\n");
		now = mm_rotate_right(now);
		now->color = MM_COLOR_BLACK;
		now->right->color = MM_COLOR_RED;

		now->right = mm_delete_right(now->right, balance);
		return now;
	}

	if(now->color == MM_COLOR_BLACK && now->left->color == MM_COLOR_BLACK &&
		now->left->left->color == MM_COLOR_BLACK && now->left->right->color == MM_COLOR_BLACK)
	{
		//printf("case 3\n");
		now->left->color = MM_COLOR_RED;
		return now;
	}

	if(now->color == MM_COLOR_RED && now->left->left->color == MM_COLOR_BLACK && now->left->right->color == MM_COLOR_BLACK)
	{
		//printf("case 4\n");
		now->color = MM_COLOR_BLACK;
		now->left->color = MM_COLOR_RED;
		*balance = 1;
		return now;
	}

	if(now->left->color == MM_COLOR_BLACK &&
		now->left->right->color == MM_COLOR_RED && now->left->left->color == MM_COLOR_BLACK)
	{
		//printf("case 5\n");
		now->left = mm_rotate_left(now->left);
		now->left->color = MM_COLOR_BLACK;
		now->left->left->color = MM_COLOR_RED;
	}

	if(now->left->color == MM_COLOR_BLACK && now->left->left->color == MM_COLOR_RED)
	{
		//printf("case 6\n");
		now = mm_rotate_right(now);
		now->color = now->right->color;
		now->right->color = now->left->color = MM_COLOR_BLACK;
		*balance = 1;
		return now;
	}

	return now;
}

PNODE mm_delete_node(PNODE now, PNODE del, char *balance, PNODE *target)
{
	PNODE ptr;
	//printf("enter\n");
	//printf("%d %d\n", now->size, del->size);
	if(target)
	{
		//printf("target\n");
		if(now->left->null == MM_NULL_NULL)
		{
			*target = now;
			*balance = (now->color == MM_COLOR_RED) || (now->right->color == MM_COLOR_RED);
			now = now->right;
			now->color = MM_COLOR_BLACK;
		}
		else
		{
			now->left = mm_delete_node(now->left, del, balance, target);
			if(!*balance)
			{
				now = mm_delete_left(now, balance);
			}
		}
		return now;
	}
	if(now == del)
	{
		//printf("del %d\n", now->size);
		if(now->right->null == MM_NULL_NULL)
		{
			*balance = (now->color == MM_COLOR_RED) || (now->left->color == MM_COLOR_RED);
			now = now->left;
			now->color = MM_COLOR_BLACK;
		}
		else
		{
			now->right = mm_delete_node(now->right, del, balance, &ptr);
			ptr->color = now->color;
			ptr->left = now->left;
			ptr->right = now->right;
			ptr->color = now->color;
			now = ptr;
			if(!*balance)
			{
				now = mm_delete_right(now, balance);
			}
		}
		//printf("exit del %d\n", now->size);
	}
	else if(mm_cmp_node(del, now))
	{
		//printf("else1\n");
		now->left = mm_delete_node(now->left, del, balance, NULL);
		if(!*balance)
		{
			return mm_delete_left(now, balance);
		}
	}
	else
	{
		//printf("else2\n");
		now->right = mm_delete_node(now->right, del, balance, NULL);
		if(!*balance)
		{
			return mm_delete_right(now, balance);
		}
	}
	return now;
}

PNODE mm_search(size_t size)
{
	PDATA data;

	data = (PDATA)mem_heap_lo();
	return mm_search_node(data->root, size);
}

void mm_insert(PNODE new)
{
	PDATA data;
	// printf("insert %d\n", new->size);

	data = (PDATA)mem_heap_lo();
	data->root = mm_insert_node(data->root, new);
	data->root->color = MM_COLOR_BLACK;
	
	// mm_print();

	/*if(new->size == 63)
	{
		mm_print();
		getchar();
	}*/
}

void mm_delete(PNODE del)
{
	char balance;
	PDATA data;
	// printf("delete %d\n", del->size);

	data = (PDATA)mem_heap_lo();
	data->root = mm_delete_node(data->root, del, &balance, NULL);
	data->root->color = MM_COLOR_BLACK;
	
	// mm_print();
	//getchar();
}

PNODE mm_get_prev(PNODE ptr)
{
	return ptr->prev;
}

PNODE mm_get_next(PNODE ptr)
{
	return (PNODE)(((void *)ptr) + (ptr->size << MM_ALIGN));
}

void *mm_frag_node(PNODE now, size_t size)
{
	size_t tsize;
	PNODE next;

	if(now->size < size + (MM_NODE_SIZE >> MM_ALIGN))
	{
		return now;
	}

	tsize = now->size - size;
	now->size = size;

	next = mm_get_next(now);
	mm_memset(next, 0, MM_NODE_SIZE);
	next->size = tsize;

	next->prev = now;
	mm_get_next(next)->prev = next;
	
	mm_insert(next);
	next->alloc = MM_ALLOC_FREE;

	return now;
}

int mm_init()
{
	PNODE node;
	PDATA data;

//	printf("mm_init\n");

	data = (PDATA)mem_sbrk(MM_DATA_SIZE);
	memset(data, 0, MM_DATA_SIZE);

	data->root = &data->null;
	data->null.color = MM_COLOR_BLACK;
	data->null.null = MM_NULL_NULL;

	node = (PNODE)mem_sbrk(MM_HEADER_SIZE);
	node->prev = NULL;
	node->size = 0;

	return 0;
}

void *mm_malloc(size_t size)
{
	PNODE ptr, next;
	
	size = (ALIGN(size) + MM_HEADER_SIZE) >> MM_ALIGN;
	ptr = mm_search(size);

	// printf("mm_malloc %d\n", size);

	if(ptr == NULL)
	{
		ptr = (PNODE)(mem_sbrk(size << MM_ALIGN) - MM_HEADER_SIZE);
		mm_memset(ptr, 0, MM_HEADER_SIZE);
		ptr->size = size;
		ptr->alloc = MM_ALLOC_ALLOC;
		
		next = mm_get_next(ptr);
		next->prev = ptr;
		next->size = 0;

		printf("mm_malloc %d new %d\n", size, (void *)ptr - mem_heap_lo());
	}
	else
	{
		mm_delete(ptr);
		if(size <= ptr->size)
		{
			ptr = mm_frag_node(ptr, size);
		}
		ptr->alloc = MM_ALLOC_ALLOC;

		printf("mm_malloc %d old %d\n", size, (void *)ptr - mem_heap_lo());
	}
	//mm_print();

	return (void *)ptr + MM_HEADER_SIZE;
}

void mm_free(void *ptr)
{
	PNODE now, prev, next;

	// printf("mm_free %p\n", ptr);

	now = (PNODE)(ptr - MM_HEADER_SIZE);
	now->alloc = MM_ALLOC_FREE;
	prev = mm_get_prev(now);
	next = mm_get_next(now);

	if(prev != NULL && prev->alloc == MM_ALLOC_FREE)
	{
		// printf("prev %p\n", prev);
		mm_delete(prev);
		prev->size += now->size;
		now = prev;
	}
	if(next->size != 0 && next->alloc == MM_ALLOC_FREE)
	{
		// printf("next %p\n", next);
		mm_delete(next);
		now->size += next->size;
	}

	mm_get_next(now)->prev = now;
	// printf("next: %p\n", mm_get_next(now));
	mm_insert(now);

	//mm_print();
}

void *mm_realloc(void *ptr, size_t size)
{
	size_t osize, sum;
	PNODE now, next, new;

	if(ptr == NULL)
	{
		return mm_malloc(size);
	}
	if(size == 0)
	{
		mm_free(ptr);
		return ptr;
	}

	osize = size;
	size = (ALIGN(size) + MM_HEADER_SIZE) >> MM_ALIGN;
	now = (PNODE)(ptr - MM_HEADER_SIZE);

	// printf("realloc %p %d\n", ptr, size);

	if(size <= now->size)
	{
		now = mm_frag_node(now, size);
		now->alloc = MM_ALLOC_ALLOC;
	}
	else if(now->size < size)
	{
		sum = now->size;
		for(next = mm_get_next(now); next->size != 0 && next->alloc == MM_ALLOC_FREE && sum < size; next = mm_get_next(next))
		{
			sum += next->size;
		}

		if(next->size == 0)
		{
			//printf("fasdfas\n");
			mem_sbrk((size - sum) << MM_ALIGN);
			next = mm_get_prev(next);
			next->size += size - sum;
			mm_get_next(next)->prev = next;
			mm_get_next(next)->size = 0;
		}
		else if(sum < size)
		{
			// printf("%d %d\n", next->size << MM_ALIGN, next->alloc);
			// getchar();
			new = mm_malloc(osize);
			mm_memcpy(new, ptr, (now->size << MM_ALIGN) - MM_HEADER_SIZE);
			mm_free(ptr);
			return new;
		}
		// printf("case 1\n");

		for(next = mm_get_next(now); next->size != 0 && next->alloc == MM_ALLOC_FREE && now->size < size; next = mm_get_next(next))
		{
			mm_delete(next);
			now->size += next->size;
		}

		mm_get_next(now)->prev = now;
	}

	return ptr;
}
