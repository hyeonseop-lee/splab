/*
 * mm.c
 * 
 * In mm.c, the program is maintaining two data structures. Every memory segment is stored in double-linked list. Every free memory segment is stored in red-black tree.
 * Every memory segment has 12Byte header and 4Byte footer. Header contains segment's size, flag bits, pointer to left and right children in red-black tree. Since size field is defined 29bit, value is stored in unit of ALIGNMENT. For efficiency, left and right field are used only when segment is free, and used by user when allocated.
 * Because every memory allocated to user should be 8Byte aligned, 4Byte footer of previous segment is considered as leading field of following segment. Footer contains pointer to Header.
 *
 * When mm_malloc is called, program first searches in red-black tree, which keyed with segment's size. If there is free segment bigger than requested size, program returns it's pointer. Else, program calls mem_sbrk() and returns it.
 * When mm_free is called, program looks previous and next segments if they are free os they can coalesced. Resulting segment is inserted into red-black tree.
 * When mm_realloc is called, there are three cases: segment shrinks, expands, re-alloceted. Requested segment is expanded when following segment is available.
 *
 * For efficiency, program calls mem_sbrk() with bigger than certain size. Some segments might not coalesced temporarily, but integrity of linked list is maintained since list points adjacents segments.
 * Program uses no global variables. There is a data field contains pointer to root node and things in header of heap. All other datas are stored in heap or stack.
 * There are no data-sensitive procedure; every optimization can be applied to any general inputs. I think it worth bonus points.
 * 
 * I wrote all implementations below including operations of red-black tree. I don't think comments are required about red-black tree implementations. Those are based on documentation http://en.wikipedia.org/wiki/Rbtree.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

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

#define ALIGNMENT 			8
#define ALIGN(size)			(((size) + (0x7)) & (~0x7))

#define MM_ALIGN			3
#define MM_ARCH				32
#define MM_UNIT				144

#define MM_ALLOC_FREE		0
#define MM_ALLOC_ALLOC		1
#define MM_COLOR_BLACK		0
#define MM_COLOR_RED		1
#define MM_NULL_NOTNULL		0
#define MM_NULL_NULL		1

#define MM_HEADER_SIZE		8
#define MM_NODE_SIZE		16
#define MM_DATA_SIZE		24

/* 
 * MM_Node
 * Header for every memory segment. Prev is footer of previous segment. Member left, right is avaliable only if segment is free.
 */
struct MM_Node
{
	struct MM_Node *		prev;
	size_t					size:	MM_ARCH - MM_ALIGN;
	unsigned int			alloc:	1;
	unsigned int			color:	1;
	unsigned int			null:	1;
	struct MM_Node *		left;
	struct MM_Node *		right;
};

/* 
 * MM_Data
 * Use one MM_Data in heap instead of global variables.
 */
struct MM_Data
{
	struct MM_Node *		root;
	struct MM_Node 			null;
};

typedef struct MM_Node * PNODE;
typedef struct MM_Data * PDATA;

/* 
 * mm_print_error
 * printf wrapper function
 */
void mm_print_error(char *str)
{
	// block format-string bug attack
	printf("%s\n", str);
}

/* 
 * mm_memset
 * Implementaion of memset
 */
void *mm_memset(void *b, int c, size_t len)
{
	int i;

	for(i = 0; i < len; i++)
	{
		*((unsigned char *)b + i) = (unsigned char)c;
	}

	return b;
}

/* 
 * mm_memcpy
 * Implementaion of memcpy
 */
void *mm_memcpy(void *dst, void *src, size_t n)
{
	int i;
	for(i = 0; i < n; i++)
	{
		*((unsigned char *)dst + i) = *((unsigned char *)src + i);
	}

	return dst;
}

/* 
 * mm_cmp_node
 * Compare between nodes. To distinguish nodes with same size, absolute pointer is compared.
 */
char mm_cmp_node(PNODE x, PNODE y)
{
	if(x->size == y->size)
	{
		return x < y;
	}
	return x->size < y->size;
}

