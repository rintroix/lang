#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "data.h"
#include "parser.h"

ast *parse(parse_ctx *ctx, ast *a);

#define lift(...)                                                              \
	_Generic((__VA_ARGS__), ast                                            \
		 : alift, rule                                                 \
		 : rlift, type                                                 \
		 : tlift)(__VA_ARGS__)

ast *alift(ast a)
{
	ast *ptr = malloc(sizeof(ast));
	*ptr = a;
	return ptr;
}

rule *rlift(rule r)
{
	rule *ptr = malloc(sizeof(rule));
	*ptr = r;
	return ptr;
}

type *tlift(type t)
{
	type *ptr = malloc(sizeof(type));
	*ptr = t;
	return ptr;
}

rule def2rule(define *d) // delme
{
	type t = d->type;
	switch (t.tag) {
	case T_UNKNOWN: {
		todo;
	} break;
	case T_BUILTIN: {
		todo;
	} break;
	case T_COMPOUND: {
		todo;
	} break;
	}
}

int is(ast *a, ast *pat)
{
	if (!pat)
		return 1;

	if (pat->tag != a->tag)
		return 0;

	switch (pat->tag) {
	case A_ID:
		if (pat->id.tag != a->id.tag)
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

	default:
		bug("%s: unhandled %d", __func__, pat->tag);
	}
}

char *show_type(type *t) {
	switch(t->tag) {
	case T_BUILTIN: {
		return t->name;
	} break;
	case T_UNKNOWN: {
		return "UNKNOWN";
	} break;
	case T_COMPOUND: {
		todo;
	} break;
	}

	todo;
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
	switch (a->tag) {
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
		if (a->id.tag == I_KW)
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
		bug("unhandled %d", a->tag);
		break;
	}
}

char *show_ast(ast *a)
{
	vec(char) chars = avec(char);
	_show_ast(chars, a);
	char *out = malloc(vlen(chars) + 1);
	forv(chars, c, i) { out[i] = *c; }
	out[vlen(chars)] = '\0';
	return out;
}

void print_ast(ast *a) { printf("%s", show_ast(a)); }

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
	assert(l.tag == A_LIST);
	push(l.list.items, a);
	return l;
}

// void compile(tb_list_ref_t context, ast *a)
// {
// 	switch (a->tag) {
// 	case A_FN: {
// 		printf("%s %s(", a->fn.tag, a->fn.name);
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

type list2type(vec(ast) items, size_t start, size_t end)
{
	check(end > start);

	if (end - start == 1) {
		ast *head = vat(items, start);
		check(head->tag == A_ID && head->id.tag == I_WORD);
		return (type){.tag = T_UNKNOWN, .name = head->id.name};
	}

	todo; // compound and funs
}

