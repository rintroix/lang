#ifndef UM_H
#define UM_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// UM general

#define UmEval2(X) X
#define UmEval(X) UmEval2(X)
#define UmCat2(X, Y) X##Y
#define UmCat(X, Y) UmCat2(X, Y)
#define UmGen(X) UmCat(X, __LINE__)

// UM vec

#ifndef UM_VEC_CACHE_LINE
#define UM_VEC_CACHE_LINE 128
#endif

typedef struct um_vec_head_t {
	size_t count;
	size_t alloc;
	struct um_vec_head_t *next;
} um_vec_head_t;

#define UmVecItem(V) (***(V))
#define UmVecType1(V) __typeof__(UmVecItem(V))
#define UmVecSize1(V) sizeof(UmVecItem(V))
#define UmVecData(V, H) ((UmVecType1(V) *)((H) + 1))
#define UmVecHead(V) (((um_vec_head_t *)(V)) - 1)

#define um_vec_alloc(V) (__typeof__(V))_um_vec_alloc(UM_VEC_CACHE_LINE, 0)
#define um_vec_alloc_manual(V, N)                                              \
	(__typeof__(V))_um_vec_alloc((N)*UmVecSize1(V), 0)
#define um_vec(T) T ***
#define um_vec_len(V) _um_vec_len(UmVecHead(V))
#define um_vec_for(V, NAME) _um_vec_for_1(V, NAME, UmGen(_hd), UmGen(_fr))
#define um_vec_get(V, N) (*um_vec_at(V, N))
#define um_vec_at(V, N)                                                        \
	(UmVecType1(V) *)_um_vec_at(UmVecHead(V), UmVecSize1(V), (N))
#define um_vec_slice(V, START, END)                                            \
	(__typeof__(V))_um_vec_slice(UmVecHead(V), UmVecSize1(V), START, END)

#define um_vec_push(V, ITEM)                                                   \
	do {                                                                   \
		UmVecType1(V) *_um_push_to_ptr =                               \
		    (UmVecType1(V) *)_um_vec_push_to(UmVecHead(V),             \
						     UmVecSize1(V));           \
		*_um_push_to_ptr = (ITEM);                                     \
	} while (0)

#define um_vec_verify(V) _um_vec_verify(UmVecHead(V))

#define _um_vec_for_1(V, NAME, H, OUTER)                                       \
	for (int(OUTER) = 1; (OUTER); (OUTER) = 0)                             \
		for (struct {                                                  \
			     int count;                                        \
			     int index;                                        \
			     UmVecType1(V) * value;                            \
		     }(NAME) = {0};                                            \
		     (OUTER); (OUTER) = 0)                                     \
	_um_vec_for_2(V, NAME, H, OUTER)

#define _um_vec_for_2(V, N, H, O)                                              \
	for (um_vec_head_t * (H) = UmVecHead(V); (H); (H) = (H)->next)         \
	_um_vec_for_3(V, N, H, O)

#define _um_vec_for_3(V, N, H, O)                                              \
	for ((N).count = 0, (N).value = UmVecData(V, H);                       \
	     (N).count < (H)->count; (N).count++, (N).index++,                 \
	    (N).value = UmVecData(V, H) + (N).count)

static inline void _um_vec_verify(um_vec_head_t *head)
{
	// TODO delme
	assert(head);
	assert(head->alloc);
}

static inline char *_um_vec_alloc(size_t alloc, um_vec_head_t *next)
{
	assert(alloc);
	um_vec_head_t *mem = malloc(sizeof(um_vec_head_t) + alloc);
	assert(mem);
	*mem = (um_vec_head_t){.alloc = alloc, .next = next};
	assert(mem->alloc);
	return (char *)(mem + 1);
}

static inline char *_um_vec_at(um_vec_head_t *head, size_t one, size_t index)
{
	while (index >= head->count) {
		index -= head->count;
		head = head->next;
	}

	return ((char *)(head + 1)) + one * index;
}

static inline char *_um_vec_push_to(um_vec_head_t *head, size_t one)
{
	_um_vec_verify(head);

	if ((head->count + 1) * one <= head->alloc)
		return ((char *)(head + 1)) + one * head->count++;

	if (!head->next)
		head->next = UmVecHead(_um_vec_alloc(head->alloc, 0));

	return _um_vec_push_to(head->next, one);
}

static inline char *_um_vec_slice(um_vec_head_t *head, size_t one, size_t start,
				  size_t end)
{
	// TODO
	_um_vec_verify(head);

	while (start > head->count) {
		start -= head->count;
		end -= head->count;
		head = head->next;
	}

	um_vec_head_t *out = UmVecHead(_um_vec_alloc(head->alloc, 0));
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

static inline size_t _um_vec_len(um_vec_head_t *head)
{
	size_t len = 0;
	do {
		len += head->count;
		head = head->next;
	} while (head);
	return len;
}

#endif // UM_H
