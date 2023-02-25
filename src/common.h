#ifndef COMMON_H_
#define COMMON_H_

#include "tbox/tbox.h"

enum ast_type { A_FN, A_LIST, A_CALL, A_ID, A_DEF, A_OPER, A_BLOCK, A_MARK };
enum id_type { I_WORD, I_KW, I_OP };

typedef struct ast {
	enum ast_type type;
	union {
		struct {
			char *name;
			char *type;
			tb_iterator_ref_t args;
			struct ast *body;
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
			char *type;
		} def;

		struct {
			char *name;
			enum id_type type;
		} id;

		struct {
			char *name;
			struct ast *left;
			struct ast *right;
		} oper;

		struct {
			tb_iterator_ref_t items;
		} block;
	};
} ast;

#define op(N)                                                                  \
	(ast)                                                                  \
	{                                                                      \
		.type = A_ID, .id = {.name = (N), .type = I_OP }               \
	}

#define kw(N)                                                                  \
	(ast)                                                                  \
	{                                                                      \
		.type = A_ID, .id = {.name = (N), .type = I_KW }               \
	}

#define word(N)                                                                \
	(ast)                                                                  \
	{                                                                      \
		.type = A_ID, .id = {.name = (N), .type = I_WORD }             \
	}

#define block(ITEMS)                                                           \
	(ast)                                                                  \
	{                                                                      \
		.type = A_BLOCK, .block = {.items = (ITEMS) }                  \
	}

#define def(N, T)                                                              \
	(ast)                                                                  \
	{                                                                      \
		.type = A_DEF, .def = {.name = (N), .type = (T) }              \
	}

#define list(ITEMS)                                                            \
	(ast)                                                                  \
	{                                                                      \
		.type = A_LIST, .list = {.items = (ITEMS) }                    \
	}

#define call(ARGS)                                                             \
	(ast)                                                                  \
	{                                                                      \
		.type = A_CALL, .call = {.args = (ARGS) }                      \
	}

#define oper(N, L, R)                                                          \
	(ast)                                                                  \
	{                                                                      \
		.type = A_OPER, .oper = {                                      \
			.name = (N),                                           \
			.left = (L),                                           \
			.right = (R)                                           \
		}                                                              \
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

#endif // COMMON_H_
