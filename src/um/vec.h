#ifndef UM_VEC_H
#define UM_VEC_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "um/common.h"

#ifndef UM_VEC_CACHE_LINE
#define UM_VEC_CACHE_LINE 128
#endif

typedef struct um_vec_h {
	size_t count;
	size_t alloc;
	struct um_vec_h *next;
} um_vec_h;

#define UmVItem(V) (***(V))
#define UmVType1(V) __typeof__(UmVItem(V))
#define UmVSize1(V) sizeof(UmVItem(V))
#define UmVData(V, H) ((UmVType1(V) *)((H) + 1))
#define UmVHead(V) (((um_vec_h *)(V)) - 1)

#define um_vec_alloc(V) (__typeof__(V))_um_vec_alloc(UM_VEC_CACHE_LINE, 0)
#define um_vec_alloc_manual(V, N)                                              \
	(__typeof__(V))_um_vec_alloc((N)*UmVSize1(V), 0)
#define um_vec(T) T ***
#define um_vec_len(V) _um_vec_len(UmVHead(V))
#define um_vec_for(V, NAME)                                                    \
	_um_vec_for(V, NAME, UmGen(_h), UmGen(_o), UmGen(_i), UmGen(_c))
#define um_vec_for_i(V, NAME, INDEX)                                           \
	_um_vec_for(V, NAME, UmGen(_h), UmGen(_o), (INDEX), UmGen(_c))
#define um_vec_for_range(V, NAME, START, END)                                  \
	_um_vec_for_range(V, NAME, UmGen(_h), UmGen(_o), UmGen(_i), UmGen(_c), \
			  UmGen(_f), UmGen(_s), (START), UmGen(_e), (END))
#define um_vec_for_range_i(V, NAME, START, END, INDEX)                         \
	_um_vec_for_range(V, NAME, UmGen(_h), UmGen(_o), (INDEX), UmGen(_c),   \
			  UmGen(_f), UmGen(_s), (START), UmGen(_e), (END))
#define um_vec_get(V, N) (*um_vec_at(V, N))
#define um_vec_at(V, N) (UmVType1(V) *)_um_vec_at(UmVHead(V), UmVSize1(V), (N))
#define um_vec_slice(V, START, END)                                            \
	(__typeof__(V))_um_vec_slice(UmVHead(V), UmVSize1(V), START, END)

#define um_vec_push(V, ITEM)                                                   \
	do {                                                                   \
		UmVType1(V) *_um_push_to_ptr =                                 \
		    (UmVType1(V) *)_um_vec_push_to(UmVHead(V), UmVSize1(V));   \
		*_um_push_to_ptr = (ITEM);                                     \
	} while (0)

#define um_vec_verify(V) _um_vec_verify(UmVHead(V))

#define _um_vec_for(V, N, H, O, I, C)                                          \
	for (int(O) = 1, (C) = 0, (I) = 0; (O); (O) = 0)                       \
		for (UmVType1(V) * (N); (O); (O) = 0)                          \
			for (um_vec_h * (H) = UmVHead(V); (H);                 \
			     (H) = (H)->next)                                  \
				for ((C) = 0, (N) = UmVData(V, H);             \
				     (C) < (H)->count;                         \
				     (C)++, (I)++, (N) = UmVData(V, H) + (C))

#define _um_vec_for_range(V, N, H, O, I, C, F, S, SI, E, EI)                   \
	for (int(O) = 1, (F), (C), (I) = 0, (S) = (SI), (E) = (EI); O;         \
	     (O) = 0)                                                          \
		for (UmVType1(V) * (N); (O); (O) = 0)                          \
			for (um_vec_h * (H) =                                  \
				 UmVRewind(UmVHead(V), &(S), &(E));            \
			     (O) && (H); (H) = (H)->next)                      \
				for ((C) = (S), (S) = 0,                       \
				    (N) = UmVData(V, H) + (C),                 \
				    (F) = (E) <= (H)->count ? ((O) = 0, (E))   \
							    : (H)->count;      \
				     (C) < (F);                                \
				     (C)++, (I)++, (N) = UmVData(V, H) + (C))

static inline um_vec_h* UmVRewind(um_vec_h *head, int *start, int *end) {
	while (*start > head->count) {
		*start -= head->count;
		*end -= head->count;
		head = head->next;
	}
	return head;
}

static inline void _um_vec_verify(um_vec_h *head)
{
	// TODO delme
	assert(head);
	assert(head->alloc);
}

static inline char *_um_vec_alloc(size_t alloc, um_vec_h *next)
{
	assert(alloc);
	um_vec_h *mem = malloc(sizeof(um_vec_h) + alloc);
	assert(mem);
	*mem = (um_vec_h){.alloc = alloc, .next = next};
	assert(mem->alloc);
	return (char *)(mem + 1);
}

static inline char *_um_vec_at(um_vec_h *head, size_t one, size_t index)
{
	while (index >= head->count) {
		index -= head->count;
		head = head->next;
	}

	return ((char *)(head + 1)) + one * index;
}

static inline char *_um_vec_push_to(um_vec_h *head, size_t one)
{
	_um_vec_verify(head);

	if ((head->count + 1) * one <= head->alloc)
		return ((char *)(head + 1)) + one * head->count++;

	if (!head->next)
		head->next = UmVHead(_um_vec_alloc(head->alloc, 0));

	return _um_vec_push_to(head->next, one);
}

static inline char *_um_vec_slice(um_vec_h *head, size_t one, size_t start,
				  size_t end)
{
	// TODO
	_um_vec_verify(head);

	while (start > head->count) {
		start -= head->count;
		end -= head->count;
		head = head->next;
	}

	um_vec_h *out = UmVHead(_um_vec_alloc(head->alloc, 0));
	_um_vec_verify(out);

	// TODO optimize memcpy in blocks
	for (;; start = 0, end -= head->count, head = head->next) {
		_um_vec_verify(head);
		if (end < head->count) {
			for (size_t i = start; i < end; i++) {
				void *dst = (void *)_um_vec_push_to(out, one);
				void *src = ((char *)(head + 1)) + i * one;
				memcpy(dst, src, one);
				_um_vec_verify(head);
			}
			break;
		} else {
			for (size_t i = start; i < head->count; i++) {
				void *dst = (void *)_um_vec_push_to(out, one);
				void *src = ((char *)(head + 1)) + i * one;
				memcpy(dst, src, one);
				_um_vec_verify(head);
			}
		}
	}

	return (char *)(out + 1);
}

static inline size_t _um_vec_len(um_vec_h *head)
{
	size_t len = 0;
	do {
		len += head->count;
		head = head->next;
	} while (head);
	return len;
}

#endif // UM_VEC_H
