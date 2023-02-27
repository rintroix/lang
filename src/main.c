#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "tbox/algorithm/find_if.h"
#include "tbox/container/vector.h"
#include "tbox/prefix/check.h"
#include "tbox/tbox.h"

#include "common.h"
#include "parser.h"
#include "string.h"

tb_element_t ast_element;
tb_element_t rule_element;
tb_element_t macro_element;
tb_element_t define_element;

ast *transform(tb_iterator_ref_t macros, ast *a);

#define get(x, y) ((ast *)tb_iterator_item(x, y))

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

ast *lift(ast a)
{
	ast *ptr = tb_malloc(sizeof(ast));
	*ptr = a;
	return ptr;
}

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

	if (pat->type != a->type)
		return 0;

	switch (pat->type) {
	case A_ID:
		if (pat->id.type != a->id.type)
			return 0;

		if (!pat->id.name)
			return 1;

		if (0 == strcmp(pat->id.name, a->id.name))
			return 1;

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

	case A_FN:
		if (!pat->fn.def.name)
			return 1;

		if (0 != strcmp(pat->fn.def.name, a->fn.def.name))
			return 0;

		if (pat->fn.args || pat->fn.def.init || pat->fn.def.type ||
		    pat->fn.args)
			bug("%s: extra fun", __func__);

		return 1;
		
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
		tb_for_all(ast *, item, a->list.items)
		{
			if (item != get(a->list.items, 0))
				printf(" ");
			print_ast(item);
		}
		printf(")");
	} break;

	case A_FN: {
		printf("FN %s", a->fn.def.name);
	} break;

	case A_CALL: {
		printf("%s(", a->call.name);
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
		tb_free(a->fn.def.name);
		if (a->fn.def.type)
			tb_free(a->fn.def.type);
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

	tb_free(a);
}

// void compile(tb_list_ref_t context, ast *a)
// {
// 	switch (a->type) {
// 	case A_FN: {
// 		printf("%s %s(", a->fn.type, a->fn.name);
// 		printf(") {\n");
// 		ast *body = a->fn.body;
// 		tb_assert(body->block.items);
// 		tb_for_all(ast *, item, body->block.items)
// 		{
// 			compile(context, item);
// 			puts(";");
// 		}
// 		printf("}");
// 		puts("");
// 	} break;

// 	case A_LIST: {
// 		bug("%s: bare list", __func__);
// 	} break;

// 	case A_CALL: {
// 		printf("%s(", a->call.name);
// 		tb_for_all(ast *, arg, a->call.args)
// 		{
// 			if (arg != tb_iterator_item(a->call.args, 0))
// 				printf(", ");
// 			compile(context, arg);
// 		}
// 		printf(")");
// 	} break;

// 	case A_REF:
// 		todo;
// 		break;

// 	case A_DEF:
// 		todo;
// 		break;

// 	case A_ID:
// 		bug("%s: id %s", __func__, a->id.name);
// 		break;

// 	case A_OPER:
// 		printf("(");
// 		printf(" ");
// 		compile(context, a->oper.left);
// 		printf(" ");
// 		printf("%s", a->oper.name);
// 		printf(" ");
// 		compile(context, a->oper.left);
// 		printf(" ");
// 		printf(")");
// 		break;

// 	case A_BLOCK:
// 		todo;
// 		break;

// 	case A_MARK:
// 		bug("%s: mark", __func__);
// 		break;
// 	}
// }

tb_iterator_ref_t funargs(tb_iterator_ref_t list)
{
	tb_vector_ref_t out =
	    tb_vector_init(tb_iterator_size(list), define_element);

	define *last = 0;
	tb_for_all(ast *, arg, list)
	{
		if (arg->type != A_ID)
			error("fun arg not an identifier");

		switch (arg->id.type) {
		case I_WORD: {
			tb_vector_insert_tail(out, &def(arg->id.name, 0, 0));
			last = tb_vector_last(out);
		} break;

		case I_KW:
			if (last->type)
				error("several keywords after fun arg");
			last->type = arg->id.name;
			break;

		default:
			error("unexpected fun arg type: %d", arg->type);
			break;
		}
	}

	return out;
}

ast* atom_or_list(tb_iterator_ref_t iter, tb_size_t start, tb_size_t end) {
	tb_assert(end >= start);
	switch (end - start) {
	case 0:
		bug("%s: empty", __func__);
		break;

	case 1:
		return get(iter, start);
		break;

	default:
		return lift(list(slice(iter, start, end)));
		break;
	}
}

ast* operate(tb_iterator_ref_t macros, tb_iterator_ref_t list, tb_size_t start)
{
	tb_size_t pos = tb_find_if(list, start, tb_iterator_tail(list), it_is, O(0));
	tb_size_t end = tb_iterator_tail(list);

	if (start == end)
		bug("%s: received empty", __func__);

	if (pos == end) {
		// TODO check empty list
		return transform(macros, atom_or_list(list, start, end));
	}

	if (pos == start) {
		bug("%s: operator without left side", __func__);
	}

	tb_assert(pos > start);

	ast *op = tb_iterator_item(list, pos);
	tb_assert(op->type == A_ID && op->id.type == I_OP);

	ast *left = transform(macros, atom_or_list(list, start, pos));
	return lift(oper(op->id.name, left, operate(macros, list, pos + 1)));
}

