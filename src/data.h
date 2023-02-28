#ifndef DATA_H
#define DATA_H

#include <stdlib.h>
#include <string.h>

struct um_vec_generic {
	size_t count;
	size_t max;
	size_t alloc;
	struct um_vec_generic *next;
};

#define um_vec(T) struct um_vec_of_##T

#define um_vec_declare_type(T)                                                 \
	um_vec(T)                                                              \
	{                                                                      \
		size_t count;                                                  \
		size_t max;                                                    \
		size_t alloc;                                                  \
		um_vec(T) * next;                                              \
		T data[];                                                      \
	}

#define um_vec_asize(T, C) (sizeof(struct um_vec_generic) + (C) * sizeof(T))

#define _um_vec_alloc(S, C)                                                    \
	memcpy(malloc(S), &(struct um_vec_generic){.max = (C), .alloc = (S)},  \
	       sizeof(struct um_vec_generic))

#define um_vec_alloc(T, C) _um_vec_alloc(um_vec_asize(T, C), C)

static inline void *um_vec_push_head(struct um_vec_generic *head)
{
	if (head->count < head->max) {
		return head;
	}

	if (!head->next)
		head->next = _um_vec_alloc(head->alloc, head->max);

	return head->next;
}

#define um_vec_push(V, ITEM)                                                   \
	do {                                                                   \
		__typeof__(V) _head =                                          \
		    um_vec_push_head((struct um_vec_generic *)V);              \
		_head->data[(_head->count)++] = (ITEM);                        \
	} while (0)

#define declare_vec(T) um_vec_declare_type(T)
#define vec(T) um_vec(X)
#define avec(T, C) um_vec_alloc(T, C)
#define push(T, V, ITEM)

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
#define kv_init(v) ((v).n = (v).m = 0, (v).a = 0)
#define kv_destroy(v) free((v).a)
#define kv_A(v, i) ((v).a[(i)])
#define kv_pop(v) ((v).a[--(v).n])
#define kv_size(v) ((v).n)
#define kv_max(v) ((v).m)

#define kv_resize(type, v, s)                                                  \
	((v).m = (s), (v).a = (type *)realloc((v).a, sizeof(type) * (v).m))

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
