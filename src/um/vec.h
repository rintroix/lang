#ifndef UM_VEC_H
#define UM_VEC_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "um/common.h"

#ifndef UMV_SIZE
#define UMV_SIZE UM_CHUNK_SIZE
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

typedef struct _umv_b {
	size_t end;
	struct _umv_b *next;
	char data[];
} _umv_b;

typedef struct _umv_h {
	size_t cap;
	size_t len;
	_umv_b bucket;
} _umv_h;

static_assert(sizeof(_umv_h) == sizeof(_umv_head(char)), "head");
static_assert(sizeof(_umv_b) == sizeof(_umv_bucket(char)), "bucket");

#define umv(T) T ***
#define umv_new_manual(T, N) ((umv(T))umv_alloc(N, sizeof(T)))
#define umv_new(T) umv_new_manual(T, UMV_SIZE)
#define umv_len(V) (UmVHead(V)->len)
#define umv_at(V, I) ((UmVItemT(V) *)_umv_at(UmVGHead(V), UmVItemS(V), I))
#define umv_get(V, I) (*umv_at(V, I))
#define umv_push(V, ...) (*UmVPushAt(V) = (__VA_ARGS__), umv_len(V) - 1)
#define umv_each(...) UmVEachN(__VA_ARGS__, H, H, H, H, I, N, L, L)(__VA_ARGS__)
#define umv_loop(...) UmVLoopN(__VA_ARGS__, H, H, I, N, L, L, L, L)(__VA_ARGS__)
#define umv_slice(V, START, END)                                               \
	umv_slice_into(V, umv_new(UmVItemT(V)), START, END)
#define umv_slice_into(V, DST, START, END)                                     \
	(__typeof__(V))_umv_slice((_umv_h *)(DST), UmVGHead(V), UmVItemS(V),   \
				  START, END)

#define UmV(V) umv(UmVItemT(V))
#define UmVItem(V) ***V
#define UmVItemT(V) __typeof__(UmVItem(V))
#define UmVItemS(V) sizeof(UmVItemT(V))
#define UmVHeadT(V) _umv_head(UmVItemT(V))
#define UmVHeadS(V) sizeof(UmVHeadT(V))
#define UmVHead(V) ((UmVHeadT(V) *)(V))
#define UmVGHead(V) ((_umv_h *)(V))
#define UmVBucketT(V) _umv_bucket(UmVItemT(V))
#define UmVBucket(V) (&(UmVHead(V)->bucket))
#define UmVGBucket(V) ((_umv_b *)(UmVBucket(V)))
#define UmVPushAt(V) (UmVItemT(V) *)_umv_push_at(UmVGHead(V), UmVItemS(V))
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
#define UmVEachImpN(V, NAME) UmVEachImpI(V, NAME, UmGen(_i))
#define UmVEachImpI(V, NAME, INDEX)                                            \
	UmVEachImp(V, NAME, UmGen(_b), UmGen(_o), INDEX, UmGen(_c))
#define UmVLoopImpN(V, NAME, START, END)                                       \
	UmVLoopImpI(V, NAME, START, END, UmGen(_i))
#define UmVLoopImpI(V, NAME, START, END, INDEX)                                \
	UmVLoopImp(V, NAME, UmGen(_h), UmGen(_o), INDEX, UmGen(_c), UmGen(_f), \
		   UmGen(_s), START, UmGen(_e), END)

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
				 (void *)_umv_rewind(UmVGHead(V), &(S), &(E)); \
			     (O) && (B); (B) = (B)->next)                      \
				for ((C) = (S), (S) = 0, (N) = (B)->data[C],   \
				    (F) = (E) <= (B)->end ? ((O) = 0, (E))     \
							  : (B)->end;          \
				     (C) < (F);                                \
				     (C)++, (I)++, (N) = (B)->data[C])

#define UmVLastBucket(B, N, CAP, ONE)                                          \
	(__typeof__(B))_umv_last_bucket((_umv_b *)B, N, CAP, ONE)

// TODO unify with deq
// TODO as expr instead

static inline _umv_h *umv_alloc(size_t count, size_t one)
{
	size_t size = um_next_pow2(sizeof(_umv_h) + one * count);
	size_t cap = (size - sizeof(_umv_h)) / one;
	_umv_h *mem = malloc(size);
	assert(mem);
	*mem = (_umv_h){.cap = cap};
	return mem;
}

static inline _umv_b *_umv_alloc_bucket(size_t cap, size_t one)
{
	_umv_b *mem = malloc(sizeof *mem + one * cap);
	assert(mem);
	*mem = (_umv_b){0};
	return mem;
}

static inline _umv_b *_umv_last_bucket(_umv_b *b, size_t n, size_t cap,
				       size_t one)
{
	assert(n <= cap);

	while (b->next)
		b = b->next;

	if (b->end + n > cap) {
		b->next = _umv_alloc_bucket(cap, one);
		b = b->next;
	}

	return b;
}

static inline size_t _umv_count_buckets(_umv_b *b)
{
	size_t n = 0;

	for (; b; b = b->next) {
		n++;
	}

	return n;
}

static inline void *_umv_push_at(_umv_h *h, size_t one)
{
	assert(h);

	size_t cap = h->cap;
	_umv_b *b = UmVLastBucket(&h->bucket, 1, cap, one);

	h->len++;
	return ((char *)b->data) + one * b->end++;
}

static inline void *_umv_at(_umv_h *h, size_t one, size_t index)
{
	assert(h);
	assert(index < h->len);

	size_t cap = h->cap;
	_umv_b *b = &h->bucket;

	while (index >= cap) {
		index -= cap;
		b = b->next;
	}

	return ((char *)b->data) + one * index;
}

// TODO optimize memcpy in blocks
// TODO hugely inefficient with push at every iteration
static inline _umv_h *_umv_slice(_umv_h *out, _umv_h *h, size_t one,
				 size_t start, size_t end)
{
	assert(end >= start);

	_umv_b *b = &h->bucket;
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

static inline _umv_b *_umv_rewind(_umv_h *h, int *start, int *end)
{
	size_t cap = h->cap;
	_umv_b *b = &h->bucket;

	while (*start > cap) {
		*start -= cap;
		*end -= cap;
		b = b->next;
	}

	return b;
}

#endif // UM_VEC_H
