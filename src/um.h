#ifndef UM_H
#define UM_H

#include <stdlib.h>

typedef struct um_vec_head_t {
	size_t count;
	size_t alloc;
	struct um_vec_head_t *next;
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
#define um_vec_head(V) (((um_vec_head_t *)(V)) - 1)
#define um_vec_len(V) (um_vec_head(V)->count)
#define um_vec_for(V, NAME) _um_vec_for_1(V, NAME, _um_head)

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

static inline void *_um_vec_alloc(size_t alloc, um_vec_head_t *next)
{
	um_vec_head_t *mem =
	    malloc(sizeof(um_vec_head_t) + alloc);
	*mem = (um_vec_head_t){.alloc = alloc, .next = next};
	return mem + 1;
}

static inline void *_um_vec_at(um_vec_head_t *head, size_t one, size_t index)
{
	if (index >= head->count)
		return _um_vec_at(head->next, one, index - head->count);

	return ((char*)(head + 1)) + one * index;
}

static inline void * _um_vec_push_to(um_vec_head_t *head, size_t one)
{
	if ((head->count + 1) * one <= head->alloc)
		return ((char *)(head + 1)) + one * head->count++;

	if (!head->next)
		head->next = um_vec_head(_um_vec_alloc(head->alloc, 0));

	return _um_vec_push_to(head->next, one);
}

#endif // UM_H
