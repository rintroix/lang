#ifndef COMMON_H_
#define COMMON_H_

#include "tbox/tbox.h"

enum ast_type { A_FN, A_CALL, A_KW, A_REF, A_DEF, A_OPER, A_MARK };

typedef struct ast {
	enum ast_type type;
	union {
		struct {
			char *name;
		} fn;

		struct {
			char *name;
			tb_stack_ref_t args;
		} call;

		struct {
			char *name;
		} kw;

		struct {
			char *name;
		} def;

		struct {
			char *name;
		} ref;

		struct {
			char *name;
			struct ast *left;
			struct ast *right;
		} oper;
	};
} ast;

ast word(char *name);
ast keyword(char *name);
ast oper(char *name, ast l, ast r);
ast dot(ast l, ast r);
ast list(tb_stack_ref_t children);
ast list0();
ast list1(ast a);
ast append(ast l, ast a);
void printl_ast(ast *t);
void push(tb_stack_ref_t stack, ast a);

#endif // COMMON_H_
