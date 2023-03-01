#ifndef UM_H
#define UM_H

#include <stdlib.h>
#include <string.h>

typedef struct um_vec_header {
	size_t count;
	size_t alloc;
	struct um_vec_header *next;
} um_vec_head_t;

#ifndef UM_CACHE_LINE
#define UM_CACHE_LINE 128
#endif

#define UM_VEC_ITEM(V) ((V)->array[0])
#define UM_VEC_TYPE1(V) __typeof__(UM_VEC_ITEM(V))
#define UM_VEC_SIZE1(V) sizeof(UM_VEC_ITEM(V))
#define UM_VEC_DATA(V, H) ((UM_VEC_TYPE1(V) *)((H) + 1)) 

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
		UM_VEC_TYPE1(V) *_um_push_to_ptr =                             \
		    _um_vec_push_to(um_vec_head(V), UM_VEC_SIZE1(V));          \
		*_um_push_to_ptr = (ITEM);                                     \
	} while (0)
#define um_vec_at(V, N)                                                        \
	(UM_VEC_TYPE1(V) *)_um_vec_at(um_vec_head(V), UM_VEC_SIZE1(V), (N))

#define um_vec_for(V, NAME) _um_vec_for_1(V, NAME, _um_head)
#define _um_vec_for_1(V, NAME, H)                                              \
	for (struct {                                                          \
		     int count;                                                \
		     int index;                                                \
		     UM_VEC_TYPE1(V) * value;                                  \
	     }(NAME) = {0};                                                    \
	     (NAME).count < 1;)                                                \
	_um_vec_for_2(V, NAME, H)

#define _um_vec_for_2(V, N, H)                                                 \
	for (um_vec_head_t * (H) = um_vec_head(V); (H); (H) = (H)->next)       \
	_um_vec_for_3(V, N, H)

#define _um_vec_for_3(V, N, H)                                                 \
	for ((N).count = 0, (N).value = UM_VEC_DATA(V, H);                     \
	     (N).count < (H)->count; (N).count++, (N).index++,                 \
	    (N).value = UM_VEC_DATA(V, H) + (N).count)

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

#endif // UM_H

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