vec(define) funargs(vec(ast) list)
{
	vec(define) out = avec(define);

	forv(list, it)
	{
		switch (it->tag) {

		case A_ID: {
			check(it->id.tag == I_WORD);
			type t = (type){.tag = T_UNKNOWN};
			push(out, (define){.name = it->id.name, .type = t});
		} break;

		case A_LIST: {
			vec(ast) items = it->list.items;
			check(vlen(items) > 1);
			ast *head = vat(items, 0);
			check(head->tag == A_ID && head->id.tag == I_WORD);
			push(out, (define){.name = head->id.name,
					   .type = list2type(items, 1,
							     vlen(items))});
		} break;

		default: {
			error("unexpected ast among args: %s", show_ast(it));
		} break;
		}

		// if (it->tag != A_ID)
		// 	error("fun arg not an identifier");

		// switch (it->id.tag) {
		// case I_WORD: {
		// 	push(out, def(it->id.name, 0, 0));
		// 	last = vat(out, vlen(out) - 1); // TODO last()
		// } break;

		// case I_KW:
		// 	if (last->type_name)
		// 		error("several keywords after fun arg");
		// 	last->type_name = it->id.name;
		// 	break;

		// default:
		// 	error("unexpected fun arg tag: %d", it->tag);
		// 	break;
		// }
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

ast *parse_operator(parse_ctx *ctx, vec(ast) list, size_t start)
{
	// TODO opreq, same as callreq
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
		return parse(ctx, atom_or_list(list, start, end));
	}

	if (pos == start) {
		bug("%s: operator without left side", __func__);
	}

	assert(pos > start);

	ast *op = vat(list, pos);
	assert(op->tag == A_ID && op->id.tag == I_OP);

	ast *left = parse(ctx, atom_or_list(list, start, pos));
	return lift(
	    aoper(op->id.name, left, parse_operator(ctx, list, pos + 1)));
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

block parse_block(parse_ctx *ctx, vec(ast) list, size_t start, size_t end)
{
	// TODO compactor first

	check(end >= start);

	vec(define) defs = avec(define);
	vec(ast) items = avec(ast);

	// TODO defs in ctx

	forvr(list, it, start, end)
	{
		ast *b = parse(ctx, it);
		push(items, *b);
	}

	return (block){.defines = defs, .items = items}; // TODO functions
}

int parse_top_one(parse_ctx *ctx, vec(ast) items)
{
	if (match(items, W("fn"), W(0), W(0), L(0))) {
		char *name = vat(items, 1)->id.name;
		type ret = list2type(items, 1, 2); // TODO hack
		vec(define) args = funargs(vat(items, 3)->list.items);
		parse_ctx local = (parse_ctx){.defines = args, .next = ctx};
		ast *init =
		    lift(ablock(parse_block(&local, items, 4, vlen(items))));
		define self = def(name, ret, init);

		push(ctx->functions, fn(self, args));
		return 1;
	}

	if (match(items, W("let"))) {
		todo;
		// if def, put to defs
		return 1;
	}

	return 0;
}

int types_compatible(type *a, type *b)
{
	log("compat %s vs %s", show_type(a), show_type(b));

	if (a->tag == T_UNKNOWN || b->tag == T_UNKNOWN)
		return 1;

	if (a->tag != b->tag)
		return 0;

	switch (a->tag) {
	case T_BUILTIN: {
		if (0 == strcmp(a->name, b->name))
			return 1;
	} break;
	case T_COMPOUND: {
		todo;
	} break;
	case T_UNKNOWN: {
		bug("unreachable");
	} break;
	}

	return 0;
}

int iscandidate(callreq *r, function *f) {
	if (vlen(r->args) != vlen(f->args))
		return 0;

	if (0 != strcmp(r->name, f->self.name))
		return 0;

	forv(r->args, a, i) {
		type *fat = &vat(f->args, i)->type;

		if (!types_compatible(*a, fat))
			return 0;
	}
	
	return 1;
}

vec(function*) find_candidates(parse_ctx *ctx, callreq *r) {
	vec(function*) out = avec(function*);
	for (; ctx; ctx = ctx->next) {
		if (!ctx->functions)
			continue;

		forv(ctx->functions, f) {
			if (iscandidate(r, f)) {
				push(out, f);
			}
		}
	}

	return out;
}

parse_ctx parse_top(parse_ctx *upper, vec(ast) items)
{
	vec(callreq) callreqs = avec(callreq);
	vec(function) functions = avec(function);
	vec(define) defines = avec(define);

	parse_ctx current = (parse_ctx){
	    .callreqs = callreqs, .functions = functions, .defines = defines};

	forv(items, it)
	{
		if (it->tag != A_LIST)
			error("not a list: %s", show_ast(it));

		if (!parse_top_one(&current, it->list.items))
			error("bad top level ast: %s", show_ast(it));
	}

	if (0 != vlen(callreqs)) {
		forv(callreqs, req)
		{
			vec(function*) candidates = find_candidates(&current, req);

			log("request %s, arity %zu, found %zu candidates", req->name,
			    vlen(req->args), vlen(candidates));
		}

		bug("unhandled call requests");
	}

	return current;
}

ast *parse_list(parse_ctx *ctx, vec(ast) items)
{
	if (has(items, O(0))) {
		return parse_operator(ctx, items, 0);
	}

	ast *head = vat(items, 0);
	assert(head->tag == A_ID && head->id.tag == I_WORD);
	char *name = head->id.name;

	vec(ast) args = avec(ast);
	forvr(items, item, 1, vlen(items)) {
		push(args, *parse(ctx, item));
	}

	ast c = acall(name, args);

	type *ret = lift((type){});
	c.type = ret;

	vec(type *) types = avec(type *);
	forv(args, arg) { push(types, arg->type); }

	for (; ctx; ctx = ctx->next) {
		if (!ctx->callreqs)
			continue;

		push(ctx->callreqs,
		     (callreq){.name = name, .ret = ret, .args = types});

		return lift(c);
	}

	bug("no callreqs found");
}

ast *find_ref(parse_ctx *ctx, char *name)
{
	for (; ctx; ctx = ctx->next) {
		forv(ctx->defines, d)
		{
			if (0 == strcmp(name, d->name)) {
				return lift((ast){.tag = A_REF,
						  .type = &d->type,
						  .ref = {.def = d}});
			}
		}
	}
	error("ref not found: %s", name);
}

ast *parse(parse_ctx *ctx, ast *a)
{
	switch (a->tag) {
	case A_REF: {
		bug("ref");
	} break;
	case A_BLOCK: {
		bug("block");
	} break;
	case A_OPER: {
		bug("oper");
	} break;
	case A_CALL: {
		bug("call");
	} break;
	case A_MARK: {
		bug("mark");
	} break;
	case A_LIST: {
		return parse_list(ctx, a->list.items);
	} break;
	case A_ID: {
		switch (a->id.tag) {
		case I_WORD: {
			log("looking for %s", a->id.name);
			return find_ref(ctx, a->id.name);
		} break;
		case I_INT: {
			a->type = lift((type){.tag=T_BUILTIN, .name="int"});
			return a;
		} break;
		case I_KW: {
			todo;
		} break;
		case I_OP: {
			todo;
		} break;
		}
	} break;
	}
}

rule type2rule(scope *s, type t) // delme
{
	bug_if_not(s);
	bug_if_not(s->types);
	todo;
}

scope *newscope(block b, scope *next)
{
	scope *out = malloc(sizeof(scope));
	vec(let) lets = avec(let);
	forv(b.defines, d)
	{
		rule r = type2rule(next, d->type);
		push(lets, (let){.name = d->name, .rule = r});
	}
	*out = (scope){.functions = b.functions, .lets = lets, .next = next};
	return out;
}

int satisfies(rule *a, define *d)
{
	rule b = def2rule(d);

	todo;
	// // TODO type match
	// switch (r->tag) {
	// case R_EMPTY:
	// 	return 1;
	// 	break;

	// case R_IS:
	// 	if (d->type.tag == T_UNKNOWN)
	// 		return 1;
	// 	bug_if_not(r->tag);
	// 	return 0 == strcmp(r->is-name,
	// 			   d->type.name); // TODO might not have name
	// 	break;

	// case R_REQ:
	// 	todo;
	// 	break;
	// }
}

int compatible(vec(define) args, vec(rule) rules)
{
	if (vlen(args) != vlen(rules)) {
		log("len mismatch %zu vs %zu", vlen(args), vlen(rules)); // delme
		return 0;
	}

	forv(args, arg, i)
	{
		if (!satisfies(vat(rules, i), arg))
			return 0;
	}

	return 1;
}

// request *newreq(scope *scope, char *name, vec(rule) args) // delme
// {
// 	vec(candidate) candidates = find_candidates(scope, name, args);
// 	if (!vlen(candidates))
// 		error("no candidates for '%s'", name);
// 	request *out = malloc(sizeof(request));
// 	*out =
// 	    (request){.scope = scope, .args = args, .candidates = candidates};
// 	return out;
// }

type *find_type(scope *s, type x)
{
	bug_if_not(s);

	for (; s; s = s->next) {
		if (!s->types)
			continue;
		log("LEN %ld", vlen(s->types));
		forv(s->types, t)
		{
			if (t->tag == x.tag && 0 == strcmp(t->name, x.name))
				return t;
		}
	}

	error("type not found: %s", show_type(&x));
}

rule *find_let(scope *s, char *name)
{
	bug_if_not(s);

	for (; s; s = s->next) {
		log("LEN %ld", vlen(s->lets));
		forv(s->lets, l)
		{
			log("%s vs %s", name, l->name);
			if (0 == strcmp(name, l->name)) {
				log("found let: %s", name);
				return &l->rule;
			}
		}
	}

	error("let not found: %s", name);
}

// rule *rulify(scope *s, ast *a)
// {
// 	switch (a->tag) {
// 	case A_LIST: {
// 		bug("list");
// 	} break;

// 	case A_CALL: {
// 		vec(rule) arg_rules = avec(rule);
// 		forv(a->call.args, arg)
// 		{
// 			rule *r = rulify(s, arg);
// 			if (!r)
// 				return 0;
// 			push(arg_rules, *r);
// 		}
// 		request *req = newreq(s, a->call.name, arg_rules);
// 		if (!req)
// 			return 0;
// 		return lift((rule){.tag = R_REQ, .req = *req});
// 	} break;

// 	case A_ID: {
// 		switch (a->id.tag) {
// 		case I_WORD: {
// 			log("id %s", a->id.name);
// 			return find_let(s, a->id.name);
// 		} break;
// 		case I_INT: {
// 			type *t = find_type(
// 			    s, (type){.tag = T_BUILTIN, .name = "int"});
// 			return lift((rule){.tag = R_IS, .is = t});
// 		} break;
// 		case I_KW: {
// 			todo;
// 		} break;
// 		case I_OP: {
// 			todo;
// 		} break;
// 		}
// 		todo;
// 	} break;

// 	case A_OPER: {
// 		todo;
// 	} break;

// 	case A_BLOCK: {
// 		log("block");
// 		s = newscope(a->block, s);
// 		forv(a->block.items, item) { rulify(s, item); }
// 		todo;
// 	} break;

// 	case A_MARK: {
// 		todo;
// 	} break;
// 	}
// }

scope *core_scope()
{
	scope *out = malloc(sizeof(scope));
	vec(type) types = avec(type);
	push(types, (type){.tag = T_BUILTIN, .name = "int"});
	push(types, (type){.tag = T_BUILTIN, .name = "float"});
	push(types, (type){.tag = T_BUILTIN, .name = "char"});
	*out = (scope){.types = types};
	return out;
}

int main(int argc, char **argv)
{
	vec(ast) tops = avec(ast);
	vec(macro) macros = avec(macro);

	check(argc == 2);

	FILE *f = fopen(argv[1], "r");
	check(f);

	ast a;
	pcc_context_t *ctx = pcc_create(f);
	while (pcc_parse(ctx, &a)) {
		push(tops, a);
	}
	pcc_destroy(ctx);

	bug_if(a.tag != 0);

	parse_ctx upper =
	    (parse_ctx){.defines = avec(define), .macros = avec(macro)};

	parse_ctx topctx = parse_top(&upper, tops);
	// TODO manually create main request

	// scope *top = newscope(btop, core_scope());
	// vec(rule) main_rules = avec(rule);
	// request *r = newreq(top, "main", main_rules);

	log("END");
}
