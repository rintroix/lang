#ifndef DATA_H
#define DATA_H

#include <stdlib.h>
#include <string.h>

struct um_vec_header {
	size_t count;
	size_t alloc;
	struct um_vec_header *next;
};

#ifndef UM_CACHE_LINE
#define UM_CACHE_LINE 128
#endif

#define um_vec_alloc() _um_vec_alloc(UM_CACHE_LINE, 0)
#define um_vec(T) struct um_vec_of_##T
#define um_vec_head(V) (((struct um_vec_header *)(V)) - 1)
#define um_vec_len(V) (um_vec_head(V)->count)
#define um_vec_declare(T)                                                      \
	um_vec(T)                                                              \
	{                                                                      \
		T *array;                                                      \
	}
#define um_vec_push(V, ITEM)                                                   \
	do {                                                                   \
		__typeof__((V)->array[0]) *_um_push_to_ptr =                   \
		    _um_vec_push_to(um_vec_head(V), sizeof((V)->array[0]));    \
		*_um_push_to_ptr = (ITEM);                                     \
	} while (0)
#define um_vec_at(V, N)                                                        \
	(__typeof__((V)->array[0]) *)_um_vec_at(um_vec_head(V),                \
						sizeof((V)->array[0]), (N))

#define declare_vec(T) um_vec_declare(T)
#define vec(T) um_vec(T)
#define avec() um_vec_alloc()
#define push(V, ITEM) um_vec_push(V, ITEM)

static inline void *_um_vec_alloc(size_t alloc, struct um_vec_header *next)
{
	struct um_vec_header *mem =
	    malloc(sizeof(struct um_vec_header) + alloc);
	*mem = (struct um_vec_header){.alloc = alloc, .next = next};
	return mem + 1;
}

static inline void *_um_vec_at(struct um_vec_header *head, size_t one, size_t index)
{
	if (index >= head->count)
		return _um_vec_at(head->next, one, index - head->count);

	return ((char*)(head + 1)) + one * index;
}

static inline void * _um_vec_push_to(struct um_vec_header *head, size_t one)
{
	if ((head->count + 1) * one <= head->alloc)
		return ((char *)(head + 1)) + one * head->count++;

	if (!head->next)
		head->next = um_vec_head(_um_vec_alloc(head->alloc, 0));

	return _um_vec_push_to(head->next, one);
}

#endif

#if 0
#define kv_roundup32(x)                                                        \
	(--(x), (x) |= (x) >> 1, (x) |= (x) >> 2, (x) |= (x) >> 4,             \
	 (x) |= (x) >> 8, (x) |= (x) >> 16, ++(x))

#define kvec_t(type)                                                           \
	struct {                                                               \
		size_t n, m;                                                   \
		type *a;                                                       \
	}
#define kv_A(v, i) ((v).a[(i)])
#define kv_pop(v) ((v).a[--(v).n])
#define kv_size(v) ((v).n)
#define kv_max(v) ((v).m)

#define kv_copy(type, v1, v0)                                                  \
	do {                                                                   \
		if ((v1).m < (v0).n)                                           \
			kv_resize(type, v1, (v0).n);                           \
		(v1).n = (v0).n;                                               \
		memcpy((v1).a, (v0).a, sizeof(type) * (v0).n);                 \
	} while (0)

#define kv_pushp(type, v)                                                      \
	(((v).n == (v).m)                                                      \
	     ? ((v).m = ((v).m ? (v).m << 1 : 2),                              \
		(v).a = (type *)realloc((v).a, sizeof(type) * (v).m), 0)       \
	     : 0),                                                             \
	    ((v).a + ((v).n++))

#define kv_a(type, v, i)                                                       \
	(((v).m <= (size_t)(i)                                                 \
	      ? ((v).m = (v).n = (i) + 1, kv_roundup32((v).m),                 \
		 (v).a = (type *)realloc((v).a, sizeof(type) * (v).m), 0)      \
	  : (v).n <= (size_t)(i) ? (v).n = (i) + 1                             \
				 : 0),                                         \
	 (v).a[(i)])

#endif
