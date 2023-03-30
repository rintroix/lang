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

typedef struct typeindex {
	size_t value;
} typeindex;

enum e_ir {
	I_SKIP = 1,	
	I_DEF,
	I_REF,
	I_CALL,
	I_OPER,
	I_LINT,
	I_LFLT,
	I_LSTR,
};

typedef struct ir {
	enum e_ir tag;	
	union {
		struct {
			size_t count;
		} skip;

		struct {
			typeindex index;
			size_t count;
			char *name;
		} def;

		struct {
			size_t index; // of def
			char *name;
		} ref;

		struct {
			size_t index; // in callreqs
			size_t count; // args
			size_t dst;
		} call;

		struct {
			size_t index; // in opreqs
			size_t count; // operands in a tree
			size_t dst;
		} oper;

		struct {
			typeindex index; // type
			int value;
			size_t dst;
		} lint;
	};
} ir;

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

typedef struct opreq {
	int isop; // TODO bool
	union {
		struct {
			char *name;
			struct opreq* left;
			struct opreq* right;
		} op;

		struct {
			size_t index; // in ir
		} val;
	};
} opreq;

typedef struct define define;

typedef struct callreq {
	char *name;
	size_t ret;
	vec(size_t) args;

	size_t arity;
	size_t argsidx;
	size_t defidx;
} callreq;

typedef struct typetable {
	vec(type) types;
	vec(callreq) calls;
	vec(opreq) ops;
	vec(size_t) uses;
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
	A_STR,
	A_FLOAT,
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
			char *value;
		} str;

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

typedef struct ir_function {
	char *name;
	typetable table;
	vec(ir) body;
	enum e_funflags flags;
} ir_function;

#define def(NAME, INDEX, INIT)                                                 \
	((define){.name = NAME, .index = (INDEX), .init = (INIT)})

#define iset(I) ((ir){.tag = I_SET, .set = {.index = I}})
#define iref(N, I) ((ir){.tag = I_REF, .ref = {.index = I, .name = N}})
#define idef(N, I) ((ir){.tag = I_DEF, .def = {.index = I, .name = N}})
#define ioper(I, N) ((ir){.tag = I_OPER, .oper = {.index = I, .count = N}})
#define iskip(N) ((ir){.tag = I_SKIP, .skip = {.count = N}})
#define ilint(I, V) ((ir){.tag = I_LINT, .lint = {.index = I, .value = V}})

#define adiamond(N) ((ast){.tag = A_DIAMOND, .diamond = {.name = (N)}})

#define akw(N) ((ast){.tag = A_KW, .kw = {.name = (N)}})

#define aref(N) ((ast){.tag = A_REF, .ref = {.name = (N)}})

#define astr(V) ((ast){.tag = A_STR, .str = {.value = (V)}})

#define aint(V) ((ast){.tag = A_INT, .integer = {.value = (V)}})

#define ablock(FUNS, DEFS, ITEMS)                                              \
	((ast){.tag = A_BLOCK,                                                 \
	       .block = {                                                      \
		   .functions = (FUNS), .defines = (DEFS), .items = (ITEMS)}})

#define alist(ITEMS) ((ast){.tag = A_LIST, .list = {.items = (ITEMS)}})

#define aoper(N, L, R)                                                         \
	((ast){.tag = A_OPER, .oper = {.name = (N), .left = (L), .right = (R)}})

#define R(x) &aref(x)
#define L(x) &alist(x)
#define K(x) &akw(x)
#define O(x) &aoper(x, NULL, NULL)
#define R0 R(NULL)
#define L0 L(NULL)
#define K0 K(NULL)
#define O0 O(NULL)

typedef struct macro {
	char *name;
} macro;

typedef struct impl {
	char *name;
	function *fp;
	typetable table;
} impl;

typedef struct output {
	deq(char) declarations;
	deq(char) definitions;
	vec(impl) implementations;
} output;

typedef struct context {
	output *out;
	vec(define) defines;
	vec(macro) macros;
	vec(function) functions;
	vec(ir_function) irfns;
	vec(type) types;
	vec(char *) includes;
	struct context *next;
} context;

static inline ast alist0(void)
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
