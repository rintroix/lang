#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define TB_TRACE_MODULE_NAME "main"
#include "tbox/tbox.h"

#include "common.h"
#include "parser.h"

tb_element_t token_element;
tb_element_t ast_element;

void print_token(token *t) {
	switch(t->type) {
	case T_LIST:
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

	case T_WORD:
		printf("%s", t->word.name);
		break;

	case T_OPER:
		printf("(");
		print_token(t->oper.left);
		printf(" ");
		printf("%s", t->oper.name);
		printf(" ");
		print_token(t->oper.right);
		printf(")");
		break;

	default:
		tb_trace_e("print_token: unhandled %d", t->type);
		abort();
		break;
	}
}

void printl_token(token *t) {
	print_token(t);
	puts("");
}

token *lift(token t) {
	token *ptr = malloc(sizeof(token));
	*ptr = t;
	return ptr;
}

token word(char *name) {
	return (token){ .type = T_WORD, .word = { .name = name } };
}

token oper(char *name, token l, token r) {
	return (token){ .type = T_OPER,
		.oper = { .name = name, .left = lift(l), .right = lift(r) } };
}

token list(tb_stack_ref_t stack) {
	tb_stack_ref_t children = tb_stack_init(tb_iterator_size(stack), token_element);
	tb_stack_copy(children, stack);
	tb_stack_clear(stack);
	return (token){ .type = T_LIST, .list = { .children = children } };
}

token list0() {
	tb_stack_ref_t children = tb_stack_init(0, token_element);
	return (token){ .type = T_LIST, .list = { .children = children } };
}

token list1(token t) {
	tb_stack_ref_t children = tb_stack_init(10, token_element);
	tb_stack_put(children, &t);
	return (token){ .type = T_LIST, .list = { .children = children } };
}

token append(token l, token t) {
	tb_check_abort(l.type == T_LIST);
	tb_stack_put(l.list.children, &t);
	return l;
}

void push(tb_stack_ref_t stack, token t) {
	tb_stack_put(stack, &t);
}

void destroy_ast(ast *node) {
	switch (node->type) {
		case A_FN:
			tb_free(node->fn.name);
			tb_trace_noimpl();
			break;

		case A_CALL:
			tb_free(node->call.name);
			tb_for_all(ast*, item, node->call.args) {
				destroy_ast(item);
			}
			tb_list_clear(node->call.args);
			tb_list_exit(node->call.args);
			break;

		case A_REF:
			tb_free(node->ref.name);
			break;

		case A_DEF:
			tb_free(node->def.name);
			tb_trace_noimpl();
			break;
	}
}

ast token_to_ast(token *t) {
	switch(t->type) {
	case T_LIST:;
		tb_list_ref_t children = t->list.children;
		token *head = tb_list_head(children);

		if (head->type == T_WORD) {
			char *name = head->word.name;
			tb_list_ref_t args = tb_list_init(0, ast_element);
			tb_for_all(token*, item, children) {
				if (item == head) continue;
				ast a = token_to_ast(item);
				tb_list_insert_tail(args, &a);
			}
			tb_list_clear(children);
			tb_list_exit(children);
			return (ast) {
				.type = A_CALL,
					.call = {
						.name = name,
						.args = args,
					},
			};
		} else {
			tb_trace_e("FIXME bad call head %d", head->type);
			printf(" -> ");
			printl_token(t);
			abort();
		}
		break;

	case T_WORD:;
		return (ast) {
			.type = A_REF,
			.ref = {
				.name = t->word.name,
			},
		};
		break;

	case T_OPER:;
		return (ast) {
			.type = A_REF,
			.ref = {
				.name = t->atom.name,
			},
		};
		break;

	default:
		tb_trace_e("unexpected token type %d", head->type);
		abort();
		break;
	}
}

void print_ast(ast *node) {
	switch (node->type) {
		case A_FN:
			tb_trace_noimpl();
			break;

		case A_CALL:
			printf("%s(", node->call.name);
			tb_for_all(ast*, item, node->call.args) {
				if (item != tb_list_head(node->call.args)) {
					printf(", ");
				}
				print_ast(item);
			}
			printf(")");
			break;

		case A_REF:
			printf("%s", node->ref.name);
			break;

		case A_DEF:
			tb_trace_noimpl();
			break;
	}
}

void manage_token(token *t) {
	printl_token(t);
	ast node = token_to_ast(t);
	print_ast(&node);
	destroy_ast(&node);
}

int main() {
	tb_check_abort(tb_init(0, 0));

	token_element = tb_element_mem(sizeof(token), 0, 0);
	ast_element = tb_element_mem(sizeof(ast), 0, 0);

	tb_stack_ref_t stack = tb_stack_init(128, token_element);

	token t;
	pcc_context_t *ctx = pcc_create(stack);
	while(pcc_parse(ctx, &t)) {
		manage_token(&t);
	}
	if (t.type != 0) {
		manage_token(&t);
	}
	pcc_destroy(ctx);

	tb_stack_clear(stack);
	tb_stack_exit(stack);
	tb_trace_d("END");

	tb_exit();
}
