#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "tbox/tbox.h"

#include "common.h"
#include "parser.h"
#include "string.h"

tb_element_t ast_element;
tb_element_t ast_ref_element;

#define todo                                                                   \
	do {                                                                   \
		tb_trace_noimpl();                                             \
		exit(3);                                                       \
	} while (0)

#define bug(...)                                                               \
	do {                                                                   \
		tb_trace_e("compiler bug: " __VA_ARGS__);                      \
		exit(2);                                                       \
	} while (0)

#define error(...)                                                             \
	do {                                                                   \
		tb_trace_e(__VA_ARGS__);                                       \
		exit(1);                                                       \
	} while (0)

tb_iterator_ref_t slice(tb_iterator_ref_t iter, tb_size_t start, tb_size_t end) {
	tb_assert(end >= start);
	tb_vector_ref_t out = tb_vector_init(end - start, ast_element);
	tb_for(ast*, item, start, end, iter) {
		tb_vector_insert_tail(out, item);
	}
	return out;
}

int is(ast *a, ast *pat)
{
	if (!pat)
		return 1;

	switch (pat->type) {
	case A_ID:
		if (pat->id.type != a->id.type)
			return 0;

		if (!pat->id.name)
			return 1;
		else if (0 == strcmp(pat->id.name, a->id.name))
			return 1;
		else
			return 0;
		break;

	case A_LIST:
		if (!pat->list.items)
			return 1;
		todo;
		break;

	case A_CALL:
		todo;
		break;

	default:
		bug("%s: unhandled %d", __func__, pat->type);
	}
}

tb_bool_t it_is(tb_iterator_ref_t iterator, tb_cpointer_t item,
		tb_cpointer_t data)
{
	(void)iterator;
	return is((ast *)item, (ast *)data);
}

int has(tb_iterator_ref_t list, ast *pattern)
{
	tb_size_t pos = tb_find_all_if(list, it_is, pattern);
	return pos != tb_iterator_tail(list);
}

void print_ast(ast *a)
{
	switch (a->type) {
	case A_LIST: {
		printf("(");
		int i = 0;
		tb_for_all(ast *, item, a->list.items)
		{
			if (i != 0)
				printf(" ");
			print_ast(item);
			i++;
		}
		printf(")");
	} break;

	case A_CALL: {
		printf("(");
		int i = 0;
		tb_for_all(ast *, item, a->call.args)
		{
			if (i != 0)
				printf(" ");
			print_ast(item);
			i++;
		}
		printf(")");
	} break;

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
		bug("%s: unhandled %d", __func__, a->type);
		break;
	}
}

void printl_ast(ast *a)
{
	print_ast(a);
	puts("");
}

ast *lift(ast a)
{
	ast *ptr = malloc(sizeof(ast));
	*ptr = a;
	return ptr;
}

tb_bool_t ismark(tb_iterator_ref_t iterator, tb_cpointer_t item,
		 tb_cpointer_t value)
{
	(void)iterator;
	(void)value;
	return ((ast *)item)->type == A_MARK;
}

ast list0()
{
	tb_vector_ref_t items = tb_vector_init(10, ast_element);
	return list(items);
}

ast list1(ast a)
{
	tb_vector_ref_t items = tb_vector_init(10, ast_element);
	tb_vector_insert_tail(items, &a);
	return list(items);
}

ast append(ast l, ast a)
{
	tb_check_abort(l.type == A_LIST);
	tb_vector_insert_tail(l.list.items, &a);
	return l;
}

void destroy_ast(ast *a)
{
	switch (a->type) {
	case A_FN:
		tb_free(a->fn.name);
		if (a->fn.type)
			tb_free(a->fn.type);
		// if (a->fn.args) // TODO
		break;

	case A_LIST: {
		tb_for_all(ast *, item, a->call.args) { destroy_ast(item); }
		tb_stack_clear(a->call.args);
		tb_stack_exit(a->call.args);
	} break;

	case A_CALL: {
		if (a->call.name)
			tb_free(a->call.name); // TODO hack, need empty list ast
		tb_for_all(ast *, item, a->call.args) { destroy_ast(item); }
		tb_stack_clear(a->call.args);
		tb_stack_exit(a->call.args);
	} break;

	case A_ID:
		tb_free(a->id.name);
		break;

	case A_DEF:
		tb_free(a->def.name);
		break;

	case A_OPER:
		tb_free(a->oper.name);
		if (a->oper.left)
			destroy_ast(a->oper.left);
		if (a->oper.right)
			destroy_ast(a->oper.right);
		break;

	case A_MARK:
		bug("mark");
		break;

	case A_BLOCK:
		todo;
		break;
	}
}

