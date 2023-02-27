#ifndef COMMON_H_
#define COMMON_H_

#include "tbox/tbox.h"

typedef struct define {
	char *name;
	char *type;
	struct ast *init;
} define;

enum ast_type {
	A_FN = 1,
	A_LIST,
	A_CALL,
	A_ID,
	A_OPER,
	A_BLOCK,
	A_MARK
};

enum id_type { I_WORD, I_KW, I_OP };

typedef struct block {
	tb_iterator_ref_t defs;
	tb_iterator_ref_t items;
} block;

typedef struct ast {
	enum ast_type type;
	union {
		struct {
			define def;
			tb_iterator_ref_t args;
		} fn;

		struct {
			tb_iterator_ref_t items;
		} list;

		struct {
			char *name;
			tb_iterator_ref_t args;
		} call;

		struct {
			char *name;
			enum id_type type;
		} id;

		struct {
			char *name;
			struct ast *left;
			struct ast *right;
		} oper;

		block block;
	};
} ast;

#define op(NAME)                                                               \
	(ast)                                                                  \
	{                                                                      \
		.type = A_ID, .id = {.name = (NAME), .type = I_OP }            \
	}

#define kw(NAME)                                                               \
	(ast)                                                                  \
	{                                                                      \
		.type = A_ID, .id = {.name = (NAME), .type = I_KW }            \
	}

#define word(NAME)                                                             \
	(ast)                                                                  \
	{                                                                      \
		.type = A_ID, .id = {.name = (NAME), .type = I_WORD }          \
	}

#define ablock(BLOCK)                                                          \
	(ast)                                                                  \
	{                                                                      \
		.type = A_BLOCK, .block = (BLOCK)                              \
	}

#define list(ITEMS)                                                            \
	(ast)                                                                  \
	{                                                                      \
		.type = A_LIST, .list = {.items = (ITEMS) }                    \
	}

#define call(NAME, ARGS)                                                       \
	(ast)                                                                  \
	{                                                                      \
		.type = A_CALL, .call = {.name = (NAME), .args = (ARGS) }      \
	}

#define oper(NAME, L, R)                                                       \
	(ast)                                                                  \
	{                                                                      \
		.type = A_OPER, .oper = {                                      \
			.name = (NAME),                                        \
			.left = (L),                                           \
			.right = (R)                                           \
		}                                                              \
	}

#define fn(DEF, ARGS)                                                          \
	(ast)                                                                  \
	{                                                                      \
		.type = A_FN, .fn = {.def = (DEF), .args = (ARGS) }            \
	}

#define def(NAME, TYPE, INIT)                                                  \
	(define)                                                               \
	{                                                                      \
		.name = NAME, .type = (TYPE), .init = (INIT)                   \
	}

ast dot(ast l, ast r);
ast list0();
ast list1(ast a);
ast append(ast l, ast a);
void printl_ast(ast *t);

#define W(x) &word(x)
#define L(x) &list(x)
#define K(x) &kw(x)
#define O(x) &op(x)
#define F(x) &fn(def(x, 0, 0))

typedef struct macro {
	char *name;
} macro;

typedef struct scope {
	tb_iterator_ref_t functions;
	struct scope *next;
} scope;

typedef struct request {
	tb_iterator_ref_t args;
	tb_iterator_ref_t rules;
	tb_iterator_ref_t candidates;
} request;

enum e_rule {
	R_EMPTY = 1,
	R_IS,
	R_CALL,
};

typedef struct rule {
	enum e_rule type;
	union {
		struct {
			char *name;
		} is;
	};
} rule;

#endif // COMMON_H_
