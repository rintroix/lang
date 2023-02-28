#ifndef DATA_H
#define DATA_H

#include "kvec.h"

#define vec(x) kvec_t(x)
#define avec(x) calloc(1, sizeof(kvec_t(x)))

#endif