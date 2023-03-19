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

enum e_type { T_UNKNOWN = 1, T_SIMPLE, T_COMPOUND };

typedef struct ast ast;

typedef struct type {
	enum e_type tag;
	char *name;
	union {
		struct {
			vec(struct type) items;
		} compound;
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

enum e_ast { A_LIST = 1, A_CALL, A_ID, A_OPER, A_BLOCK, A_REF };

enum e_id { I_WORD, I_KW, I_OP, I_INT };

struct ast {
	enum e_ast tag;
	size_t index;
	union {
		struct {
			vec(ast) items;
		} list;

		struct {
			define *def;
		} ref;

		struct {
			char *name;
			size_t index;
			vec(ast) args;
		} call;

		struct {
			char *name;
			enum e_id tag;
		} id;

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

typedef struct function {
	define self;
	vec(define) args;
	typetable table;
} function;

#define def(NAME, INDEX, INIT)                                                 \
	((define){.name = NAME, .index = (INDEX), .init = (INIT)})

#define aop(NAME) ((ast){.tag = A_ID, .id = {.name = (NAME), .tag = I_OP}})

#define akw(NAME) ((ast){.tag = A_ID, .id = {.name = (NAME), .tag = I_KW}})

#define aword(NAME) ((ast){.tag = A_ID, .id = {.name = (NAME), .tag = I_WORD}})

#define aint(NAME) ((ast){.tag = A_ID, .id = {.name = (NAME), .tag = I_INT}})

#define ablock(FUNS, DEFS, ITEMS) ((ast){.tag = A_BLOCK, .block = {.functions = (FUNS), .defines = (DEFS), .items = (ITEMS)}})

#define alist(ITEMS) ((ast){.tag = A_LIST, .list = {.items = (ITEMS)}})

#define acall(NAME, ARGS)                                                      \
	((ast){.tag = A_CALL, .call = {.name = (NAME), .args = (ARGS)}})

#define aoper(NAME, L, R)                                                      \
	((ast){.tag = A_OPER,                                                  \
	       .oper = {.name = (NAME), .left = (L), .right = (R)}})

#define W(x) &aword(x)
#define L(x) &alist(x)
#define K(x) &akw(x)
#define O(x) &aop(x)

typedef struct macro {
	char *name;
} macro;

typedef struct context {
	vec(define) defines;
	vec(macro) macros;
	vec(function) functions;
	struct context *next;
} context;

typedef struct impl {
	function* fp;
	typetable table;
} impl;

typedef struct output {
	deq(char) declarations;
	deq(char) definitions;
	vec(impl) implementations;
} output;

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
