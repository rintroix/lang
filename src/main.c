#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "data.h"
#include "parser.h"

ast *transform(vec(macro) macros, ast *a);

ast *lift(ast a)
{
	ast *ptr = malloc(sizeof(ast));
	*ptr = a;
	return ptr;
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

int has(vec(ast) list, ast *pattern)
{
	vfor(list, it)
	{
		if (is(it, pattern))
			return 1;
	}
	return 0;
}

void print_ast(ast *a)
{
	switch (a->type) {
	case A_LIST: {
		printf("(");
		vfori(a->list.items, it, i) {
			if (i != 0)
				printf(" ");
			printl_ast(it);
		}
		printf(")");
	} break;

	case A_FN: {
		printf("FN %s", a->fn.def.name);
	} break;

	case A_CALL: {
		printf("%s(", a->call.name);
		vfori(a->call.args, arg, i)
		{
			if (i != 0)
				printf(" ");
			print_ast(arg);
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
}

ast list0()
{
	vec(ast) items = avec(items);
	return list(items);
}

ast list1(ast a)
{
	vec(ast) items = avec(items);
	push(items, a);
	return list(items);
}

ast append(ast l, ast a)
{
	assert(l.type == A_LIST);
	push(l.list.items, a);
	return l;
}

void destroy_ast(ast *a)
{
	todo;
	// switch (a->type) {
	// case A_FN:
	// 	tb_free(a->fn.def.name);
	// 	if (a->fn.def.type)
	// 		tb_free(a->fn.def.type);
	// 	// if (a->fn.args) // TODO
	// 	break;

	// case A_LIST: {
	// 	tb_for_all(ast *, item, a->call.args) { destroy_ast(item); }
	// 	tb_stack_clear(a->call.args);
	// 	tb_stack_exit(a->call.args);
	// } break;

	// case A_CALL: {
	// 	if (a->call.name)
	// 		tb_free(a->call.name); // TODO hack, need empty list ast
	// 	tb_for_all(ast *, item, a->call.args) { destroy_ast(item); }
	// 	tb_stack_clear(a->call.args);
	// 	tb_stack_exit(a->call.args);
	// } break;

	// case A_ID:
	// 	tb_free(a->id.name);
	// 	break;

	// case A_OPER:
	// 	tb_free(a->oper.name);
	// 	if (a->oper.left)
	// 		destroy_ast(a->oper.left);
	// 	if (a->oper.right)
	// 		destroy_ast(a->oper.right);
	// 	break;

	// case A_MARK:
	// 	bug("mark");
	// 	break;

	// case A_BLOCK:
	// 	todo;
	// 	break;
	// }

	// tb_free(a);
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

vec(define) funargs(vec(ast) list)
{
	vec(define) out = avec(out);

	define *last = 0;
	vfor(list, it)
	{
		if (it->type != A_ID)
			error("fun arg not an identifier");

		switch (it->id.type) {
		case I_WORD: {
			push(out, def(it->id.name, 0, 0));
			last = vat(out, vlen(out) - 1); // TODO last()
		} break;

		case I_KW:
			if (last->type)
				error("several keywords after fun arg");
			last->type = it->id.name;
			break;

		default:
			error("unexpected fun arg type: %d", it->type);
			break;
		}
	}

	return out;
}

ast* atom_or_list(vec(ast) iter, size_t start, size_t end) {
	assert(end >= start);
	switch (end - start) {
	case 0:
		bug("%s: empty", __func__);
		break;

	case 1:
		return vat(iter, start);
		break;

	default:
		return lift(list(vslice(iter, start, end)));
		break;
	}
}

ast* operate(vec(macro) macros, vec(ast) list, size_t start)
{
	// TODO better find
	size_t pos = vlen(list);
	vforri(list, it, start, vlen(list), index) {
		if (is(it, O(0))) {
			pos = index;
			break;
		}
	}

	size_t end = vlen(list);

	if (start == end)
		bug("%s: received empty", __func__);

	if (pos == end) {
		// TODO check empty list
		return transform(macros, atom_or_list(list, start, end));
	}

	if (pos == start) {
		bug("%s: operator without left side", __func__);
	}

	assert(pos > start);

	ast *op = vat(list, pos);
	assert(op->type == A_ID && op->id.type == I_OP);

	ast *left = transform(macros, atom_or_list(list, start, pos));
	return lift(oper(op->id.name, left, operate(macros, list, pos + 1)));
}

int _match(vec(ast) list, ast **patterns, size_t n)
{
	vfori(list, it, index)
	{
		if (index == n)
			return 1;

		if (!is(it, patterns[index]))
			return 0;
	}

	return vlen(list) == n;
}

#define LEN(A) (sizeof(A)/sizeof((A)[0]))
#define MA(...) ((ast *[]){__VA_ARGS__})
#define match(L, ...) _match(L, MA(__VA_ARGS__), LEN(MA(__VA_ARGS__)))

block transform_block(vec(macro) macros, vec(ast) iter, size_t start,
		      size_t end)
{
	check(end >= start);

	vec(ast) items = avec(items);

	vfor(iter, it)
	{
		ast *b = transform(macros, it);
		push(items, *b);
	}

	// TODO block needs defs?
	return (struct block){.defs = 0, .items = items};
}

ast *transform(vec(macro) macros, ast *a)
{
	// TODO compactor first

	if (a->type != A_LIST)
		return a;

	vec(ast) items = a->list.items;
	if (match(items, W("fn"), W(0), K(0), L(0))) {
		char *name = vat(items, 1)->id.name;
		char *type = vat(items, 2)->id.name;
		vec(define) args = funargs(vat(items, 3)->list.items);
		ast *init = lift(ablock(transform_block(
		    macros, items, 4, vlen(items))));
		return lift(fn(def(name, type, init), args));
	} else if (has(items, O(0))) {
		return operate(macros, items, 0);
	}

	return a;
}

scope *newscope1(ast *f, scope *next)
{
	assert(f->type == A_FN);
	scope *out = malloc(sizeof(scope));
	vec(ast) functions = avec(functions);
	push(functions, *f);
	*out = (scope){.functions = functions, .next = next};
	return out;
}

scope *newscope(vec(ast) asts, scope *next)
{
	scope *out = malloc(sizeof(scope));
	vec(ast) functions = avec(functions);

	vfor(asts, it)
	{
		switch(it->type) {
		case A_FN:
			push(functions, *it);
			break;
		default:
			bug("%s: unhandled %d", __func__, it->type);
			break;
		}
	}
	*out = (scope){.functions = functions, .next = next};
	return out;
}

int satisfies(rule *r, define *d) {
	switch(r->type) {
	case R_EMPTY:
		return 1;
		break;

	case R_IS:
		if (! d->type)
			return 1;
		if (! r->type)
			bug("%s: is no type", __func__);
		return 0 == strcmp(r->is.name, d->type);		
		break;

	case R_CALL:
		todo;
		break;
	}
}

int compatible(vec(define) args, vec(rule) rules) {
	if (vlen(args) != vlen(rules))
		return 0;

	vfori(args, arg, i) {
		if (!satisfies(vat(rules, i), arg))
			return 0;
	}

	return 1;
}

void rulify(scope *s, ast *a, vec(rule) rules) {
	switch (a->type) {
	case A_LIST: {
		todo;
	} break;

	case A_FN: {
		define d = a->fn.def;
		assert(d.init);
		scope *fs = newscope1(a, s);
		rulify(fs, d.init, rules);
		todo;
		// open new scope
		// add self to scope
		// add args to rules

	} break;

	case A_CALL: {
		todo;
	} break;

	case A_ID:
		todo;
		break;

	case A_OPER:
		todo;
		break;

	case A_BLOCK:
		todo;
		break;

	case A_MARK:
		todo;
		break;
	}
}

int infer(scope *s, candidate* c) {
	assert(c->ast->type == A_FN);
	rulify(s, c->ast, c->rules);

	return 1; // todo
}

vec(candidate)
    find_candidates(scope *scope, char *name, vec(rule) arg_rules)
{
	vec(candidate) candidates = avec(candidates);

	vfor(scope->functions, it)
	{
		if (it->type != A_FN)
			bug("%s: not fun", __func__);

		if (0 != strcmp(name, it->fn.def.name))
			continue;

		if (compatible(it->fn.args, arg_rules)) {
			vec(rule) rules = avec(rules);

			candidate c = can(it, rules);
			if (infer(scope, &c))
				push(candidates, c);
		}		
	}

	return candidates;
}

request *newreq(scope *scope, char *name, vec(rule) args)
{
	request *out = malloc(sizeof(request));
	vec(candidate) candidates = find_candidates(scope, name, args);
	if (!vlen(candidates))
		error("no candidates for '%s'", name);
	*out = (request){.scope = scope,
			 .args = args,
			 .candidates = candidates};
	return out;
}

rule *newrule(enum e_rule type, char* name)
{
	rule *out = malloc(sizeof(rule));
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

void compile(request *r) {
	todo;
}

int main()
{
	vec(ast) topast = avec(topast);
	vec(macro) macros = avec(macros);

	ast a;
	pcc_context_t *ctx = pcc_create(0);
	while (pcc_parse(ctx, &a)) {
		// TODO recognise macro
		push(topast, a);
	}
	pcc_destroy(ctx);

	if (a.type != 0) 
		bug("%s: parser top level return", __func__);

	vfor(topast, it) {
		*it = *transform(macros, it);
	}


	scope *top = newscope(topast, 0);
	vec(rule) main_rules = avec(main_rules);
	request *mainreq = newreq(top, "main", main_rules);
	compile(mainreq);

	log("END");
}


