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

enum e_type { T_UNKNOWN, T_BUILTIN, T_COMPOUND };

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
	type type;
	struct ast *init;
} define;

typedef struct function {
	define self;
	vec(define) args;
} function;

#define fn(DEF, ARGS) ((function){.self = (DEF), .args = (ARGS)})

enum e_ast {
	A_LIST = 1,
	A_CALL,
	A_ID,
	A_OPER,
	A_BLOCK,
	A_MARK
};

enum e_id { I_WORD, I_KW, I_OP, I_INT };


typedef struct ast ast;

typedef struct block {
	vec(function) functions;
	vec(define) defines;
	vec(ast) items;
} block;

struct ast {
	enum e_ast tag;
	union {
		struct {
			vec(ast) items;
		} list;

		struct {
			char *name;
			vec(ast) args;
		} call;

		struct {
			char *name;
			enum e_id tag;
		} id;

		struct {
			char *name;
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

#define acall(NAME, ARGS)                                                       \
	((ast){.tag = A_CALL, .call = {.name = (NAME), .args = (ARGS)}})

#define aoper(NAME, L, R)                                                       \
	((ast){.tag = A_OPER,                                                 \
	       .oper = {.name = (NAME), .left = (L), .right = (R)}})

#define def(NAME, TYPE, INIT)                                                  \
	((define){.name = NAME, .type = (TYPE), .init = (INIT)})

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

typedef struct scope {
	vec(function) functions;
	vec(struct let) lets;
	vec(type) types;
	struct scope *next;
} scope;

typedef struct candidate {
	function *function;
	struct rule *body;
} candidate;

#define can(F, B) ((candidate){.function = (F), .body = (B)})

typedef struct request {
	struct scope *scope;
	vec(struct rule) args;
	vec(candidate) candidates;
} request;

enum e_rule {
	R_EMPTY = 1,
	R_IS,
	R_REQ,
};

typedef struct rule {
	enum e_rule tag;
	union {
		type *is;

		request req;
	};
} rule;

typedef struct let {
	char *name;
	struct rule rule;	
} let;

#endif // DATA_H
