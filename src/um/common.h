#ifndef UM_COMMON_H
#define UM_COMMON_H

#define UmEval2(X) X
#define UmEval(X) UmEval2(X)
#define UmCat2(X, Y) X##Y
#define UmCat(X, Y) UmCat2(X, Y)
#define UmGen(X) UmCat(X, __LINE__)
#define UmE(M) _Static_assert(0, M)

#endif // UM_COMMON_H
