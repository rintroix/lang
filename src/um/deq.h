#ifndef UM_DEQ_H
#define UM_DEQ_H

#include "um/common.h"

#ifndef UMD_SIZE
#define UMD_SIZE UM_CHUNK_SIZE
#endif

#define _umd_head(T)                                                           \
	struct {                                                                   \
		size_t cap;                                                            \
		size_t len;                                                            \
		_umd_bucket(T) bucket;                                                 \
	}

#define _umd_bucket(T)                                                         \
	struct {                                                                   \
		size_t start;                                                          \
		size_t end;                                                            \
		void *next;                                                            \
		T data[];                                                              \
	}

typedef struct _umd_b {
	size_t start;
	size_t end;
	struct _umd_b *next;
	char data[];
} _umd_b;

typedef struct _umd_h {
	size_t cap;
	size_t len;
	_umd_b bucket;
} _umd_h;

static_assert(sizeof(_umd_h) == sizeof(_umd_head(char)), "head");
static_assert(sizeof(_umd_b) == sizeof(_umd_bucket(char)), "bucket");

#define umd(T) T *****
#define umd_new_manual(T, N) ((umd(T))umd_alloc(N, sizeof(T)))
#define umd_new(T) umd_new_manual(T, UMD_SIZE)
#define umd_len(D) (UmDHead(D)->len)
#define umd_at(D, I) ((UmDItemT(D) *)_umd_at(UmDGHead(D), UmDItemS(D), I))
#define umd_get(D, I) (*umd_at(D, I))
#define umd_each(...) UmDEachN(__VA_ARGS__, H, H, H, H, I, N, L, L)(__VA_ARGS__)
#define umd_loop(...) UmDLoopN(__VA_ARGS__, H, H, I, N, L, L, L, L)(__VA_ARGS__)
#define umd_push(D, ...) (*UmDPushAt(D) = (__VA_ARGS__), umd_len(D) - 1)

#define UmD(D) umd(UmDItemT(D))
#define UmDItem(D) *****D
#define UmDItemT(D) __typeof__(UmDItem(D))
#define UmDItemS(D) sizeof(UmDItemT(D))
#define UmDHeadT(D) _umd_head(UmDItemT(D))
#define UmDHeadS(D) sizeof(UmDHeadT(D))
#define UmDHead(D) ((UmDHeadT(D) *)(D))
#define UmDGHead(D) ((_umd_h *)(D))
#define UmDBucketT(D) _umd_bucket(UmDItemT(D))
#define UmDBucket(D) (&(UmDHead(D)->bucket))
#define UmDGBucket(D) ((_umd_b *)(UmDBucket(D)))
#define UmDPushAt(D) (UmDItemT(D) *)_umd_push_at(UmDGHead(D), UmDItemS(D))
#define UmDCountBuckets(D) _umd_count_buckets(UmDGBucket(D))
#define UmDAddBucket(B, CAP, ONE)                                              \
	do {                                                                       \
		assert(!(B)->next);                                                    \
		(B)->next = _umd_alloc_bucket(CAP, ONE);                               \
		(B) = (B)->next;                                                       \
	} while (0)

#define UmDDump(D)                                                             \
	do {                                                                       \
		printf("DEQ DUMP for '%s'\n", #D);                                     \
		printf("\tSELF %p CAP %zu LEN %zu\n", UmDHead(D), UmDHead(D)->cap,     \
			   UmDHead(D)->len);                                               \
		printf("\tBUCKET %p START %zu END %zu\n", UmDBucket(D),                \
			   UmDBucket(D)->start, UmDBucket(D)->end);                        \
	} while (0)

#define UmDEachN(a, b, c, d, e, f, g, X, ...) UmDEachImp##X
#define UmDLoopN(a, b, c, d, e, f, g, X, ...) UmDLoopImp##X

#define UmDEachImpL(...) UmE("umd_each: not enough arguments")
#define UmDEachImpH(...) UmE("umd_each: too many arguments")
#define UmDLoopImpL(...) UmE("umd_loop: not enough arguments")
#define UmDLoopImpH(...) UmE("umd_loop: too many arguments")
#define UmDEachImpN(D, NAME) UmDEachImpI(D, NAME, UmGen(_i))
#define UmDEachImpI(D, NAME, INDEX)                                            \
	UmDEachImp(D, NAME, UmGen(_b), UmGen(_o), INDEX, UmGen(_c))

// D self
// N name
// B bucket
// O sentinel
// I overall index
// C bucket index
#define UmDEachImp(D, N, B, O, I, C)                                           \
	for (int(O) = 1, (C) = 0, (I) = 0; (O); (O) = 0)                           \
		for (UmDItemT(D)(N); (O); (O) = 0)                                     \
			for (UmDBucketT(D) * (B) = (void *)UmDBucket(D); (B);              \
				 (B) = (B)->next)                                              \
				for ((C) = (B)->start, (N) = (B)->data[C]; (C) < (B)->end;     \
					 (C)++, (I)++, (N) = (B)->data[C])

// B bucket
// N amount of free items needed
#define UmDLastBucket(B, N, CAP, ONE)                                          \
	(__typeof__(B))_umd_last_bucket((_umd_b *)B, N, CAP, ONE)

static inline _umd_h *umd_alloc(size_t count, size_t one)
{
	size_t size = um_next_pow2(sizeof(_umd_h) + one * count);
	size_t cap = (size - sizeof(_umd_h)) / one;
	_umd_h *mem = malloc(size);
	assert(mem);
	*mem = (_umd_h){.cap = cap};
	return mem;
}

static inline _umd_b *_umd_alloc_bucket(size_t cap, size_t one)
{
	_umd_b *mem = malloc(sizeof *mem + one * cap);
	assert(mem);
	*mem = (_umd_b){0};
	return mem;
}

static inline _umd_b *_umd_last_bucket(_umd_b *b, size_t n, size_t cap,
									   size_t one)
{
	assert(n <= cap);

	while (b->next)
		b = b->next;

	if (b->end + n > cap) {
		b->next = _umd_alloc_bucket(cap, one);
		b = b->next;
	}

	return b;
}

static inline size_t _umd_count_buckets(_umd_b *b)
{
	size_t n = 0;

	for (; b; b = b->next) {
		n++;
	}

	return n;
}

static inline void *_umd_at(_umd_h *h, size_t one, size_t index)
{
	assert(h);
	_umd_b *b = &h->bucket;

	assert(index < h->len);

	while (index >= b->end - b->start) {
		index -= b->end - b->start;
		b = b->next;
	}

	return ((char *)b->data) + b->start + one * index;
}

static inline void *_umd_push_at(_umd_h *h, size_t one)
{
	assert(h);
	_umd_b *b = UmDLastBucket(&h->bucket, 1, h->cap, one);

	h->len++;
	return ((char *)b->data) + one * b->end++;
}

#endif // UM_DEQ_H
