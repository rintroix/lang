#ifndef UM_VEC_H
#define UM_VEC_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "um/common.h"

#ifndef UMV_BUCKET_SIZE
#define UMV_BUCKET_SIZE 128
#endif

#define _umv_head(T)                                                           \
	struct {                                                               \
		size_t cap;                                                    \
		size_t len;                                                    \
		_umv_bucket(T) bucket;                                         \
	}

#define _umv_bucket(T)                                                         \
	struct {                                                               \
		size_t end;                                                    \
		void *next;                                                    \
		T data[];                                                      \
	}

typedef struct _umv_bucket_struct {
	size_t end;
	struct _umv_bucket_struct *next;
	char data[];
} _umv_bucket_struct;

typedef struct _umv_head_struct {
	size_t cap;
	size_t len;
	_umv_bucket_struct bucket;
} _umv_head_struct;

_Static_assert(sizeof(_umv_head_struct) == sizeof(_umv_head(char)), "head");
_Static_assert(sizeof(_umv_bucket_struct) == sizeof(_umv_bucket(char)),
	       "bucket");

// TODO head types to deq
// TODO non void* funcs to deq

#define umv(T) T ***
#define umv_new_manual(T, N) ((umv(T))umv_alloc(N, sizeof(T)))
#define umv_new(T) umv_new_manual(T, UMV_BUCKET_SIZE / sizeof(T))
#define umv_len(V) (UmVHead(V)->len)
#define umv_at(V, I) ((UmVItemT(V) *)_umv_at(UmVGHead(V), UmVItemS(V), I))
#define umv_get(V, I) (*umv_at(V, I))
#define umv_push(V, ...)                                                       \
	(*(UmVItemT(V) *)_umv_push_at(UmVGHead(V), UmVItemS(V)) =              \
	     (__VA_ARGS__),                                                    \
	 umv_len(V) - 1)
#define umv_each(...) UmVEachN(__VA_ARGS__, H, H, H, H, I, N, L, L)(__VA_ARGS__)
#define umv_loop(...) UmVLoopN(__VA_ARGS__, H, H, I, N, L, L, L, L)(__VA_ARGS__)
#define umv_slice(V, START, END)                                               \
	umv_slice_into(V, umv_new(UmVItemT(V)), START, END)
// TODO check dst is a umv
#define umv_slice_into(V, DST, START, END)                                     \
	(__typeof__(V))_umv_slice((_umv_head_struct *)(DST), UmVGHead(V),      \
				  UmVItemS(V), START, END)

// TODO GHead to deq + push + at
#define _umv(V) umv(UmVItemT(V))
#define UmVItem(V) ***V
#define UmVItemT(V) __typeof__(UmVItem(V))
#define UmVItemS(V) sizeof(UmVItemT(V))
#define UmVHeadT(V) _umv_head(UmVItemT(V))
#define UmVHeadS(V) sizeof(UmVHeadT(V))
#define UmVHead(V) ((UmVHeadT(V) *)(V))
#define UmVGHead(V) ((_umv_head_struct *)(V))
#define UmVBucketT(V) _umv_bucket(UmVItemT(V))
#define UmVBucket(V) (&(UmVHead(V)->bucket))
#define UmVGBucket(V) ((_umv_bucket_struct *)(UmVBucket(V)))
#define UmVCountBuckets(V) _umv_count_buckets(UmVGBucket(V))
#define UmVAddBucket(B, CAP, ONE)                                              \
	do {                                                                   \
		assert(!(B)->next);                                            \
		(B)->next = _umv_alloc_bucket(CAP, ONE);                       \
		(B) = (B)->next;                                               \
	} while (0)

#define UmVDump(V)                                                             \
	do {                                                                   \
		printf("VEC DUMP CAP %zu\n", UmVHead(V)->cap);                 \
		printf("\tBUCKET END %zu\n", UmVBucket(V)->end);               \
	} while (0)

#define UmVEachN(a, b, c, d, e, f, g, X, ...) UmVEachImp##X
#define UmVLoopN(a, b, c, d, e, f, g, X, ...) UmVLoopImp##X

#define UmVEachImpL(...) UmE("umv_each: not enough arguments")
#define UmVEachImpH(...) UmE("umv_each: too many arguments")
#define UmVLoopImpL(...) UmE("umv_loop: not enough arguments")
#define UmVLoopImpH(...) UmE("umv_loop: too many arguments")
#define UmVEachImpN(V, NAME)                                                   \
	UmVEachImp(V, NAME, UmGen(_b), UmGen(_o), UmGen(_i), UmGen(_c))
#define UmVEachImpI(V, NAME, INDEX)                                            \
	UmVEachImp(V, NAME, UmGen(_b), UmGen(_o), (INDEX), UmGen(_c))
#define UmVLoopImpN(V, NAME, START, END)                                       \
	UmVLoopImp(V, NAME, UmGen(_h), UmGen(_o), UmGen(_i), UmGen(_c),        \
		   UmGen(_f), UmGen(_s), (START), UmGen(_e), (END))
