#ifndef COMMON_H_
#define COMMON_H_

#include "tbox/tbox.h"

enum token_type { T_WORD = 1, T_LIST, T_OPER };
enum ast_type { A_FN, A_CALL, A_REF, A_DEF };

typedef struct token {
	enum token_type type;
	union {
		struct {
			char *name;
		} word;

		struct {
			char *name;
			struct token *left;
			struct token *right;
		} oper;

		struct {
			tb_stack_ref_t children;
		} list;
	};
} token;

typedef struct head {
	struct token *token;
} head;

typedef struct ast {
	enum ast_type type;
	union {
		struct {
			char *name;
		} fn;

		struct {
			char *name;
			tb_list_ref_t args;
		} call;

		struct {
			char *name;
		} def;

		struct {
			char *name;
		} ref;
	};
} ast;

token word(char *name);
token oper(char *name, token l, token r);
token dot(token l, token r);
token list(tb_stack_ref_t children);
token list0();
token list1(token t);
token append(token l, token t);
void printl_token(token *t);
void push(tb_stack_ref_t stack, token t);
// void collect_list(tb_stack_ref_t stack);

#endif // COMMON_H_
