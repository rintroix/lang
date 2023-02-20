#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define TB_TRACE_MODULE_NAME "main"
#include "tbox/tbox.h"

#include "common.h"
#include "parser.h"

tb_element_t token_element;
tb_element_t ast_element;

void push_atom(tb_stack_ref_t stack, char *name) {
	tb_stack_put(stack, &(token){ .type = TOKEN_ATOM, .atom = { .name = name } });
}

void push_open(tb_stack_ref_t stack) {
	tb_stack_put(stack, &(token){ .type = TOKEN_OPEN });
}

void push_dot(tb_stack_ref_t stack) {
	tb_stack_put(stack, &(token){ .type = TOKEN_DOT });
}

void push_list(tb_stack_ref_t stack, tb_list_ref_t children) {
	tb_stack_put(stack, &(token){ .type = TOKEN_LIST, .list = { .children = children } });
}

void collect_list(tb_stack_ref_t stack) {
	tb_list_ref_t children = tb_list_init(10, token_element);

	while (1) {
		token t = *(token*)tb_stack_top(stack);
		tb_stack_pop(stack);

		if (t.type == TOKEN_OPEN) {
			push_list(stack, children);
			return;
		}

		if (t.type == TOKEN_DOT) {
			push_list(stack, children);
			children = tb_list_init(10, token_element);
		} else {
			tb_list_insert_head(children, &t);
		}
	}
}

void print_token(token *t) {
	switch(t->type) {
	case TOKEN_LIST:
		printf("(");
		int i = 0;
		tb_for_all(token*, item, t->list.children) {
			if (i == 0)
				i = 1;
			else
				printf(" ");
			print_token(item);
		}
		printf(")");
		break;

	case TOKEN_ATOM:
		printf("%s", t->atom.name);
		break;

	default:
		assert(2 == 3);
		break;
	}
}

void destroy_ast(ast *node) {
	switch (node->type) {
		case AST_FN:
			tb_free(node->fn.name);
			tb_trace_noimpl();
			break;

		case AST_CALL:
			tb_free(node->call.name);
			tb_for_all(ast*, item, node->call.args) {
				destroy_ast(item);
			}
			tb_list_clear(node->call.args);
			tb_list_exit(node->call.args);
			break;

		case AST_REF:
			tb_free(node->ref.name);
			break;

		case AST_DEF:
			tb_free(node->def.name);
			tb_trace_noimpl();
			break;
	}
}

ast token_to_ast(token *t) {
	switch(t->type) {
	case TOKEN_LIST:;
		tb_list_ref_t children = t->list.children;
		token *head = tb_list_head(children);

		if (head->type == TOKEN_ATOM) {
			char *name = head->atom.name;
			tb_list_remove_head(children);
			tb_list_ref_t args = tb_list_init(10, ast_element);
			tb_for_all(token*, item, children) {
				ast a = token_to_ast(item);
				tb_list_insert_tail(args, &a);
			}
			tb_list_clear(children);
			tb_list_exit(children);
			return (ast) {
				.type = AST_CALL,
					.call = {
						.name = name,
						.args = args,
					},
			};
		} else {
			tb_trace_e("FIXME bad call head %d", head->type);
			printf(" -> ");
			print_token(t);
			puts("");
			abort();
		}
		break;

	case TOKEN_ATOM:;
		ast out = (ast) {
			.type = AST_REF,
			.ref = {
				.name = t->atom.name,
			},
		};
		return out;
		break;

	default:
		tb_trace_e("unexpected token type %d", head->type);
		abort();
		break;
	}
}

void print_ast(ast *node) {
	switch (node->type) {
		case AST_FN:
			tb_trace_noimpl();
			break;

		case AST_CALL:
			printf("%s(", node->call.name);
			tb_for_all(ast*, item, node->call.args) {
				if (item != tb_list_head(node->call.args)) {
					printf(", ");
				}
				print_ast(item);
			}
			printf(")");
			break;

		case AST_REF:
			printf("%s", node->ref.name);
			break;

		case AST_DEF:
			tb_trace_noimpl();
			break;
	}
}

int main() {
	tb_check_abort(tb_init(tb_null, tb_null));

	token_element = tb_element_mem(sizeof(token), 0, 0);
	ast_element = tb_element_mem(sizeof(ast), 0, 0);

	tb_stack_ref_t stack = tb_stack_init(128, token_element);

	pcc_context_t *ctx = pcc_create(stack);
	while(pcc_parse(ctx, NULL)) {
		tb_for_all(token*, item, stack) {
			ast node = token_to_ast(item);
			print_ast(&node);
			destroy_ast(&node);
		}
		puts("");

		tb_stack_clear(stack);
	}
	pcc_destroy(ctx);

	tb_stack_clear(stack);
	tb_stack_exit(stack);
	tb_trace_d("END");

	// tb_exit();
}