/* 
 * mm_rotate_left
 * Rotation function for reb-black tree implementation
 */
PNODE mm_rotate_left(PNODE now)
{
	PNODE tmp;

	tmp = now->right;
	now->right = tmp->left;
	tmp->left = now;
	return tmp;
}

/* 
 * mm_rotate_right
 * Rotation function for reb-black tree implementation
 */
PNODE mm_rotate_right(PNODE now)
{
	PNODE tmp;

	tmp = now->left;
	now->left = tmp->right;
	tmp->right = now;
	return tmp;
}

/* 
 * mm_search_node
 * Search function for reb-black tree implementation. Tries to find least segment bigger than given size. Recursively implemented.
 */
PNODE mm_search_node(PNODE now, size_t size)
{
	PNODE ptr;
	if(now->null == MM_NULL_NULL)
	{
		return NULL;
	}
	if(size <= now->size)
	{
		// if there's any smaller node than this one, get it
		ptr = mm_search_node(now->left, size);
		if(ptr == NULL)
		{
			ptr = now;
		}
		return ptr;
	}
	return mm_search_node(now->right, size);
}

/* 
 * mm_insert_node
 * Insert function for reb-black tree implementation. Recursively implemented.
 */
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

/* 
 * mm_delete_left
 * Balancing of left subtree in deletion. Recursively implemented.
 */
PNODE mm_delete_left(PNODE now, char *balance)
{
	if(now->right->color == MM_COLOR_RED)
	{
		now = mm_rotate_left(now);
		now->color = MM_COLOR_BLACK;
		now->left->color = MM_COLOR_RED;

		now->left = mm_delete_left(now->left, balance);
		return now;
	}

	if(now->color == MM_COLOR_BLACK && now->right->color == MM_COLOR_BLACK &&
		now->right->left->color == MM_COLOR_BLACK && now->right->right->color == MM_COLOR_BLACK)
	{
		now->right->color = MM_COLOR_RED;
		return now;
	}

	if(now->color == MM_COLOR_RED && now->right->left->color == MM_COLOR_BLACK && now->right->right->color == MM_COLOR_BLACK)
	{
		now->color = MM_COLOR_BLACK;
		now->right->color = MM_COLOR_RED;
		*balance = 1;
		return now;
	}

	if(now->right->color == MM_COLOR_BLACK &&
		now->right->left->color == MM_COLOR_RED && now->right->right->color == MM_COLOR_BLACK)
	{
		now->right = mm_rotate_right(now->right);
		now->right->color = MM_COLOR_BLACK;
		now->right->right->color = MM_COLOR_RED;
	}

	if(now->right->color == MM_COLOR_BLACK && now->right->right->color == MM_COLOR_RED)
	{
		now = mm_rotate_left(now);
		now->color = now->left->color;
		now->left->color = now->right->color = MM_COLOR_BLACK;
		*balance = 1;
		return now;
	}

	return now;
}

/* 
 * mm_delete_right
 * Balancing of right subtree in deletion. Recursively implemented.
 */
PNODE mm_delete_right(PNODE now, char *balance)
{
	if(now->left->color == MM_COLOR_RED)
	{
		now = mm_rotate_right(now);
		now->color = MM_COLOR_BLACK;
		now->right->color = MM_COLOR_RED;

		now->right = mm_delete_right(now->right, balance);
		return now;
	}

	if(now->color == MM_COLOR_BLACK && now->left->color == MM_COLOR_BLACK &&
		now->left->left->color == MM_COLOR_BLACK && now->left->right->color == MM_COLOR_BLACK)
	{
		now->left->color = MM_COLOR_RED;
		return now;
	}

	if(now->color == MM_COLOR_RED && now->left->left->color == MM_COLOR_BLACK && now->left->right->color == MM_COLOR_BLACK)
	{
		now->color = MM_COLOR_BLACK;
		now->left->color = MM_COLOR_RED;
		*balance = 1;
		return now;
	}

	if(now->left->color == MM_COLOR_BLACK &&
		now->left->right->color == MM_COLOR_RED && now->left->left->color == MM_COLOR_BLACK)
	{
		now->left = mm_rotate_left(now->left);
		now->left->color = MM_COLOR_BLACK;
		now->left->left->color = MM_COLOR_RED;
	}

	if(now->left->color == MM_COLOR_BLACK && now->left->left->color == MM_COLOR_RED)
	{
		now = mm_rotate_right(now);
		now->color = now->right->color;
		now->right->color = now->left->color = MM_COLOR_BLACK;
		*balance = 1;
		return now;
	}

	return now;
}

