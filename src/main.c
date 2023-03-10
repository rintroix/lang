#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "data.h"
#include "parser.h"

ast *transform(vec(macro) macros, ast *a);
rule rulify(scope *s, ast *a);

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

		// case A_FN: // todo delme
		// 	if (!pat->fn.def.name)
		// 		return 1;

		// 	if (0 != strcmp(pat->fn.def.name, a->fn.def.name))
		// 		return 0;

		// 	if (pat->fn.args || pat->fn.def.init || pat->fn.def.type
		// || 	    pat->fn.args) 		bug("%s: extra fun", __func__);

		// 	return 1;

		// 	break;

	default:
		bug("%s: unhandled %d", __func__, pat->type);
	}
}

int has(vec(ast) list, ast *pattern)
{
	forv(list, it)
	{
		if (is(it, pattern))
			return 1;
	}
	return 0;
}

void cadd(vec(char) out, char *s)
{
	while (*s) {
		push(out, *s);
		s++;
	}
}

void _show_ast(vec(char) out, ast *a)
{
	switch (a->type) {
	case A_LIST: {
		cadd(out, "(");
		forv(a->list.items, it, i)
		{
			if (i != 0)
				cadd(out, " ");
			_show_ast(out, it);
		}
		cadd(out, ")");
	} break;

	case A_CALL: {
		cadd(out, a->call.name);
		cadd(out, "(");
		forv(a->call.args, arg, i)
		{
			if (i != 0)
				cadd(out, " ");
			_show_ast(out, arg);
		}
		cadd(out, ")");
	} break;

	case A_ID:
		if (a->id.type == I_KW)
			cadd(out, ":");
		cadd(out, a->id.name);
		break;

	case A_OPER:
		cadd(out, "(");
		_show_ast(out, a->oper.left);
		cadd(out, " ");
		cadd(out, a->oper.name);
		cadd(out, " ");
		_show_ast(out, a->oper.right);
		cadd(out, ")");
		break;

	default:
		bug("unhandled %d", a->type);
		break;
	}
}

char *show_ast(ast *a) {
	vec(char) chars = avec(char);
	_show_ast(chars, a);
	char *out = malloc(vlen(chars) + 1);
	forv(chars, c, i) { out[i] = *c; }
	out[vlen(chars)] = '\0';
	return out;
}

void print_ast(ast *a)
{
	printf("%s", show_ast(a));
}

void printl_ast(ast *a)
{
	print_ast(a);
	printf("\n");
}

ast alist0()
{
	vec(ast) items = avec(ast);
	return alist(items);
}