void compile(tb_list_ref_t context, ast *a)
{
	switch (a->type) {
	case A_FN: {
		printf("%s %s(", a->fn.type, a->fn.name);
		printf(") {\n");
		tb_for_all(ast *, item, a->fn.body->block.items)
		{
			compile(context, item);
			puts(";");
		}
		printf("}");
		puts("");
	} break;

	case A_LIST: {
		bug("%s: bare list", __func__);
	} break;

	case A_CALL: {
		printf("%s(", a->call.name);
		tb_for_all(ast *, arg, a->call.args)
		{
			if (arg != tb_iterator_item(a->call.args, 0))
				printf(", ");
			compile(context, arg);
		}
		printf(")");
		todo;
	} break;

	case A_DEF:
		todo;
		break;

	case A_ID:
		bug("%s: id %s", __func__, a->id.name);
		break;

	case A_OPER:
		todo;
		break;

	case A_BLOCK:
		todo;
		break;

	case A_MARK:
		bug("%s: mark", __func__);
		break;
	}
}

tb_iterator_ref_t funargs(tb_iterator_ref_t list)
{
	tb_vector_ref_t out =
	    tb_vector_init(tb_iterator_size(list), ast_element);

	ast *last = 0;
	tb_for_all(ast *, arg, list)
	{
		if (arg->type != A_ID)
			error("fun arg not an identifier");

		switch (arg->id.type) {
		case I_WORD:;
			ast *d = lift(def(arg->id.name, 0));
			last = d;
			tb_vector_insert_tail(out, d);
			break;

		case I_KW:
			if (last->def.type)
				error("several keywords after fun arg");
			last->def.type = arg->id.name;
			break;

		default:
			error("unexpected fun arg type: %d", arg->type);
			break;
		}
	}

	return out;
}

ast _operate(tb_iterator_ref_t macros, tb_iterator_ref_t list, tb_size_t start)
{
	tb_size_t pos = tb_find_if(list, start, tb_iterator_tail(list), it_is, O(0));

	if (pos == start) {
		bug("%s: operator without left side", __func__);
	}

	if (pos == tb_iterator_tail(list)) {
		bug("%s: no operator", __func__);
		// TODO next return whole slice
	}

	tb_assert(pos > start);

	tb_vector_ref_t left = slice(list, start, pos - 1);

	ast *op = tb_iterator_item(list, pos);
	tb_trace_d("TTT %s %d %d", op->id.name, op->type, op->id.type);
	tb_assert(op->type == A_ID && op->id.type == I_OP);

	// TODO macros

	return oper(op->id.name, list(left), _operate(macros, list, pos + 1));
}

ast operate(tb_iterator_ref_t macros, tb_iterator_ref_t list)
{
	return _operate(macros, list, 0);
}

int _match(tb_vector_ref_t list, ast **patterns)
{
	tb_for_all(ast *, a, list)
	{
		ast *pat = *patterns;

		if (pat->type == A_MARK)
			return 1;

		// TODO atom pat == 0

		if (!is(a, pat))
			return 0;

		patterns++;
	}

	return 0;
}

#define match(list, ...)                                                       \
	_match(list, (ast *[]){__VA_ARGS__, &(ast){.type = A_MARK}})

#define get(x, y) ((ast *)tb_iterator_item(x, y))

ast *transform(tb_iterator_ref_t macros, ast *a)
{
	// TODO compactor first

	if (a->type == A_LIST) {
		tb_iterator_ref_t items = a->list.items;
		tb_vector_ref_t bodies = tb_vector_init(10, ast_element);
		tb_trace_d("LIST SIZE %d", tb_iterator_size(items));
		printl_ast(a);
		if (match(items, W("fn"), W(0), K(0), L(0))) {
			puts("MATCHED");
			char *name = get(items, 1)->id.name;
			char *type = get(items, 2)->id.name;
			tb_iterator_ref_t args =
			    funargs(get(items, 3)->list.items);
			tb_for(ast *, b, 4, tb_iterator_tail(items), items)
			{
				ast *bt = transform(macros, b);
				tb_vector_insert_tail(bodies, bt);
			}
			ast *body = lift(block(bodies));

			return lift((ast){
			    .type = A_FN,
			    .fn =
				{
				    .name = name,
				    .args = args,
				    .type = type,
				    .body = body,
				},
			});
		} else if (has(items, O(0))) {
			return lift(operate(macros, items));
		}
	}

	return a;
}

void manage_ast(ast *a)
{
	printl_ast(a);
	tb_list_ref_t macros = tb_list_init(0, tb_element_ptr(0, 0));
	ast *b = transform(macros, a);
	tb_list_ref_t context = tb_list_init(0, ast_ref_element);
	compile(context, b);
	tb_list_clear(context);
	tb_list_exit(context);
	destroy_ast(b);
}

int main()
{
	tb_check_abort(tb_init(0, 0));

	ast_element = tb_element_mem(sizeof(ast), 0, 0);
	ast_ref_element = tb_element_ptr(0, 0);

	tb_stack_ref_t stack = tb_stack_init(128, ast_element);

	ast a;
	pcc_context_t *ctx = pcc_create(stack);
	while (pcc_parse(ctx, &a)) {
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
