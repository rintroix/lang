#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "data.h"
#include "parser.h"

ast *parse(parse_ctx *ctx, typetable *table, ast *a);

#define lift(...)                                                              \
	_Generic((__VA_ARGS__), ast : alift, type : tlift)(__VA_ARGS__)

#define trace(...)                                                             \
	_Generic((__VA_ARGS__), ast * : aptrace, type : ttrace)(__VA_ARGS__)

size_t add_type(typetable *table, type t)
{
	size_t index = vlen(table->body);
	push(table->body, t);
	return index;
}

size_t add_arg_type(typetable *table, type t)
{
	size_t index = vlen(table->args);
	push(table->args, t);
	return index;
}

size_t add_ret_type(typetable *table, type t)
{
	table->ret = t;
	return 0;
}

size_t add_opreq(typetable *table, opreq o)
{
	size_t index = vlen(table->ops);
	push(table->ops, o);
	return index;
}

size_t add_callreq(typetable *table, callreq c)
{
	size_t index = vlen(table->calls);
	push(table->calls, c);
	return index;
}

type get_ret_type(typetable *table) { return table->ret; }

type get_arg_type(typetable *table, size_t index)
{
	return vget(table->args, index);
}

type get_type(typetable *table, size_t index)
{
	return vget(table->body, index);
}

typetable new_table()
{
	return (typetable){
	    .args = avec(type),
	    .body = avec(type),
	    .calls = avec(callreq),
	    .ops = avec(opreq),
	};
}

typetable clone_table(typetable table)
{
	return (typetable){
	    .ret = table.ret,
	    .args = vslice(table.args, 0, vlen(table.args)),
	    .body = vslice(table.body, 0, vlen(table.body)),
	    .calls = vslice(table.calls, 0, vlen(table.calls)),
	    .ops = vslice(table.ops, 0, vlen(table.ops)),
	};
}

ast *alift(ast a)
{
	ast *ptr = malloc(sizeof(ast));
	*ptr = a;
	return ptr;
}

// rule *rlift(rule r)
// {
// 	rule *ptr = malloc(sizeof(rule));
// 	*ptr = r;
// 	return ptr;
// }

type *tlift(type t)
{
	type *ptr = malloc(sizeof(type));
	*ptr = t;
	return ptr;
}

int is(ast *a, ast *pat)
{
	if (!pat)
		return 1;

	if (pat->tag != a->tag)
		return 0;

	switch (pat->tag) {
	case A_OPER: {
		todo;
	} break;
	case A_REF: {
		todo;
	} break;
	case A_MARK: {
		todo;
	} break;
	case A_BLOCK: {
		todo;
	} break;
	case A_ID: {
		if (pat->id.tag != a->id.tag)
			return 0;

		if (!pat->id.name)
			return 1;

		if (0 == strcmp(pat->id.name, a->id.name))
			return 1;

		return 0;
	} break;

	case A_LIST: {
		if (!pat->list.items)
			return 1;
		todo;
	} break;

	case A_CALL: {
		todo;
	} break;
	}

	bug("unhandled %d", pat->tag);
}

char *show_type(type t)
{
	switch (t.tag) {
	case T_SIMPLE: {
		return t.name;
	} break;
	case T_UNKNOWN: {
		return "UNKNOWN";
	} break;
	case T_COMPOUND: {
		todo;
	} break;
	}

	bug("wrong type tag: %d", t.tag);
}

type ttrace(type t) { todo; }

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
	case A_MARK: {
		bug("mark");
	} break;
	case A_BLOCK: {
		todo;
	} break;
	case A_REF: {
		cadd(out, "<");
		cadd(out, a->ref.def->name);
		cadd(out, ">");
	} break;
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

ast *aptrace(ast *a)
{
	log("AST %s", show_ast(a));
	return a;
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
		return (type){.tag = T_SIMPLE, .name = head->id.name};
	}

	todo; // compound and funs
}

