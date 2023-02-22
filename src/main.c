#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define TB_TRACE_MODULE_NAME "main"
#include "tbox/tbox.h"

#include "common.h"
#include "parser.h"

tb_element_t ast_element;

void print_ast(ast *a) {
	switch(a->type) {
	case A_CALL:
		printf("(");
		int i = 0;
		tb_for_all(ast*, item, a->call.args) {
			if (i == 0)
				i = 1;
			else
				printf(" ");
			print_ast(item);
		}
		printf(")");
		break;

	case A_KW:
		printf(":%s", a->kw.name);
		break;

	case A_REF:
		printf("%s", a->ref.name);
		break;

	case A_OPER:
		printf("(");
		print_ast(a->oper.left);
		printf(" ");
		printf("%s", a->oper.name);
		printf(" ");
		print_ast(a->oper.right);
		printf(")");
		break;

	default:
		tb_trace_e("print_ast: unhandled %d", a->type);
		abort();
		break;
	}
}

void printl_ast(ast *a) {
	print_ast(a);
	puts("");
}

ast *lift(ast a) {
	ast *ptr = malloc(sizeof(ast));
	*ptr = a;
	return ptr;
}

ast keyword(char *name) {
	return (ast){ .type = A_KW, .kw = { .name = name } };
}

ast word(char *name) {
	return (ast){ .type = A_REF, .ref = { .name = name } };
}

ast oper(char *name, ast l, ast r) {
	return (ast){ .type = A_OPER,
		.oper = { .name = name, .left = lift(l), .right = lift(r) } };
}

tb_bool_t ismark(tb_iterator_ref_t iterator, tb_cpointer_t item, tb_cpointer_t value) {
	(void)iterator;
	(void)value;
	return ((ast*)item)->type == 0;
}

ast list(tb_stack_ref_t stack) {
	tb_stack_ref_t args = tb_stack_init(tb_iterator_size(stack), ast_element);
	size_t mark = tb_rfind_if(stack, 0, tb_iterator_size(stack), ismark, 0);
	tb_for(ast*, item, mark + 1, tb_iterator_size(stack), stack) {
		tb_stack_put(args, item);
	}
	while (((ast*)tb_stack_last(stack))->type != 0) {
		tb_stack_pop(stack);
	}
	tb_stack_pop(stack); // marker
	return (ast){ .type = A_CALL, .call = { .args = args } };
}

ast list0() {
	tb_stack_ref_t args = tb_stack_init(0, ast_element);
	return (ast){ .type = A_CALL, .call = { .args = args } };
}

ast list1(ast a) {
	tb_stack_ref_t args = tb_stack_init(1, ast_element);
	tb_stack_put(args, &a);
	return (ast){ .type = A_CALL, .call = { .args = args } };
}

ast append(ast l, ast a) {
	tb_check_abort(l.type == A_CALL);
	tb_stack_put(l.call.args, &a);
	return l;
}

void push(tb_stack_ref_t stack, ast a) {
	tb_stack_put(stack, &a);
}

void destroy_ast(ast *a) {
	switch (a->type) {
		case A_FN:
			tb_free(a->fn.name);
			break;

		case A_CALL:
			if (a->call.name)
				tb_free(a->call.name); // TODO hack, need empty list ast
			tb_for_all(ast*, item, a->call.args) {
				destroy_ast(item);
			}
			tb_stack_clear(a->call.args);
			tb_stack_exit(a->call.args);
			break;

		case A_KW:
			tb_free(a->kw.name);
			break;

		case A_REF:
			tb_free(a->ref.name);
			break;

		case A_DEF:
			tb_free(a->def.name);
			break;

		case A_OPER:
			tb_free(a->oper.name);
			if (a->oper.left) destroy_ast(a->oper.left);
			if (a->oper.right) destroy_ast(a->oper.right);
			break;
	}
}

void manage_ast(ast *a) {
	printl_ast(a);
	destroy_ast(a);
}

int main() {
	tb_check_abort(tb_init(0, 0));

	ast_element = tb_element_mem(sizeof(ast), 0, 0);

	tb_stack_ref_t stack = tb_stack_init(128, ast_element);

	ast a;
	pcc_context_t *ctx = pcc_create(stack);
	while(pcc_parse(ctx, &a)) {
		manage_ast(&a);
	}
	if (a.type != 0) {
		manage_ast(&a);
	}
	pcc_destroy(ctx);

	tb_stack_clear(stack);
	tb_stack_exit(stack);
	tb_trace_d("END");

	// tb_exit();
}