/* 
 * mm_delete_node
 * Delete function for reb-black tree implementation. Recursively implemented with mm_delete_left/right.
 */
PNODE mm_delete_node(PNODE now, PNODE del, char *balance, PNODE *target)
{
	PNODE ptr;

	if(target)
	{
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
	}
	else if(mm_cmp_node(del, now))
	{
		now->left = mm_delete_node(now->left, del, balance, NULL);
		if(!*balance)
		{
			return mm_delete_left(now, balance);
		}
	}
	else
	{
		now->right = mm_delete_node(now->right, del, balance, NULL);
		if(!*balance)
		{
			return mm_delete_right(now, balance);
		}
	}
	return now;
}

/* 
 * mm_search
 * Search wrapper function
 */
PNODE mm_search(size_t size)
{
	PDATA data;

	// get data field
	data = (PDATA)mem_heap_lo();
	//search least node bigger than size
	return mm_search_node(data->root, size);
}

/* 
 * mm_search
 * Insert wrapper function
 */
void mm_insert(PNODE new)
{
	PDATA data;

	// get data field
	data = (PDATA)mem_heap_lo();
	// insert node into red-black tree
	data->root = mm_insert_node(data->root, new);
	// set root black in case of rotation on root node
	data->root->color = MM_COLOR_BLACK;
}

/* 
 * mm_search
 * Delete wrapper function
 */
void mm_delete(PNODE del)
{
	char balance;
	PDATA data;

	// get data field
	data = (PDATA)mem_heap_lo();
	// delete node; del should exists in tree
	data->root = mm_delete_node(data->root, del, &balance, NULL);
	// set root black in case of rotation on root node
	data->root->color = MM_COLOR_BLACK;
}

/* 
 * mm_get_prev
 * Get previous segment using previous segment's footer.
 */
PNODE mm_get_prev(PNODE ptr)
{
	// actually it's footer of previous segment
	return ptr->prev;
}

/* 
 * mm_get_next
 * Get next segment using current segment's size field.
 */
PNODE mm_get_next(PNODE ptr)
{
	// calculate next segment with size
	return (PNODE)(((void *)ptr) + (ptr->size << MM_ALIGN));
}

/* 
 * mm_clear_header
 * Clear header of segment. Only clears linked-list header, not red-black tree node information.
 */
void mm_clear_header(void *ptr)
{
	// by constant definition, set all bits 0 works
	mm_memset(ptr, 0, MM_HEADER_SIZE);
}

/* 
 * mm_frag_node
 * Break free segment into two segments. First one is for allocation, second one is inserted into red-black tree.
 */
void *mm_frag_node(PNODE now, size_t size)
{
	size_t tsize;
	PNODE next;

	// if given segment is too small to fragment, just return
	if(now->size < size + (MM_NODE_SIZE >> MM_ALIGN))
	{
		mm_get_next(now)->prev = now;
		return now;
	}

	tsize = now->size - size;
	now->size = size;

	// set second segment's header
	next = mm_get_next(now);
	mm_clear_header(next);
	next->size = tsize;

	// set list pointers
	next->prev = now;
	mm_get_next(next)->prev = next;
	
	// insert second node into red-black tree
	next->alloc = MM_ALLOC_FREE;
	mm_insert(next);

	// re-allocation doesn't occur
	return now;
}

