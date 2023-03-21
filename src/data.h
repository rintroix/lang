#ifndef DATA_H
#define DATA_H

#include "um/um.h"

#define vec(T) umv(T)
#define avec(T) umv_new(T)
#define push(...) umv_push(__VA_ARGS__)
#define vlen(V) umv_len(V)
#define vat(V, N) umv_at(V, N)
#define vget(V, N) umv_get(V, N)
#define vslice(V, START, END) umv_slice(V, START, END)
#define veach(...) umv_each(__VA_ARGS__)
#define vloop(...) umv_loop(__VA_ARGS__)

#define deq(T) umd(T)
#define adeq(T) umd_new(T)
#define dget(D, N) umd_get(D, N)
#define dat(D, N) umd_at(D, N)
#define dpush(D, X) umd_push(D, X)
#define dlen(D) umd_len(D)
#define deach(...) umd_each(__VA_ARGS__)
#define dloop(...) umd_loop(__VA_ARGS__)
#define dprint(...) umd_append_fmt(__VA_ARGS__)

enum e_type {
	T_UNKNOWN = 1,
	T_INTEGER,
	T_FLOATING,
	T_STRING,
	T_COMPOUND,
};

typedef struct ast ast;

typedef struct type {
	enum e_type tag;
	int solid;
	char *name;
	union {
		struct {
			vec(struct type) items;
		} compound;

		struct {
			size_t bits;
		} integer;

		struct {
			size_t bits;
		} floating;

		struct {
			size_t index;
		} unknown;
	};
} type;

typedef struct define define;

typedef struct callreq {
	char *name;
	size_t ret;
	vec(size_t) args;
} callreq;

typedef struct opreq {
	char *name;
	size_t ret;
	size_t left;
	size_t right;
} opreq;

typedef struct typetable {
	vec(type) types;
	vec(callreq) calls;
	vec(opreq) ops;
	size_t arity;
} typetable;

#define fn(DEF, ARGS, TABLE)                                                   \
	((function){                                                           \
	    .self = (DEF),                                                     \
	    .args = (ARGS),                                                    \
	    .table = (TABLE),                                                  \
	})

enum e_ast {
	A_LIST = 1,
	A_INT,
	A_FLOAT,
	A_CALL,
	A_KW,
	A_OPER,
	A_BLOCK,
	A_REF,
	A_DIAMOND,
};

struct ast {
	enum e_ast tag;
	size_t index;
	union {
		struct {
			vec(ast) items;
		} list;

		struct {
			char *name;
			define *def;
		} ref;

		struct {
			char *name;
		} kw;

		struct {
			int value;
		} integer;

		struct {
			float value;
		} floating;

		struct {
			char *name;
			size_t req;
			vec(ast) args;
		} call;

		struct {
			char *name;
		} diamond;

		struct {
			char *name;
			size_t index;
			struct ast *left;
			struct ast *right;
		} oper;

		struct {
			vec(struct function) functions;
			vec(struct define) defines;
			vec(struct ast) items;
			int after_block;
		} block;
	};
};

struct define {
	char *name;
	size_t index;
	ast init;
};

enum e_funflags {
	FUN_EXTERN = 1 << 1,	 
};

typedef struct function {
	define self;
	vec(define) args;
	typetable table;
	enum e_funflags flags;
} function;

#define def(NAME, INDEX, INIT)                                                 \
	((define){.name = NAME, .index = (INDEX), .init = (INIT)})

#define adiamond(NAME) ((ast){.tag = A_DIAMOND, .diamond = {.name = (NAME)}})

#define akw(NAME) ((ast){.tag = A_KW, .kw = {.name = (NAME)}})

#define aref(NAME) ((ast){.tag = A_REF, .ref = {.name = (NAME)}})

#define aint(V) ((ast){.tag = A_INT, .integer = {.value = (V)}})

#define ablock(FUNS, DEFS, ITEMS) ((ast){.tag = A_BLOCK, .block = {.functions = (FUNS), .defines = (DEFS), .items = (ITEMS)}})

#define alist(ITEMS) ((ast){.tag = A_LIST, .list = {.items = (ITEMS)}})

#define acall(NAME, ARGS)                                                      \
	((ast){.tag = A_CALL, .call = {.name = (NAME), .args = (ARGS)}})

#define aoper(NAME, L, R)                                                      \
	((ast){.tag = A_OPER,                                                  \
	       .oper = {.name = (NAME), .left = (L), .right = (R)}})

#define R(x) &aref(x)
#define L(x) &alist(x)
#define K(x) &akw(x)
#define O(x) &aoper(x, 0, 0)

typedef struct macro {
	char *name;
} macro;

typedef struct impl {
	function* fp;
	typetable table;
} impl;

typedef struct output {
	deq(char) declarations;
	deq(char) definitions;
	vec(impl) implementations;
} output;

typedef struct context {
	output* out;
	vec(define) defines;
	vec(macro) macros;
	vec(function) functions;
	vec(type) types;
	vec(char*) includes;
	struct context *next;
} context;

static inline ast alist0()
{
	vec(ast) items = avec(ast);
	return alist(items);
}

static inline ast alist1(ast a)
{
	vec(ast) items = avec(ast);
	push(items, a);
	return alist(items);
}

static inline ast append(ast l, ast a)
{
	assert(l.tag == A_LIST);
	push(l.list.items, a);
	return l;
}

#endif // DATA_H
