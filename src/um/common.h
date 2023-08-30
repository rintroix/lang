#ifndef UM_COMMON_H
#define UM_COMMON_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef UM_CHUNK_SIZE
#define UM_CHUNK_SIZE 10
#endif

#define UmEval2(X) X
#define UmEval(X) UmEval2(X)
#define UmCat2(X, Y) X##Y
#define UmCat(X, Y) UmCat2(X, Y)
#define UmGen(X) UmCat(X, __LINE__)
#define UmE(M) static_assert(0, M)

static inline uint32_t um_next_pow2(uint32_t x)
{
	x -= 1;
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);

	return x + 1;
}

#endif // UM_COMMON_H
