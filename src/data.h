#ifndef DATA_H
#define DATA_H

#include "um.h"

#define declare_vec(T) um_vec_declare(T)
#define vec(T) um_vec(T)
#define avec() um_vec_alloc()
#define push(V, ITEM) um_vec_push(V, ITEM)
#define vat(V, N) um_vec_at(V, N)
#define vfor(V, N) um_vec_for(V, N)

#endif // DATA_H