vec(define) funargs(typetable *tt, vec(ast) list)
{
	vec(define) out = avec(define);

	forv(list, it)
	{
		switch (it->tag) {
		case A_ID: {
			check(it->id.tag == I_WORD);
			size_t index =
			    add_arg_type(tt, (type){.tag = T_UNKNOWN});
			push(out,
			     (define){.name = it->id.name, .index = index});
		} break;

		case A_LIST: {
			vec(ast) items = it->list.items;
			check(vlen(items) > 1);
			ast *head = vat(items, 0);
			check(head->tag == A_ID && head->id.tag == I_WORD);
			size_t index =
			    add_arg_type(tt, list2type(items, 1, vlen(items)));
			push(out,
			     (define){.name = head->id.name, .index = index});
		} break;

		default: {
			error("unexpected arg: %s", show_ast(it));
		} break;
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

ast *parse_operator(parse_ctx *ctx, typetable *table, vec(ast) list,
		    size_t start)
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
		bug("received empty");

	if (pos == end) {
		// TODO check empty list
		return parse(ctx, table, atom_or_list(list, start, end));
	}

	if (pos == start) {
		bug("%s: operator without left side", __func__);
	}

	assert(pos > start);

	ast *op = vat(list, pos);
	assert(op->tag == A_ID && op->id.tag == I_OP);

	ast *left = parse(ctx, table, atom_or_list(list, start, pos));
	ast *right = parse_operator(ctx, table, list, pos + 1);
	ast out = aoper(op->id.name, left, right);
	out.index = add_type(table, (type){.tag = T_UNKNOWN});
	out.oper.index = add_opreq(table, (opreq){.name = op->id.name,
						  .ret = out.index,
						  .left = left->index,
						  .right = right->index});
	return lift(out);
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

block parse_block(parse_ctx *ctx, typetable *table, vec(ast) list, size_t start,
		  size_t end)
{
	// TODO compactor first

	check(end >= start);

	vec(define) defs = avec(define);
	vec(ast) items = avec(ast);

	// TODO defs in ctx

	forvr(list, it, start, end)
	{
		ast *b = parse(ctx, table, it);
		push(items, *b);
	}

	return (block){.defines = defs, .items = items}; // TODO functions
}

int parse_top_one(parse_ctx *ctx, vec(ast) items)
{
	if (match(items, W("fn"), W(0), W(0), L(0))) {
		char *name = vat(items, 1)->id.name;
		typetable table = new_table();

		size_t retindex = add_ret_type(
		    &table, list2type(items, 2, 3)); // TODO extraction hack

		// TODO funargs require context as well, inits might need it
		vec(define) args = funargs(&table, vat(items, 3)->list.items);
		parse_ctx local = (parse_ctx){.defines = args, .next = ctx};
		ast *init = lift(
		    ablock(parse_block(&local, &table, items, 4, vlen(items))));
		define self = def(name, retindex, init);

		push(ctx->functions, fn(self, args, table));
		return 1;
	}

	if (match(items, W("let"))) {
		todo;
		// if def, put to defs
		return 1;
	}

	return 0;
}

int types_compatible(type a, type b)
{
	dbg("compat %s vs %s", show_type(a), show_type(b));

	if (a.tag == T_UNKNOWN || b.tag == T_UNKNOWN)
		return 1;

	if (a.tag != b.tag)
		return 0;

	switch (a.tag) {
	case T_SIMPLE: {
		return 0 == strcmp(a.name, b.name);
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

int table_compatible(typetable a, typetable b)
{
	if (vlen(a.args) != vlen(b.args))
		return 0;

	if (!types_compatible(a.ret, b.ret))
		return 0;

	forv(a.args, atp, i)
	{
		if (!types_compatible(*atp, vget(b.args, i)))
			return 0;
	}

	return 1;
}

vec(function *) find_candidates(parse_ctx *ctx, typetable *table, char *name)
{
	vec(function *) out = avec(function *);

	for (; ctx; ctx = ctx->next) {
		if (!ctx->functions)
			continue;

		forv(ctx->functions, f)
		{
			if (0 != strcmp(name, f->self.name))
				continue;

			if (table_compatible(*table, f->table)) {
				push(out, f);
			}
		}
	}

	return out;
}

parse_ctx parse_top(parse_ctx *upper, vec(ast) items)
{
	vec(function) functions = avec(function);
	vec(define) defines = avec(define);

	parse_ctx current = (parse_ctx){
	    .functions = functions, .defines = defines, .next = upper};

	forv(items, it)
	{
		if (it->tag != A_LIST)
			error("not a list: %s", show_ast(it));

		if (!parse_top_one(&current, it->list.items))
			error("bad top ast: %s", show_ast(it));
	}

	return current;
}

ast *parse_list(parse_ctx *ctx, typetable *table, vec(ast) items)
{
	if (has(items, O(0))) {
		return parse_operator(ctx, table, items, 0);
	}

	ast *head = vat(items, 0);
	assert(head->tag == A_ID && head->id.tag == I_WORD);
	char *name = head->id.name;

	vec(ast) args = avec(ast);
	vec(size_t) arg_indices = avec(size_t);
	forvr(items, item, 1, vlen(items))
	{
		ast arg = *parse(ctx, table, item);
		push(args, arg);
		push(arg_indices, arg.index);
	}

	ast c = acall(name, args);
	c.index = add_type(table, (type){.tag = T_UNKNOWN});
	c.call.index = add_callreq(
	    table,
	    (callreq){.name = name, .ret = c.index, .args = arg_indices});

	return lift(c);
}

ast *find_ref(parse_ctx *ctx, char *name)
{
	for (; ctx; ctx = ctx->next) {
		if (!ctx->defines)
			continue;

		forv(ctx->defines, d)
		{
			if (0 != strcmp(name, d->name))
				continue;

			return lift((ast){.tag = A_REF,
					  .index = d->index,
					  .ref = {.def = d}});
		}
	}

	error("ref not found: %s", name);
}

ast *parse(parse_ctx *ctx, typetable *table, ast *a)
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
		return (parse_list(ctx, table, a->list.items));
	} break;
	case A_ID: {
		switch (a->id.tag) {
		case I_WORD: {
			return find_ref(ctx, a->id.name);
		} break;
		case I_INT: {
			a->index = add_type(
			    table, (type){.tag = T_SIMPLE, .name = "int"});
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

	error("type not found: %s", show_type(x));
}

type unify_types(type a, type b)
{
	log("unify %s and %s", show_type(a), show_type(b));

	if (a.tag == T_UNKNOWN)
		return b;

	if (b.tag == T_UNKNOWN)
		return a;

	if (b.tag != a.tag)
		error("can't unify different tags yet");

	switch (a.tag) {
	case T_UNKNOWN: {
		bug("can't happen");
	} break;
	case T_COMPOUND: {
		todo;
	} break;
	case T_SIMPLE: {
		bug_if_not(a.name);
		bug_if_not(b.name);
		if (0 == strcmp(a.name, b.name))
			return a;
		error("unify simple types: %s vs %s", a.name, b.name);
	} break;
	}

	bug("unreachable");
}

void compile_ast(typetable *table, ast *a, int indent)
{
	bug_if_not(a);

	switch (a->tag) {
	case A_MARK: {
		bug("mark");
	} break;
	case A_BLOCK: {
		printf("%*s{\n", indent, "");
		forv(a->block.items, item)
		{
			compile_ast(table, item, indent + 2);
		}
		printf("%*s}\n", indent, "");
	} break;
	case A_REF: {
		printf("%s", a->ref.def->name);
	} break;
	case A_LIST: {
		bug("list");
	} break;
	case A_CALL: {
		if (indent)
			printf("%*s", indent, "");
		printf("%s(", a->call.name);
		forv(a->call.args, arg, i)
		{
			if (i)
				printf(", ");
			compile_ast(table, arg, 0);
		}
		printf(")");
		if (indent)
			printf(";\n");
	} break;

	case A_ID: {
		if (a->id.tag == I_KW)
			bug("keyword");
		if (a->id.tag == I_OP)
			bug("op");
		if (a->id.tag == I_WORD)
			bug("word");
		if (a->id.tag == I_INT)
			printf("%s", a->id.name);
	} break;

	case A_OPER: {
		todo;
	} break;
	}
}

void compile_one(parse_ctx *ctx, typetable *table, function *fun)
{
	printf("%s %s(", show_type(get_ret_type(table)), fun->self.name);
	forv(fun->args, arg, i)
	{
		if (i != 0)
			printf(" ,");
		printf("%s %s", show_type(get_arg_type(table, i)), arg->name);
	}
	printf(") {\n");
	compile_ast(table, fun->self.init, 2);
	printf("}\n");
}

void compile(parse_ctx *ctx, typetable *table, callreq *req)
{
	vec(function *) candidates = find_candidates(ctx, table, req->name);

	if (vlen(candidates) > 1)
		error("too many candidates: %s", req->name);

	if (vlen(candidates) == 0)
		error("no candidates: %s", req->name);

	function *f = vget(candidates, 0);

	typetable unitable = clone_table(f->table);

	unitable.ret = unify_types(unitable.ret, table->ret);
	forv(unitable.args, argp, i)
	{
		*argp = unify_types(*argp, get_arg_type(table, i));
	}

	compile_one(ctx, &unitable, f);
}

int main(int argc, char **argv)
{
	check(argc == 2);

	FILE *f = fopen(argv[1], "r");
	check(f);

	vec(ast) tops = avec(ast);

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
	typetable maintable = new_table();
	size_t mainret =
	    add_ret_type(&maintable, (type){.tag = T_SIMPLE, .name = "int"});
	vec(size_t) mainargs = avec(size_t);
	callreq mainreq =
	    (callreq){.name = "main", .ret = mainret, .args = mainargs};

	compile(&topctx, &maintable, &mainreq);

	log("END");
}
