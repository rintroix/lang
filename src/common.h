#ifndef COMMON_H_
#define COMMON_H_

#include "tbox/tbox.h"

enum token_type { TOKEN_OPEN, TOKEN_ATOM, TOKEN_LIST, TOKEN_DOT };
enum ast_type { AST_FN, AST_CALL, AST_REF, AST_DEF };

typedef struct token {
	enum token_type type;
	union {
		struct {
			char *name;
		} atom;

		struct {
			tb_list_ref_t children;
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

void push_atom(tb_stack_ref_t stack, char *name);
void push_open(tb_stack_ref_t stack);
void push_dot(tb_stack_ref_t stack);
void collect_list(tb_stack_ref_t stack);

#endif // COMMON_H_
