#ifndef DATA_H
#define DATA_H

#include "um/um.h"

#define vec(T) um_vec(T)
#define avec(V) (um_vec_alloc(V))
#define push(V, ...) um_vec_push(V, __VA_ARGS__)
#define vlen(V) (um_vec_len(V))
#define vat(V, N) (um_vec_at(V, N))
#define vget(V, N) (*vat(V, N))
#define forv(...) um_vec_for(__VA_ARGS__)
#define forvr(...) um_vec_for_range(__VA_ARGS__)
#define vslice(V, START, END) (um_vec_slice(V, START, END))

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

typedef struct define {
	char *name;
	size_t index;
	ast *init;
} define;

#define def(NAME, INDEX, INIT)                                                  \
	((define){.name = NAME, .index = (INDEX), .init = (INIT)})

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
	type ret;
	vec(type) args;
	vec(type) body;
	vec(callreq) calls;
	vec(opreq) ops;
} typetable;
 
typedef struct function {
	define self;
	vec(define) args;
	typetable table;
} function;

#define fn(DEF, ARGS, TABLE)                                                   \
	((function){                                                           \
	    .self = (DEF),                                                     \
	    .args = (ARGS),                                                    \
	    .table = (TABLE),                                                  \
	})

enum e_ast { A_LIST = 1, A_CALL, A_ID, A_OPER, A_BLOCK, A_REF, A_MARK };

enum e_id { I_WORD, I_KW, I_OP, I_INT };

typedef struct block {
	vec(function) functions;
	vec(define) defines;
	vec(ast) items;
} block;

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

		struct block block;
	};
};

#define aop(NAME) ((ast){.tag = A_ID, .id = {.name = (NAME), .tag = I_OP}})

#define akw(NAME) ((ast){.tag = A_ID, .id = {.name = (NAME), .tag = I_KW}})

#define aword(NAME) ((ast){.tag = A_ID, .id = {.name = (NAME), .tag = I_WORD}})

#define aint(NAME) ((ast){.tag = A_ID, .id = {.name = (NAME), .tag = I_INT}})

#define ablock(BLOCK) ((ast){.tag = A_BLOCK, .block = (BLOCK)})

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

ast alist0();
ast alist1(ast a);
ast append(ast l, ast a);
void printl_ast(ast *t);

typedef struct macro {
	char *name;
} macro;

typedef struct parse_ctx {
	vec(define) defines;
	vec(macro) macros;
	vec(function) functions;
	struct parse_ctx *next;
} parse_ctx;

typedef struct scope {
	vec(function) functions;
	vec(struct let) lets;
	vec(type) types;
	struct scope *next;
} scope;

#endif // DATA_H