#define UmVLoopImpI(V, NAME, START, END, INDEX)                                \
	UmVLoopImp(V, NAME, UmGen(_h), UmGen(_o), (INDEX), UmGen(_c),          \
		   UmGen(_f), UmGen(_s), (START), UmGen(_e), (END))

// V self
// N name
// B bucket
// O sentinel
// I overall index
// C bucket index
#define UmVEachImp(V, N, B, O, I, C)                                           \
	for (int(O) = 1, (C) = 0, (I) = 0; (O); (O) = 0)                       \
		for (UmVItemT(V)(N); (O); (O) = 0)                             \
			for (UmVBucketT(V) * (B) = (void *)UmVBucket(V); (B);  \
			     (B) = (B)->next)                                  \
				for ((C) = 0, (N) = (B)->data[C];              \
				     (C) < (B)->end;                           \
				     (C)++, (I)++, (N) = (B)->data[C])

// V self
// N name
// B bucket
// O sentinel
// I overall index
// C bucket index
// F finish name
// S  start index name
// SI start index value
// E  end index name
// EI end index value
#define UmVLoopImp(V, N, B, O, I, C, F, S, SI, E, EI)                          \
	for (int(O) = 1, (F), (C), (I) = 0, (S) = (SI), (E) = (EI); O;         \
	     (O) = 0)                                                          \
		for (UmVItemT(V)(N); (O); (O) = 0)                             \
			for (UmVBucketT(V) * (B) =                             \
				 _umv_rewind(UmVGHead(V), &(S), &(E));         \
			     (O) && (B); (B) = (B)->next)                      \
				for ((C) = (S), (S) = 0, (N) = (B)->data[C],   \
				    (F) = (E) <= (B)->end ? ((O) = 0, (E))     \
							  : (B)->end;          \
				     (C) < (F);                                \
				     (C)++, (I)++, (N) = (B)->data[C])

#define UmVNextSpaceOnEnd(B, N, CAP, ONE)                                      \
	do {                                                                   \
		while ((B)->end + (N) > (CAP)) {                               \
			if ((B)->next) {                                       \
				(B) = (B)->next;                               \
			} else {                                               \
				UmVAddBucket(B, CAP, ONE);                     \
				break;                                         \
			}                                                      \
		}                                                              \
	} while (0)

// TODO unify with deq
// TODO as expr instead

static inline void *umv_alloc(size_t count, size_t one)
{
	// TODO same in deq
	_umv_head_struct *mem = malloc(sizeof *mem + one * count);
	assert(mem);
	*mem = (_umv_head_struct){.cap = count};
	return mem;
}

static inline void *_umv_alloc_bucket(size_t count, size_t one)
{
	// TODO same in deq
	_umv_bucket_struct *mem = malloc(sizeof *mem + one * count);
	assert(mem);
	*mem = (_umv_bucket_struct){0};
	return mem;
}

// TODO in deq
static inline size_t _umv_count_buckets(_umv_bucket_struct *b)
{
	size_t n = 0;

	for (; b; b = b->next) {
		n++;
	}

	return n;
}

// TODO to deq
static inline void *_umv_push_at(_umv_head_struct *h, size_t one)
{
	assert(h);

	size_t cap = h->cap;

	_umv_bucket_struct *b = &h->bucket;

	UmVNextSpaceOnEnd(b, 1, cap, one);

	h->len++;
	return ((char *)b->data) + one * b->end++;
}

// TODO head to deq
static inline void *_umv_at(_umv_head_struct *h, size_t one, size_t index)
{
	assert(h);

	size_t cap = h->cap;

	_umv_bucket_struct *b = &h->bucket;

	assert(index < h->len);

	while (index >= cap) {
		index -= cap;
		b = b->next;
	}

	return ((char *)b->data) + one * index;
}

// TODO optimize memcpy in blocks
// TODO hugely inefficient with push at every iteration
static inline void *_umv_slice(_umv_head_struct *out, _umv_head_struct *h,
			       size_t one, size_t start, size_t end)
{
	assert(end >= start);

	_umv_bucket_struct *b = &h->bucket;
	size_t cap = h->cap;

	while (start >= cap) {
		start -= cap;
		end -= cap;
		b = b->next;
	}

	for (;; start = 0, end -= cap, b = b->next) {
		if (end <= cap) {
			for (size_t i = start; i < end; i++) {
				void *dst = _umv_push_at(out, one);
				void *src = ((char *)b->data) + i * one;
				memcpy(dst, src, one);
			}
			break;
		} else {
			for (size_t i = start; i < cap; i++) {
				void *dst = _umv_push_at(out, one);
				void *src = ((char *)b->data) + i * one;
				memcpy(dst, src, one);
			}
		}
	}

	return out;
}

static inline void *_umv_rewind(_umv_head_struct *h, int *start, int *end)
{
	size_t cap = h->cap;
	_umv_bucket_struct *b = &h->bucket;

	while (*start > cap) {
		*start -= cap;
		*end -= cap;
		b = b->next;
	}

	return b;
}

#endif // UM_VEC_H
