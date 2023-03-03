#ifndef DATA_H
#define DATA_H

#include "um/um.h"

#define vec(T) um_vec(T)
#define avec(V) (um_vec_alloc(V))
#define push(V, ITEM) um_vec_push(V, ITEM)
#define vlen(V) (um_vec_len(V))
#define vat(V, N) (um_vec_at(V, N))
#define vget(V, N) (*vat(V, N))
#define vfor(...) um_vec_for(__VA_ARGS__)
#define vforr(...) um_vec_for_range(__VA_ARGS__)
#define vslice(V, START, END) (um_vec_slice(V, START, END))

#endif // DATA_H
