#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define TB_TRACE_MODULE_NAME "main"
#include "tbox/tbox.h"

#include "common.h"
#include "parser.h"
#include "string.h"

tb_element_t ast_element;
tb_element_t ast_ref_element;

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

	case A_ID:
		if (a->id.type == I_KW)
			printf(":");
		printf("%s", a->id.name);
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

ast kw(char *name) {
	return (ast){ .type = A_ID, .id = { .name = name, .type = I_KW } };
}

ast word(char *name) {
	return (ast){ .type = A_ID, .id = { .name = name, .type = I_WORD } };
}

ast op(char *name) {
	return (ast){ .type = A_ID, .id = { .name = name, .type = I_OP } };
}

ast oper(char *name, ast l, ast r) {
	return (ast){ .type = A_OPER, .oper = { .name = name, .left = lift(l), .right = lift(r) } };
}

tb_bool_t ismark(tb_iterator_ref_t iterator, tb_cpointer_t item, tb_cpointer_t value) {
	(void)iterator;
	(void)value;
	return ((ast*)item)->type == A_MARK;
}

ast call(tb_stack_ref_t args) {
	return (ast){ .type = A_CALL, .call = { .args = args } };
}

ast list(tb_stack_ref_t stack) {
	tb_stack_ref_t args = tb_stack_init(tb_iterator_size(stack), ast_element);
	size_t mark = tb_rfind_if(stack, 0, tb_iterator_size(stack), ismark, 0);
	tb_for(ast*, item, mark + 1, tb_iterator_size(stack), stack) {
		tb_stack_put(args, item);
	}
	while (((ast*)tb_stack_last(stack))->type != A_MARK) {
		tb_stack_pop(stack);
	}
	tb_stack_pop(stack); // mark
	return call(args);
}

ast list0() {
	tb_stack_ref_t args = tb_stack_init(0, ast_element);
	return call(args);
}

ast list1(ast a) {
	tb_stack_ref_t args = tb_stack_init(1, ast_element);
	tb_stack_put(args, &a);
	return call(args);
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

		case A_ID:
			tb_free(a->id.name);
			break;

		case A_DEF:
			tb_free(a->def.name);
			break;

		case A_OPER:
			tb_free(a->oper.name);
			if (a->oper.left) destroy_ast(a->oper.left);
			if (a->oper.right) destroy_ast(a->oper.right);
			break;

		case A_MARK:
			tb_trace_e("compiler bug: mark destroyed");
			tb_abort();
			break;
	}
}

void comp(tb_list_ref_t context, ast *a) {
	
}

void compile(ast *a) {
	tb_list_ref_t context = tb_list_init(0, ast_ref_element);
	comp(context, a);
}

tb_iterator_ref_t funargs(tb_iterator_ref_t list) {
	tb_vector_ref_t out = tb_vector_init(tb_iterator_size(list), ast_element);
	
	return out;
}

int _match(tb_list_ref_t list, ast *pat) {
	tb_for_all(ast*, a, list) {
		if (pat->type == A_MARK)
			break;

		if (a->type != pat->type) {
			return 0;
		}

		switch(pat->type) {
			case A_CALL:
				break;
			case A_ID:
				if (pat->id.name && pat->id.type == a->id.type && 0 != strcmp(pat->id.name, a->id.name)) {
					return 0;
				}
				break;
			default:
				tb_trace_e("match unhandled for %d", pat->type);
				tb_abort();
				break;
		}

		pat++;
	}	

	return 1;
}

#define match(list, ...) _match(list, (ast[]){__VA_ARGS__, (ast){.type=A_MARK}})

#define get(x, y) ((ast*)tb_iterator_item(x, y))

#define W(x) word(x)
#define L(x) call(x)
#define K(x) kw(x)
#define O(x) op(x)

ast *transform(tb_list_ref_t macros, ast *a) {
	// TODO compactor first

	if (a->type == A_CALL) {
		tb_iterator_ref_t list = a->call.args;
		if (match(list, W("fn"), W(0), K(0), L(0))) {
			puts("MATCHED");
			char *name = get(list, 1)->id.name;
			char *ret  = get(list, 2)->id.name;
			tb_iterator_ref_t args = funargs(get(list, 3)->call.args);
			tb_trace_i("FN %s ARGS %d", name, tb_iterator_size(args));
		}
	}
	return a;
}

void manage_ast(ast *a) {
	printl_ast(a);
	tb_list_ref_t macros = tb_list_init(0, tb_element_ptr(0, 0));
	ast *b = transform(macros, a);
	compile(b);
	destroy_ast(b);
}

int main() {
	tb_check_abort(tb_init(0, 0));

	ast_element = tb_element_mem(sizeof(ast), 0, 0);
	ast_ref_element = tb_element_ptr(0, 0);

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