int _match(tb_vector_ref_t list, ast **patterns, size_t n)
{
	tb_for_all(ast *, a, list)
	{
		if (a_itor == n)
			return 1;

		if (!is(a, patterns[a_itor]))
			return 0;
	}

	return tb_iterator_size(list) == n;
}

#define LEN(A) (sizeof(A)/sizeof((A)[0]))
#define MA(...) ((ast *[]){__VA_ARGS__})
#define match(L, ...) _match(L, MA(__VA_ARGS__), LEN(MA(__VA_ARGS__)))

block transform_block(tb_iterator_ref_t macros, tb_iterator_ref_t iter,
		      tb_size_t start, tb_size_t end)
{
	tb_check_abort(end >= start);
	tb_vector_ref_t items = tb_vector_init(end - start, ast_element);
	tb_for(ast *, a, start, end, iter)
	{
		ast *b = transform(macros, a);
		tb_vector_insert_tail(items, b);
	}

	// TODO block needs defs?
	return (block){.items = items};
}

ast *transform(tb_iterator_ref_t macros, ast *a)
{
	// TODO compactor first

	if (a->type != A_LIST)
		return a;

	tb_iterator_ref_t items = a->list.items;
	if (match(items, W("fn"), W(0), K(0), L(0))) {
		puts("MATCHED");
		char *name = get(items, 1)->id.name;
		char *type = get(items, 2)->id.name;
		tb_iterator_ref_t args = funargs(get(items, 3)->list.items);
		ast *init = lift(ablock(transform_block(
		    macros, items, 4, tb_iterator_tail(items))));
		return lift(fn(def(name, type, init), args));
	} else if (has(items, O(0))) {
		return operate(macros, items, 0);
	}

	return a;
}

void manage_ast(ast *a)
{
	printl_ast(a);
	tb_vector_ref_t macros = tb_vector_init(10, tb_element_ptr(0, 0));
	ast *b = transform(macros, a);
	printl_ast(b);
 	// tb_list_ref_t rules = tb_vector_init(0, rule_element);


	
	// infer(context, b);
	// compile(context, b);
	// tb_list_clear(context);
	// tb_list_exit(context);
	// destroy_ast(b);
}

scope *newscope(tb_iterator_ref_t asts, scope *next)
{
	scope *out = tb_malloc(sizeof(scope));
	tb_vector_ref_t functions =
	    tb_vector_init(tb_iterator_size(asts), ast_element);
	tb_for_all(ast*, a, asts)
	{
		switch(a->type) {
		case A_FN:
			tb_vector_insert_tail(functions, a);
			break;
		default:
			bug("%s: unhandled %d", __func__, a->type);
			break;
		}
	}
	*out = (scope){.functions = functions, .next = next};
	return out;
}

tb_iterator_ref_t find_candidates(scope *scope, char *name,
				  tb_iterator_ref_t args)
{
	tb_for_all(ast *, a, scope->functions)
	{
		if (a->type != A_FN)
			bug("%s: not fun", __func__);

		if (0 != strcmp(name, a->fn.def.name))
			continue;

		// todo next
		// if (compatible(a->fn.j))		
	}

	return 0;
}

request *newreq(scope *scope, char *name, tb_iterator_ref_t args)
{
	request *out = tb_malloc(sizeof(request));
	tb_iterator_ref_t rules = tb_vector_init(20, rule_element);
	tb_iterator_ref_t candidates = find_candidates(scope, name, args);
	if (! candidates)
		error("no candidates");
	*out = (request){ .args = args, .rules = rules, .candidates = candidates };
	return out;
}

rule *newrule(enum e_rule type, char* name)
{
	rule *out = tb_malloc(sizeof(rule));
	switch (type) {
	case R_EMPTY:
		*out = (rule){.type = type};
		break;
	case R_IS:
		*out = (rule){.type = type, .is = { .name = name }};
		break;
	case R_CALL:
		todo;
		break;
	}
	return out;
}


// TODO delme
void infer(scope *s, ast *fn, request* r) {
	if (fn->type != A_FN)
		bug("%s: not fun", __func__);
}

int main()
{
	tb_check_abort(tb_init(0, 0));

	ast_element = tb_element_mem(sizeof(ast), 0, 0);
	rule_element = tb_element_mem(sizeof(rule), 0, 0);
	macro_element = tb_element_mem(sizeof(macro), 0, 0);
	define_element = tb_element_mem(sizeof(define), 0, 0);

	tb_vector_ref_t topast = tb_vector_init(128, ast_element);
	tb_vector_ref_t macros = tb_vector_init(128, ast_element);

	ast a;
	pcc_context_t *ctx = pcc_create(0);
	while (pcc_parse(ctx, &a)) {
		// TODO recognise macro
		tb_vector_insert_tail(topast, lift(a));
	}
	pcc_destroy(ctx);

	if (a.type != 0) 
		bug("%s: parser top level return", __func__);

	tb_for_all(ast *, item, topast) {
		tb_vector_replace(topast, item_itor, transform(macros, item));
	}

	// tb_size_t main_pos = tb_find_all_if(topast, it_is, F("main"));
	// if (main_pos == tb_iterator_tail(topast))
	// 	error("main not found");
	// ast *main = get(topast, main_pos);
	scope *top = newscope(topast, 0);
	tb_vector_ref_t main_rules = tb_vector_init(0, rule_element); // empty
	request *mainreq = newreq(top, "main", main_rules);

// typedef struct request {
// 	char *name;
// 	tb_iterator_ref_t rules;
// 	scope scope;
// } request;

	tb_trace_d("END");

	// tb_exit();
}

