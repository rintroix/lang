#ifndef UM_DEQ_H
#define UM_DEQ_H

#include "um/common.h"

#ifndef UM_DEQ_BUCKET_SIZE
#define UM_DEQ_BUCKET_SIZE 128
#endif

#define _umd_head(T)                                                           \
	struct {                                                               \
		size_t cap;                                                    \
		size_t len;                                                    \
		_umd_bucket(T) bucket;                                         \
	}

#define _umd_bucket(T)                                                         \
	struct {                                                               \
		size_t start;                                                  \
		size_t end;                                                    \
		void *next;                                                    \
		T data[];                                                      \
	}

#define _umd_generic_head                                                      \
	struct {                                                               \
		size_t cap;                                                    \
		size_t len;                                                    \
		_umd_bucket(char) bucket;                                      \
	}

#define umd(T) T****
#define umd_new_manual(T, N) ((umd(T))umd_alloc(N, sizeof(T)))
#define umd_new(T) umd_new_manual(T, UM_DEQ_BUCKET_SIZE / sizeof(T))
#define umd_len(D) (UmDHead(D)->len)
#define umd_at(D, I) ((UmDItemT(D)*)_umd_at(UmDHead(D), UmDItemS(D), I))
#define umd_get(D, I) (*umd_at(D, I))
#define umd_push(D, X) _umd_push(UmDHead(D), &(__typeof__(X)){X}, UmDItemS(D))
#define umd_each(...) UmDEachN(__VA_ARGS__, H, H, H, H, I, N, L, L)(__VA_ARGS__)
#define umd_loop(...) UmDLoopN(__VA_ARGS__, H, H, I, N, L, L, L, L)(__VA_ARGS__)

#define UmDCover(D) umd(UmDItemT) 
#define UmDItem(D) ****D
#define UmDItemT(D) __typeof__(UmDItem(D))
#define UmDItemS(D) sizeof(UmDItemT(D))
#define UmDGHeadT(D) _umd_head(char)
#define UmDHeadT(D) _umd_head(UmDItemT(D))
#define UmDHeadS(D) sizeof(UmDHeadT(D))
#define UmDHead(D) ((UmDHeadT(D) *)(D))
#define UmDGHead(D) ((UmDGHeadT(D) *)(D))
#define UmDBucketT(D) _umd_bucket(UmDItemT(D))
#define UmDBucket(D) (&(UmDHead(D)->bucket))
#define UmDCountBuckets(D) _umd_count_buckets(UmDBucket(D))
#define UmDAddBucket(B, CAP, ONE)                                              \
	do {                                                                   \
		assert(!(B)->next);                                            \
		(B)->next = _umd_alloc_bucket(CAP, ONE);                       \
		(B) = (B)->next;                                               \
	} while (0)