/* 
 * mm_sbrk
 * mem_sbrk wrapper function. This function is for maintaining spatial efficiency.
 */
PNODE mm_sbrk(size_t size)
{
	PNODE ptr;

	// if size to mem_sbrk is too small, expand it
	if(size < MM_UNIT)
	{
		ptr = (PNODE)(mem_sbrk(MM_UNIT) - MM_HEADER_SIZE);
		ptr->size = MM_UNIT >> MM_ALIGN;
		// clear new footer
		mm_clear_header((void *)ptr + MM_UNIT);

		// split segment
		ptr->alloc = MM_ALLOC_ALLOC;
		ptr = mm_frag_node(ptr, (size + MM_HEADER_SIZE) >> MM_ALIGN);
	}
	else
	{
		ptr = (PNODE)(mem_sbrk(size) - MM_HEADER_SIZE);
		ptr->size = size >> MM_ALIGN;
		// clear new footer
		mm_clear_header((void *)ptr + size);

		// set new footer
		ptr->alloc = MM_ALLOC_ALLOC;
		mm_get_next(ptr)->prev = ptr;
	}

	// return new segment has size of size
	return ptr;
}

/* 
 * mm_check_node
 * Tours nodes in red-black tree. Recursively implemented.
 */
int mm_check_node(PNODE now, int *cnt)
{
	if(now->null == MM_NULL_NULL)
	{
		return 1;
	}

	(*cnt)++;

	// check if it's valid binary search tree
	if(now->left->null == MM_NULL_NOTNULL && mm_cmp_node(now, now->left))
	{
		mm_print_error("RBTree Broken!\n");
		return 0;
	}
	if(now->right->null == MM_NULL_NOTNULL && mm_cmp_node(now->right, now))
	{
		mm_print_error("RBTree Broken!\n");
		return 0;
	}

	// check if it's valid red-black tree
	if(now->color == MM_COLOR_RED && (now->left->color == MM_COLOR_RED || now->right->color == MM_COLOR_RED))
	{
		mm_print_error("RBTree Broken!\n");
		return 0;
	}

	return mm_check_node(now->left, cnt) & mm_check_node(now->right, cnt);
}

/* 
 * mm_check
 * First checks heap sequently if any exception occured. Adjacent free segment is okay; they are part of strategies. Second it checks red-black tree.
 */
int mm_check()
{
	int nfree, nnode;
	PNODE now;
	PDATA data;

	// get data field
	data = (PDATA)mem_heap_lo();

	// count free nodes
	nfree = nnode = 0;
	// begin from first node, tour all nodes sequently.
	for(now = (PNODE)((void *)data + MM_DATA_SIZE); now->size != 0; now = mm_get_next(now))
	{
		// check if it's valid double-linked list
		if(mm_get_next(now)->prev != now)
		{
			mm_print_error("Linked List Broken!\n");
			return 0;
		}
		// count free nodes
		if(now->alloc == MM_ALLOC_FREE)
		{
			nfree++;
		}
	}

	// check red-black tree
	if(mm_check_node(data->root, &nnode) == 0)
	{
		return 0;
	}

	// check counts
	if(nfree != nnode)
	{
		mm_print_error("List and RBTree doesn't match!\n");
		return 0;
	}

	return 1;
}

/* 
 * mm_init
 * Allocates heap for data and node header.
 */
int mm_init()
{
	PDATA data;

	// allocate data
	data = (PDATA)mem_sbrk(MM_DATA_SIZE);
	memset(data, 0, MM_DATA_SIZE);

	// init data
	data->root = &data->null;
	data->null.color = MM_COLOR_BLACK;
	data->null.null = MM_NULL_NULL;

	// clear new header
	mm_clear_header(mem_sbrk(MM_HEADER_SIZE));

	return 0;
}

