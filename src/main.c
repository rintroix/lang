#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "data.h"
#include "parser.h"
#include "um/um.h"

ast parse(context *ctx, typetable *table, ast a);

#define lift(...)                                                              \
	_Generic((__VA_ARGS__), ast : alift, type : tlift)(__VA_ARGS__)

#define trace(...)                                                             \
	_Generic((__VA_ARGS__), ast * : aptrace, type : ttrace)(__VA_ARGS__)

typedef enum e_flags {
	F_RETURN = 1 << 0,
} flags;

static inline enum e_flags noret(enum e_flags f) {
	return f & ~F_RETURN;
}

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

type *tlift(type t)
{
	type *ptr = malloc(sizeof(type));
	*ptr = t;
	return ptr;
}

int is(ast a, ast *pat)
{
	if (!pat)
		return 1;

	if (pat->tag != a.tag)
		return 0;

	switch (pat->tag) {
	case A_OPER: {
		todo;
	} break;
	case A_REF: {
		todo;
	} break;
	case A_BLOCK: {
		todo;
	} break;
	case A_ID: {
		if (pat->id.tag != a.id.tag)
			return 0;

		if (!pat->id.name)
			return 1;

		if (0 == strcmp(pat->id.name, a.id.name))
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
	veach(list, it)
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

void _show_ast(vec(char) out, ast a)
{
	switch (a.tag) {
	case A_BLOCK: {
		todo;
	} break;
	case A_REF: {
		cadd(out, "<");
		cadd(out, a.ref.def->name);
		cadd(out, ">");
	} break;
	case A_LIST: {
		cadd(out, "(");
		veach(a.list.items, it, i)
		{
			if (i != 0)
				cadd(out, " ");
			_show_ast(out, it);
		}
		cadd(out, ")");
	} break;

	case A_CALL: {
		cadd(out, a.call.name);
		cadd(out, "(");
		veach(a.call.args, arg, i)
		{
			if (i != 0)
				cadd(out, " ");
			_show_ast(out, arg);
		}
		cadd(out, ")");
	} break;

	case A_ID:
		if (a.id.tag == I_KW)
			cadd(out, ":");
		cadd(out, a.id.name);
		break;

	case A_OPER:
		cadd(out, "(");
		_show_ast(out, *a.oper.left);
		cadd(out, " ");
		cadd(out, a.oper.name);
		cadd(out, " ");
		_show_ast(out, *a.oper.right);
		cadd(out, ")");
		break;
	}
}

char *show_ast(ast a)
{
	vec(char) chars = avec(char);
	_show_ast(chars, a);
	char *out = malloc(vlen(chars) + 1);
	veach(chars, c, i) { out[i] = c; }
	out[vlen(chars)] = '\0';
	return out;
}

void print_ast(ast a) { printf("%s", show_ast(a)); }

void printl_ast(ast a)
{
	print_ast(a);
	printf("\n");
}

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

	veach(list, it)
	{
		switch (it.tag) {
		case A_ID: {
			check(it.id.tag == I_WORD);
			size_t index =
			    add_arg_type(tt, (type){.tag = T_UNKNOWN});
			push(out,
			     (define){.name = it.id.name, .index = index});
		} break;

		case A_LIST: {
			vec(ast) items = it.list.items;
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

ast atom_or_list(vec(ast) iter, size_t start, size_t end)
{
	assert(end >= start);
	switch (end - start) {
	case 0:
		bug("%s: empty", __func__);
		break;

	case 1:
		return *vat(iter, start);
		break;

	default:
		return alist(vslice(iter, start, end));
		break;
	}
}

ast parse_operator(context *ctx, typetable *table, vec(ast) list,
		    size_t start)
{
	// TODO opreq, same as callreq
	// TODO better find
	size_t pos = vlen(list);
	vloop(list, it, start, vlen(list), index)
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

	ast left = parse(ctx, table, atom_or_list(list, start, pos));
	ast right = parse_operator(ctx, table, list, pos + 1);
	ast out = aoper(op->id.name, lift(left), lift(right));
	out.index = add_type(table, (type){.tag = T_UNKNOWN});
	out.oper.index = add_opreq(table, (opreq){.name = op->id.name,
						  .ret = out.index,
						  .left = left.index,
						  .right = right.index});
	return out;
}

int _match(vec(ast) list, ast **patterns, size_t n)
{
	veach(list, it, index)
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

ast parse_block(context *ctx, typetable *table, vec(ast) list, size_t start,
		size_t end)
{
	// TODO compactor first

	check(end >= start);

	vec(define) defs = avec(define);
	vec(ast) items = avec(ast);

	// TODO defs in ctx

	vloop(list, it, start, end)
	{
		ast b = parse(ctx, table, it);
		push(items, b);
	}

	return ablock(0, defs, items); // TODO functions
}

int parse_top_one(context *ctx, vec(ast) items)
{
	if (match(items, W("fn"), W(0), W(0), L(0))) {
		char *name = vat(items, 1)->id.name;
		typetable table = new_table();

		size_t retindex = add_ret_type(
		    &table, list2type(items, 2, 3)); // TODO extraction hack

		// TODO funargs require context as well, inits might need it
		vec(define) args = funargs(&table, vat(items, 3)->list.items);
		context local = (context){.defines = args, .next = ctx};
		ast init = parse_block(&local, &table, items, 4, vlen(items));
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

	veach(a.args, it, i)
	{
		if (!types_compatible(it, vget(b.args, i)))
			return 0;
	}

	return 1;
}

vec(function *) find_candidates(context *ctx, typetable *table, char *name)
{
	vec(function *) out = avec(function *);

	for (; ctx; ctx = ctx->next) {
		if (!ctx->functions)
			continue;

		veach(ctx->functions, f, i)
		{
			if (0 != strcmp(name, f.self.name))
				continue;

			if (table_compatible(*table, f.table)) {
				push(out, vat(ctx->functions, i)); // TODO ptr loop
			}
		}
	}

	return out;
}

context parse_top(context *upper, vec(ast) items)
{
	vec(function) functions = avec(function);
	vec(define) defines = avec(define);

	context current = (context){
	    .functions = functions, .defines = defines, .next = upper};

	veach(items, it)
	{
		if (it.tag != A_LIST)
			error("not a list: %s", show_ast(it));

		if (!parse_top_one(&current, it.list.items))
			error("bad top ast: %s", show_ast(it));
	}

	return current;
}

ast parse_list(context *ctx, typetable *table, vec(ast) items)
{
	if (has(items, O(0))) {
		return parse_operator(ctx, table, items, 0);
	}

	ast *head = vat(items, 0);
	assert(head->tag == A_ID && head->id.tag == I_WORD);
	char *name = head->id.name;

	vec(ast) args = avec(ast);
	vec(size_t) arg_indices = avec(size_t);
	vloop(items, item, 1, vlen(items))
	{
		ast arg = parse(ctx, table, item);
		push(args, arg);
		push(arg_indices, arg.index);
	}

	ast out = acall(name, args);
	out.index = add_type(table, (type){.tag = T_UNKNOWN});
	out.call.index = add_callreq(
	    table,
	    (callreq){.name = name, .ret = out.index, .args = arg_indices});

	return out;
}

ast find_ref(context *ctx, char *name)
{
	for (; ctx; ctx = ctx->next) {
		if (!ctx->defines)
			continue;

		veach(ctx->defines, d, i)
		{
			if (0 != strcmp(name, d.name))
				continue;

			// TODO ptr loop
			return ((ast){.tag = A_REF,
				      .index = d.index,
				      .ref = {.def = vat(ctx->defines, i)}});
		}
	}

	error("ref not found: %s", name);
}

ast parse(context *ctx, typetable *table, ast a)
{
	switch (a.tag) {
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
	case A_LIST: {
		return (parse_list(ctx, table, a.list.items));
	} break;
	case A_ID: {
		switch (a.id.tag) {
		case I_WORD: {
			return find_ref(ctx, a.id.name);
		} break;
		case I_INT: {
			a.index = add_type(
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

__attribute__((format(printf, 2, 3))) void odec(output *o,
						const char *restrict fmt, ...)
{
	va_list base;
	va_start(base, fmt);
	check(0 <= umd_append_vfmt(o->declarations, fmt, base));
	va_end(base);
}

__attribute__((format(printf, 2, 3))) void odef(output *o,
						const char *restrict fmt, ...)
{
	va_list base;
	va_start(base, fmt);
	check(0 <= umd_append_vfmt(o->definitions, fmt, base));
	va_end(base);
}

void compile_ast(output *o, typetable *table, flags fl, ast a, int indent)
{
	switch (a.tag) {
	case A_BLOCK: {
		int sub = indent + 2;
		if (!a.block.after_block) {
			odef(o, "%*s{\n", indent, "");
		} else {
			sub = indent;
		}
		// TODO range loop
		veach(a.block.items, item, i)
		{
			if (i + 1 == vlen(a.block.items)) {
				compile_ast(o, table, fl, item, sub);
			} else {
				compile_ast(o, table, noret(fl), item, sub);
			}
		}
		if (!a.block.after_block) {
			odef(o, "%*s}\n", indent, "");
		}
	} break;
	case A_REF: {
		odef(o, "%s", a.ref.def->name);
	} break;
	case A_LIST: {
		bug("list");
	} break;
	case A_CALL: {
		if (indent)
			odef(o, "%*s", indent, "");

		if (fl & F_RETURN)
			odef(o, "return ");

		odef(o, "%s(", a.call.name);
		veach(a.call.args, arg, i)
		{
			if (i)
				odef(o, ", ");
			compile_ast(o, table, noret(fl), arg, 0);
		}
		odef(o, ")");
		if (indent)
			odef(o, ";\n");
	} break;

	case A_ID: {
		if (a.id.tag == I_KW)
			bug("keyword");
		if (a.id.tag == I_OP)
			bug("op");
		if (a.id.tag == I_WORD)
			bug("word");
		if (a.id.tag == I_INT)
			odef(o, "%s", a.id.name);
	} break;

	case A_OPER: {
		todo;
	} break;
	}
}

ast returning(size_t tindex, ast a)
{
	switch (a.tag) {
	case A_BLOCK: {
		size_t len = vlen(a.block.items);
		if (!len)
			bug("empty block requested to return");
		ast *last = vat(a.block.items, len - 1);
		*last = returning(tindex, *last);
		return a;
	} break;
	case A_REF: {
		todo;
	} break;
	case A_LIST: {
		todo;
	} break;
	case A_CALL: {
		todo;
	} break;

	case A_ID: {
		todo;
	} break;

	case A_OPER: {
		todo;
	} break;
	}
}

void compile_fn(context *ctx, output *o, typetable *table, function *fun)
{
	char *ret = show_type(get_ret_type(table));
	char *name = fun->self.name;

	odec(o, "%s %s(", ret, name);
	odef(o, "%s %s(", ret, name);

	veach(fun->args, arg, i)
	{
		if (i != 0)
			odec(o, ", "), odef(o, ", ");
		char *tname = show_type(get_arg_type(table, i));
		odec(o, "%s %s", tname, arg.name);
		odef(o, "%s %s", tname, arg.name);
	}
	odec(o, ");\n");
	odef(o, ") {\n");

	ast body = fun->self.init;
	if (body.tag == A_BLOCK)
		body.block.after_block = 1;
	compile_ast(o, table, F_RETURN, body, 2);
	
	odef(o, "}\n");
}

void compile(context *ctx, output *o, typetable *table, callreq *req)
{
	vec(function *) candidates = find_candidates(ctx, table, req->name);

	if (vlen(candidates) > 1)
		error("too many candidates: %s", req->name);

	if (vlen(candidates) == 0)
		error("no candidates: %s", req->name);

	function *f = vget(candidates, 0);

	typetable unitable = clone_table(f->table);

	unitable.ret = unify_types(unitable.ret, table->ret);
	veach(unitable.args, argp, i) // TODO ptr loop
	{
		*vat(unitable.args, i) =
		    unify_types(argp, get_arg_type(table, i));
	}

	compile_fn(ctx, o, &unitable, f);
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

	context upper =
	    (context){.defines = avec(define), .macros = avec(macro)};

	context topctx = parse_top(&upper, tops);
	typetable maintable = new_table();
	size_t mainret =
	    add_ret_type(&maintable, (type){.tag = T_SIMPLE, .name = "int"});
	vec(size_t) mainargs = avec(size_t);
	callreq mainreq =
	    (callreq){.name = "main", .ret = mainret, .args = mainargs};

	output o = (output){
	    .declarations = adeq(char),
	    .definitions = adeq(char),
	};

	compile(&topctx, &o, &maintable, &mainreq);

	printf("%s\n", umd_to_cstr(o.declarations));
	printf("%s\n", umd_to_cstr(o.definitions));

	log("END");
}