#define UmDDump(D)                                                             \
	do {                                                                   \
		printf("DEQ DUMP for '%s'\n", #D);                             \
		printf("\tSELF %p CAP %zu LEN %zu\n", UmDHead(D),              \
		       UmDHead(D)->cap, UmDHead(D)->len);                      \
		printf("\tBUCKET %p START %zu END %zu\n", UmDBucket(D),        \
		       UmDBucket(D)->start, UmDBucket(D)->end);                \
	} while (0)

#define UmDEachN(a, b, c, d, e, f, g, X, ...) UmDEachImp##X
#define UmDLoopN(a, b, c, d, e, f, g, X, ...) UmDLoopImp##X

#define UmDEachImpL(...) UmE("umd_each: not enough arguments")
#define UmDEachImpH(...) UmE("umd_each: too many arguments")
#define UmDLoopImpL(...) UmE("umd_loop: not enough arguments")
#define UmDLoopImpH(...) UmE("umd_loop: too many arguments")
#define UmDEachImpN(D, NAME)                                                    \
	UmDEachImp(D, NAME, UmGen(_b), UmGen(_o), UmGen(_i), UmGen(_c))
#define UmDEachImpI(D, NAME, INDEX)                                             \
	UmDEachImp(D, NAME, UmGen(_b), UmGen(_o), (INDEX), UmGen(_c))

// C count withing bucket
// I overall index
// O sentinel
// B bucket
// N name
// D self
#define UmDEachImp(D, N, B, O, I, C)                                           \
	for (int(O) = 1, (C) = 0, (I) = 0; (O); (O) = 0)                       \
		for (UmDItemT(D)(N); (O); (O) = 0)                             \
			for (UmDBucketT(D) * (B) = (void *)UmDBucket(D); (B);  \
			     (B) = (B)->next)                                  \
				for ((C) = (B)->start, (N) = (B)->data[C];     \
				     (C) < (B)->end;                           \
				     (C)++, (I)++, (N) = (B)->data[C])

#define UmDNextSpaceOnEnd(B, N, CAP, ONE)                                      \
	while ((B)->end + (N) > (CAP)) {                                       \
		if ((B)->next) {                                               \
			(B) = (B)->next;                                       \
		} else {                                                       \
			(B)->next = _umd_alloc_bucket(CAP, ONE);               \
			(B) = (B)->next;                                       \
			break;                                                 \
		}                                                              \
	}

static inline void* umd_alloc(size_t count, size_t one) {
	typedef _umd_head(char) t;
	t *mem = malloc(sizeof *mem + one * count);
	assert(mem);
	*mem = (t){.cap=count};
	return mem;
}

static inline void* _umd_alloc_bucket(size_t count, size_t one) {
	typedef _umd_bucket(char) t;
	t *mem = malloc(sizeof *mem + one * count);
	assert(mem);
	*mem = (t){0};
	return mem;
}

static inline size_t _umd_count_buckets(void *vbucket) {
	size_t n = 0;
	_umd_bucket(int) *b = vbucket;

	for (; b; b = b->next) {
		n++;
	}

	return n;
}

static inline size_t _umd_push(void *vhead, void *src, size_t one)
{
	assert(vhead);
	_umd_head(char) *h = vhead;
	_umd_bucket(char) *b = (void*)&h->bucket; // c23

	size_t cap = h->cap;

	UmDNextSpaceOnEnd(b, 1, cap, one);

	memcpy(((char *)b->data) + one * b->end, src, one);
	b->end++;

	return h->len++;
}


static inline void *_umd_at(void *vhead, size_t one, size_t index)
{
	assert(vhead);
	_umd_head(char) *h = vhead;
	_umd_bucket(char) *b = (void*)&h->bucket; // c23

	assert(index < h->len);

	while (index >= b->end - b->start) {
		index -= b->end - b->start;
		b = b->next;
	}

	return ((char *)b->data) + b->start + one * index;
}

#if 0

#define UmVFor(a, b, c, d, e, f, g, X, ...) _um_vec_for##X
#define UmVForR(a, b, c, d, e, f, g, X, ...) _um_vec_for_range##X

#define _um_vec_for_range_d(V, NAME, START, END)                               \
	_um_vec_for_range(V, NAME, UmGen(_h), UmGen(_o), UmGen(_i), UmGen(_c), \
			  UmGen(_f), UmGen(_s), (START), UmGen(_e), (END))
#define _um_vec_for_range_i(V, NAME, START, END, INDEX)                        \
	_um_vec_for_range(V, NAME, UmGen(_h), UmGen(_o), (INDEX), UmGen(_c),   \
			  UmGen(_f), UmGen(_s), (START), UmGen(_e), (END))

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

static inline char *_um_vec_slice(um_vec_h *head, size_t one, size_t start,
				  size_t end)
{
	_um_vec_verify(head);
	assert(end >= start);

	while (start && start >= head->count) {
		start -= head->count;
		end -= head->count;
		head = head->next;
	}

	um_vec_h *out = UmVHead(_um_vec_alloc(head->alloc, 0));
	_um_vec_verify(out);

	// TODO optimize memcpy in blocks
	for (;; start = 0, end -= head->count, head = head->next) {
		_um_vec_verify(head);
		if (end <= head->count) {
			for (size_t i = start; i < end; i++) {
				void *dst = (void *)_um_vec_push_to(out, one);
				void *src = ((char *)(head + 1)) + i * one;
				memcpy(dst, src, one);
			}
			break;
		} else {
			for (size_t i = start; i < head->count; i++) {
				void *dst = (void *)_um_vec_push_to(out, one);
				void *src = ((char *)(head + 1)) + i * one;
				memcpy(dst, src, one);
			}
		}
	}

	return (char *)(out + 1);
}

#endif // 0

#endif // UM_DEQ_H