ast alist1(ast a)
{
	vec(ast) items = avec(ast);
	push(items, a);
	return alist(items);
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
	vec(define) out = avec(define);

	define *last = 0;
	forv(list, it)
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

ast *atom_or_list(vec(ast) iter, size_t start, size_t end)
{
	assert(end >= start);
	switch (end - start) {
	case 0:
		bug("%s: empty", __func__);
		break;

	case 1:
		return vat(iter, start);
		break;

	default:
		return lift(alist(vslice(iter, start, end)));
		break;
	}
}

ast *operate(vec(macro) macros, vec(ast) list, size_t start)
{
	// TODO better find
	size_t pos = vlen(list);
	forvr(list, it, start, vlen(list), index)
	{
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
	return lift(aoper(op->id.name, left, operate(macros, list, pos + 1)));
}

int _match(vec(ast) list, ast **patterns, size_t n)
{
	forv(list, it, index)
	{
		if (index == n)
			return 1;

		if (!is(it, patterns[index]))
			return 0;
	}

	return vlen(list) == n;
}

#define LEN(A) (sizeof(A) / sizeof((A)[0]))
#define MA(...) ((ast *[]){__VA_ARGS__})
#define match(L, ...) _match(L, MA(__VA_ARGS__), LEN(MA(__VA_ARGS__)))

block transform_block(vec(macro) macros, vec(ast) list, size_t start,
		      size_t end)
{
	check(end >= start);

	vec(define) defs = avec(define);
	vec(ast) items = avec(ast);

	forvr(list, it, start, end)
	{
		ast *b = transform(macros, it);
		push(items, *b);
	}

	return (block){.defs = defs, .items = items};
}

block transform_where(vec(macro) macros, vec(ast) items, size_t start,
		      size_t end)
{
	check(end >= start);

	vec(function) funs = avec(function);
	vec(define) defs = avec(define);

	forvr(items, it, start, end)
	{
		if (it->type != A_LIST)
			error("no a list: %s", show_ast(it));

		vec(ast) list = it->list.items;

		if (match(list, W("fn"), W(0), K(0), L(0))) {
			char *name = vat(list, 1)->id.name;
			char *type = vat(list, 2)->id.name;
			vec(define) args = funargs(vat(list, 3)->list.items);
			ast *init = lift(ablock(
			    transform_block(macros, list, 4, vlen(list))));
			define self = def(name, type, init);

			push(funs, fn(self, args));
			continue;
		}

		if (match(list, W("let"))) {
			todo;
			// if def, put to defs
			continue;
		}

		error("wrong where item"); // todo better
	}

	return (block){.functions = funs, .defs = defs, .items = avec(ast)};
}

ast *transform(vec(macro) macros, ast *a)
{
	// TODO compactor first

	if (a->type != A_LIST)
		return a;

	vec(ast) items = a->list.items;

	if (has(items, O(0))) {
		return operate(macros, items, 0);
	}

	// else it is a call
	ast *head = vat(items, 0);
	assert(head->type == A_ID && head->id.type == I_WORD);

	vec(ast) args = vslice(items, 1, vlen(items));

	return lift(acall(head->id.name, args));
}

scope *newscope(vec(function) funs, vec(define) defs, scope *next)
{
	scope *out = malloc(sizeof(scope));
	*out = (scope){.functions = funs ? funs : avec(function),
		       .defs = defs ? defs : avec(define),
		       .next = next};
	return out;
}

int satisfies(rule *r, define *d)
{
	switch (r->type) {
	case R_EMPTY:
		return 1;
		break;

	case R_IS:
		if (!d->type)
			return 1;
		if (!r->type)
			bug("%s: is no type", __func__);
		return 0 == strcmp(r->is.name, d->type);
		break;

	case R_REQ:
		todo;
		break;
	}
}

int compatible(vec(define) args, vec(rule) rules)
{
	if (vlen(args) != vlen(rules)) {
		log("len mismatch %zu vs %zu", vlen(args), vlen(rules));
		return 0;
	}

	forv(args, arg, i)
	{
		if (!satisfies(vat(rules, i), arg))
			return 0;
	}

	return 1;
}

vec(candidate) find_candidates(scope *scope, char *name, vec(rule) arg_rules)
{
	vec(candidate) candidates = avec(candidate);

	while (scope) {
		log("find for %s among %zu", name, vlen(scope->functions));
		forv(scope->functions, it)
		{
			log("%s vs %s", name, it->def.name);

			if (0 != strcmp(name, it->def.name))
				continue;

			log("name good");

			if (compatible(it->args, arg_rules)) {
				log("compatible");
				vec(rule) rules = avec(rule);

				// TODO next
				// candidate c = can(it, rules);
				// if (infer(scope, &c))
				// 	push(candidates, c);
			}
		}
		scope = scope->next;
	}

	return candidates;
}

request *newreq(scope *scope, char *name, vec(rule) args)
{
	request *out = malloc(sizeof(request));
	vec(candidate) candidates = find_candidates(scope, name, args);
	if (!vlen(candidates))
		error("no candidates for '%s'", name);
	*out =
	    (request){.scope = scope, .args = args, .candidates = candidates};
	return out;
}

rule *newrule(enum e_rule type, char *name)
{
	rule *out = malloc(sizeof(rule));
	switch (type) {
	case R_EMPTY:
		*out = (rule){.type = type};
		break;
	case R_IS:
		*out = (rule){.type = type, .is = {.name = name}};
		break;
	case R_REQ:
		todo;
		break;
	}
	return out;
}

define *find_def(scope *s, char *name)
{
	while (s) {
		log("LEN %ld", vlen(s->defs));
		forv(s->defs, d)
		{
			log("%s vs %s", name, d->name);
			if (0 == strcmp(name, d->name)) {
				log("found def: %s", name);
				return d;
			}
		}
		s = s->next;
	}
	error("def not found: %s", name);
}

rule rulify(scope *s, ast *a)
{
	switch (a->type) {
	case A_LIST: {
		bug("list");
	} break;

		// case A_FN: {
		// 	// uODO field in scope for opening function
		// 	s = newscope(0, a->fn.args, s);
		// 	rulify(s, a->fn.def.init, rules);
		// } break;

	case A_CALL: {
		vec(rule) arg_rules = avec(rule);
		// TODO map
		forv(a->call.args, arg) { push(arg_rules, rulify(s, arg)); }
		return (rule){.type = R_REQ,
			      .req = *newreq(s, a->call.name, arg_rules)};
	} break;

	case A_ID:
		log("id %s", a->id.name);
		assert(a->id.type == I_WORD);
		find_def(s, a->id.name);
		todo;
		break;

	case A_OPER:
		todo;
		break;

	case A_BLOCK:
		log("block");
		s = newscope(0, a->block.defs, s);
		forv(a->block.items, item) { rulify(s, item); }
		todo;
		break;

	case A_MARK:
		todo;
		break;
	}
}

void compile(request *r) { todo; }

int main()
{
	vec(ast) tops = avec(ast);
	vec(macro) macros = avec(macro);

	ast a;
	pcc_context_t *ctx = pcc_create(0);
	while (pcc_parse(ctx, &a)) {
		push(tops, a);
	}
	pcc_destroy(ctx);

	if (a.type != 0)
		bug("parser top level return");

	forv(tops, it) { *it = *transform(macros, it); }

	block btop = transform_where(macros, tops, 0, vlen(tops));

	// scope *top = newscope(tops, 0, 0);
	// vec(rule) main_rules = avec(rule);
	// request *mainreq = newreq(top, "main", main_rules);
	// compile(mainreq);

	log("END");
}
