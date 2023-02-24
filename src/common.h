#ifndef COMMON_H_
#define COMMON_H_

#include "tbox/tbox.h"

enum ast_type { A_FN, A_CALL, A_ID, A_DEF, A_OPER, A_BLOCK, A_MARK };
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

ast word(char *name);
ast kw(char *name);
ast op(char *name);
ast oper(char *name, ast l, ast r);
ast dot(ast l, ast r);
ast list(tb_stack_ref_t children);
ast list0();
ast list1(ast a);
ast append(ast l, ast a);
void printl_ast(ast *t);
void push(tb_stack_ref_t stack, ast a);

#endif // COMMON_H_
