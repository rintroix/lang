#ifndef DATA_H
#define DATA_H

#include "um/um.h"

#define vec(T) um_vec(T)
#define avec(V) (um_vec_alloc(V))
#define push(V, ITEM) um_vec_push(V, ITEM)
#define vlen(V) (um_vec_len(V))
#define vat(V, N) (um_vec_at(V, N))
#define vget(V, N) (*vat(V, N))
#define vfor(V, N) um_vec_for(V, N)
#define vfori(V, N, I) um_vec_for_i(V, N, I)
#define vforr(V, N, S, E) um_vec_for_range(V, N, S, E)
#define vforri(V, N, S, E, I) um_vec_for_range_i(V, N, S, E, I)
#define vslice(V, START, END) (um_vec_slice(V, START, END))

#endif // DATA_H