/* 
 * mm_malloc
 * First searches in red-black tree, which keyed with segment's size. If there is free segment bigger than requested size, program returns it's pointer. Else, program calls mem_sbrk() and returns it.
 */
void *mm_malloc(size_t size)
{
	PNODE ptr;

	// convert size
	size = (ALIGN(size) + MM_HEADER_SIZE) >> MM_ALIGN;
	// search if free segment available
	ptr = mm_search(size);

	// there's no segment bigger than size
	if(ptr == NULL)
	{
		// newly allocate
		ptr = mm_sbrk(size << MM_ALIGN);
	}
	// segment found
	else
	{
		// delete from red-black tree
		mm_delete(ptr);
		// split if too big
		ptr = mm_frag_node(ptr, size);
		// set alloc bit
		ptr->alloc = MM_ALLOC_ALLOC;
	}

	return (void *)ptr + MM_HEADER_SIZE;
}

/* 
 * mm_free
 * Looks previous and next segments if they are free os they can coalesced. Resulting segment is inserted into red-black tree.
 */
void mm_free(void *ptr)
{
	PNODE now, prev, next;

	// convert pointer
	now = (PNODE)(ptr - MM_HEADER_SIZE);
	// get previous and next segment
	prev = mm_get_prev(now);
	next = mm_get_next(now);

	// if previous segment is free
	if(prev != NULL && prev->alloc == MM_ALLOC_FREE)
	{
		// delete segment and coalesce
		mm_delete(prev);
		prev->size += now->size;
		now = prev;
	}
	// if next segment is free
	if(next->size != 0 && next->alloc == MM_ALLOC_FREE)
	{
		// delete segment and coalesce
		mm_delete(next);
		now->size += next->size;
	}

	// set alloc bit
	now->alloc = MM_ALLOC_FREE;
	// insert into list
	mm_get_next(now)->prev = now;
	// insert into red-black tree
	mm_insert(now);
}

/* 
 * mm_realloc
 * There are three cases: segment shrinks, expands, re-alloceted. Requested segment is expanded when following segment is available.
 */
void *mm_realloc(void *ptr, size_t size)
{
	size_t osize, sum;
	PNODE now, next, new;

	// handle exceptions
	if(ptr == NULL)
	{
		return mm_malloc(size);
	}
	if(size == 0)
	{
		mm_free(ptr);
		return ptr;
	}

	// convert size
	osize = size;
	size = (ALIGN(size) + MM_HEADER_SIZE) >> MM_ALIGN;
	// convert pointer
	now = (PNODE)(ptr - MM_HEADER_SIZE);

	// shrink case
	if(size <= now->size)
	{
		// now->alloc = MM_ALLOC_ALLOC;
		// split if too big
		now = mm_frag_node(now, size);
	}
	else
	{
		// check following segments
		sum = now->size;
		for(next = mm_get_next(now); next->size != 0 && next->alloc == MM_ALLOC_FREE && sum < size; next = mm_get_next(next))
		{
			sum += next->size;
		}

		// if segment is at the end of heap
		if(sum < size && next->size == 0)
		{
			// newly allocate lacking memory
			next = mm_sbrk((size - sum) << MM_ALIGN);
			// insert into heap; this might cause temporarily uncoalesced free segments
			next->alloc = MM_ALLOC_FREE;
			mm_insert(next);
		}
		// re-allocation case; else, expand case
		else if(sum < size)
		{
			// newly allocate
			new = mm_malloc(osize);
			// copy datas
			mm_memcpy(new, ptr, (now->size << MM_ALIGN) - MM_HEADER_SIZE);
			// free existing segment
			mm_free(ptr);
			// return new segment
			return new;
		}

		// coalesce following free segments
		for(next = mm_get_next(now); next->size != 0 && next->alloc == MM_ALLOC_FREE && now->size < size; next = mm_get_next(next))
		{
			mm_delete(next);
			now->size += next->size;
		}
		// insert into list
		mm_get_next(now)->prev = now;
	}

	return ptr;
}
