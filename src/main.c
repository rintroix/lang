#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "data.h"
#include "parser.h"
#include "um/um.h"

// TODO lets
// TODO propagate through assignment
// TODO call fetches actual generated fn

static inline type tint(size_t bits)
{
	return (type){.tag = T_INTEGER, .integer = {.bits = bits}};
}

ast buggyast = (ast){.tag = A_ID, .id = {.tag = I_KW, .name = "!BUG!"}};

ast parse(context *ctx, typetable *table, ast a);
void compile(context *ctx, output *o, typetable *table, callreq req);

#define lift(...)                                                              \
	_Generic((__VA_ARGS__), ast : alift, type : tlift)(__VA_ARGS__)

#define trace(...)                                                             \
	_Generic((__VA_ARGS__), ast * : aptrace, type : ttrace)(__VA_ARGS__)

typedef enum e_flags {
	F_RETURN = 1 << 0,
} flags;

static inline enum e_flags noret(enum e_flags f) { return f & ~F_RETURN; }

char *show_type(type t)
{
	switch (t.tag) {
	case T_UNKNOWN: {
		return "UNKNOWN";
	} break;
	case T_INTEGER: {
		switch (t.integer.bits) {
		case 0:
			return "int";
		case 16:
			return "i16";
		case 32:
			return "i32";
		case 64:
			return "i64";
		default:
			bug("wrong integer bits");
		}
	} break;
	case T_FLOATING: {
		switch (t.integer.bits) {
		case 0:
			return "float";
		case 16:
			return "f16";
		case 32:
			return "f32";
		case 64:
			return "f64";
		default:
			bug("wrong floating bits");
		}
	} break;
	case T_STRING: {
		todo;
	} break;
	case T_COMPOUND: {
		todo;
	} break;
	}

	bug("wrong type tag: %d", t.tag);
}

int same_type(type a, type b)
{
	if (a.tag != b.tag)
		return 0;

	switch (a.tag) {
	case T_INTEGER: {
		return a.integer.bits == b.integer.bits;
	} break;
	case T_COMPOUND: {
		todo;
	} break;
	case T_FLOATING: {
		todo;
	} break;
	case T_STRING: {
		todo;
	} break;
	case T_UNKNOWN: {
		return a.unknown.index == b.unknown.index;
	} break;
	}

	bug("unreachable");
}

type _unify_types(type a, type b)
{
	if (a.tag == T_UNKNOWN)
		return b;

	if (b.tag == T_UNKNOWN)
		return a;

	if (b.tag != a.tag)
		bug("unify different tags");

	switch (a.tag) {
	case T_UNKNOWN: {
		bug("unknown type");
	} break;
	case T_FLOATING: {
		return a.floating.bits > b.floating.bits ? a : b;
	} break;
	case T_STRING: {
		todo;
	} break;
	case T_COMPOUND: {
		todo;
	} break;
	case T_INTEGER: {
		size_t abits = a.integer.bits;
		size_t bbits = b.integer.bits;
		if (a.solid && b.solid) {
			if (abits != bbits)
				bug("type mismatch: %s! vs %s!", show_type(a),
				    show_type(b));
			return a;
		}

		if (a.solid) {
			if (abits < bbits)
				bug("type mismatch: %s! vs %s", show_type(a),
				    show_type(b));

			return a;
		}

		if (b.solid) {
			if (abits > bbits)
				bug("type mismatch: %s vs %s!", show_type(a),
				    show_type(b));

			return a;
		}

		return abits > bbits ? a : b;
	} break;
	}

	bug("unreachable");
}

type unify_types(type a, type b)
{
	type r = _unify_types(a, b);
	log("UNIFY: %s + %s = %s", show_type(a), show_type(b), show_type(r));
	return r;
}

size_t add_type(typetable *table, type t) { return push(table->types, t); }

void change_type(typetable *table, size_t index, type t)
{
	*vat(table->types, index) = t;
}

size_t add_unknown(typetable *table)
{
	return push(
	    table->types,
	    (type){.tag = T_UNKNOWN, .unknown = {.index = vlen(table->types)}});
}

size_t add_opreq(typetable *table, opreq o) { return push(table->ops, o); }

size_t add_callreq(typetable *table, callreq c)
{
	return push(table->calls, c);
}

callreq *get_callreqp(typetable *table, size_t index)
{
	return vat(table->calls, index);
}

type get_type(typetable *table, size_t index)
{
	return vget(table->types, index);
}

typetable new_table()
{
	return (typetable){
	    .types = avec(type),
	    .calls = avec(callreq),
	    .ops = avec(opreq),
	    .arity = 0,
	};
}

typetable clone_table(typetable table)
{
	return (typetable){
	    .types = vslice(table.types, 0, vlen(table.types)),
	    .calls = vslice(table.calls, 0, vlen(table.calls)),
	    .ops = vslice(table.ops, 0, vlen(table.ops)),
	    .arity = table.arity,
	};
}

int same_table(typetable a, typetable b)
{
	if (a.arity != b.arity)
		return 0;

	vloop(a.types, t, 0, a.arity, i)
	{
		if (!same_type(t, vget(b.types, i)))
			return 0;
	}

	return 1;
}

type find_simple_type(context *ctx, char *name)
{
	for (; ctx; ctx = ctx->next) {
		if (!ctx->types)
			continue;

		veach(ctx->types, t) {}
	}
	// TODO actually find

	if (0 == strcmp("int", name))
		return tint(0);

	if (0 == strcmp("float", name))
		return (type){.tag = T_FLOATING, .integer = {.bits = 0}};

	error("type not found: %s", name);
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

char *show_sig(typetable table)
{
	deq(char) str = adeq(char);
	umd_append_fmt(str, "[");
	vloop(table.types, t, 0, table.arity + 1, i)
	{
		if (i != 0)
			umd_append_fmt(str, ", ");
		umd_append_fmt(str, "%s", show_type(t));
	}
	umd_append_fmt(str, "]");

	return umd_to_cstr(str);
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

type list2type(context *ctx, vec(ast) items, size_t start, size_t end)
{
	check(end > start);

	if (end - start == 1) {
		ast *head = vat(items, start);
		check(head->tag == A_ID && head->id.tag == I_WORD);
		char *name = head->id.name;
		return find_simple_type(ctx, name);
	}

	todo; // compound and funs
}

vec(define) funargs(context *ctx, typetable *table, vec(ast) list)
{
	vec(define) out = avec(define);

	veach(list, it)
	{
		switch (it.tag) {
		case A_ID: {
			check(it.id.tag == I_WORD);
			size_t index = add_unknown(table);
			push(out, (define){.name = it.id.name, .index = index});
		} break;

		case A_LIST: {
			vec(ast) items = it.list.items;
			check(vlen(items) > 1);
			ast head = vget(items, 0);
			check(head.tag == A_ID && head.id.tag == I_WORD);
			char *name = head.id.name;
			type t = list2type(ctx, items, 1, vlen(items));
			t.solid = 1;
			size_t index = add_type(table, t);
			push(out, (define){.name = name, .index = index});
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

ast parse_operator(context *ctx, typetable *table, vec(ast) list, size_t start)
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
	out.index = add_unknown(table);
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

void back_propagate(typetable *table, ast *a, size_t index)
{
	a->index = index;
	switch (a->tag) {
	case A_REF: {
		a->ref.def->index = index;
	} break;
	case A_BLOCK: {
		vec(ast) items = a->block.items;
		bug_if_not(vlen(items));
		back_propagate(table, vat(items, vlen(items) - 1), index);
	} break;
	case A_OPER: {
		bug("oper");
	} break;
	case A_CALL: {
		callreq *cr = get_callreqp(table, a->call.req);
		cr->ret = index;
	} break;
	case A_LIST: {
		bug("list");
	} break;
	case A_ID: {
		bug("id");
	} break;
	}

	return;
}

int make_fn(context *ctx, char *name, type *fret, vec(ast) args, vec(ast) body)
{
	typetable table = new_table();
	size_t retindex = fret ? add_type(&table, *fret) : add_unknown(&table);

	// TODO funargs require context as well, inits might need it
	vec(define) defs = funargs(ctx, &table, args);
	table.arity = vlen(args);
	context local = (context){.defines = defs, .next = ctx};
	ast init = parse_block(&local, &table, body, 0, vlen(body)); // TODO
	back_propagate(&table, &init, 0);
	type iret = get_type(&table, init.index);
	if (fret) {
		type ret = unify_types(*fret, iret);
		change_type(&table, 0, ret);
	}

	define self = def(name, retindex, init);

	push(ctx->functions, fn(self, defs, table));
	return 1;
}

int make_extern(context *ctx, char *name, type ret, vec(ast) args)
{
	for (; ctx; ctx = ctx->next) {
		if (!ctx->functions)
			continue;

		typetable table = new_table();
		size_t index = add_type(&table, ret);
		define self =
		    (define){.name = name, .index = index, .init = buggyast};
		vec(define) defs = funargs(ctx, &table, args);
		table.arity = vlen(defs);

		function f = fn(self, defs, table);
		f.flags = FUN_EXTERN;

		push(ctx->functions, f);

		return 1;
	}

	bug("no externs");
}

int parse_top_one(context *ctx, vec(ast) items)
{
	if (match(items, W("fn"), W(0), W(0), L(0))) {
		char *name = vat(items, 1)->id.name;
		// TODO extraction hack
		type t = list2type(ctx, items, 2, 3);
		t.solid = 1;
		vec(ast) args = vat(items, 3)->list.items;
		vec(ast) body = vslice(items, 4, vlen(items));
		return make_fn(ctx, name, &t, args, body);
	}

	if (match(items, W("fn"), W(0), L(0))) {
		char *name = vat(items, 1)->id.name;
		vec(ast) args = vat(items, 2)->list.items;
		vec(ast) body = vslice(items, 3, vlen(items));
		return make_fn(ctx, name, 0, args, body);
	}

	if (match(items, W("let"))) {
		todo;
		// if def, put to defs
		return 1;
	}

	if (match(items, W("extern"), W("fn"), W(0), W(0), L(0))) {
		char *name = vat(items, 2)->id.name;
		// TODO extraction hack
		type t = list2type(ctx, items, 3, 4);
		t.solid = 1;
		vec(ast) args = vat(items, 4)->list.items;
		return make_extern(ctx, name, t, args);
	}

	return 0;
}

int types_compatible(type a, type b)
{
	if (a.tag == T_UNKNOWN || b.tag == T_UNKNOWN)
		return 1;

	if (a.tag != b.tag)
		return 0; // TODO promotions

	switch (a.tag) {
	case T_INTEGER: {
		if (a.integer.bits == 0 || b.integer.bits == 0)
			return 1;
		if (a.integer.bits == b.integer.bits)
			return 1;
		return 0;
	} break;
	case T_FLOATING: {
		todo;
	} break;
	case T_STRING: {
		todo;
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
	if (a.arity != b.arity)
		return 0;

	vloop(a.types, it, 0, 1 + a.arity, i)
	{
		if (!types_compatible(it, vget(b.types, i)))
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

		veach(ctx->functions, f, i, fp)
		{
			if (0 != strcmp(name, f.self.name))
				continue;

			dbg("name good: %s", name);

			if (table_compatible(*table, f.table)) {
				push(out, fp);
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
	out.index = add_unknown(table);
	out.call.req = add_callreq(
	    table,
	    (callreq){.name = name, .ret = out.index, .args = arg_indices});

	return out;
}

ast find_ref(context *ctx, char *name)
{
	for (; ctx; ctx = ctx->next) {
		if (!ctx->defines)
			continue;

		veach(ctx->defines, d, i, dp)
		{
			if (0 != strcmp(name, d.name))
				continue;

			return ((ast){.tag = A_REF,
				      .index = d.index,
				      .ref = {.def = dp}});
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
			a.index = add_type(table, tint(0));
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

int oreg(output *o, function *fp, typetable table)
{
	veach(o->implementations, imp)
	{
		if (fp != imp.fp)
			continue;

		if (same_table(table, imp.table))
			return 0;
	}

	push(o->implementations, (impl){.fp = fp, .table = table});
	return 1;
}

void compile_ast(output *o, typetable *table, flags fl, ast a, int indent)
{
	switch (a.tag) {
	case A_BLOCK: {
		size_t len = vlen(a.block.items);
		bug_if(len == 0);
		int sub = indent;
		if (!a.block.after_block) {
			odef(o, "%*s{\n", indent, "");
			sub = indent + 2;
		}
		vloop(a.block.items, item, 0, len - 1)
		{
			compile_ast(o, table, noret(fl), item, sub);
		}
		compile_ast(o, table, fl, vget(a.block.items, len - 1), sub);
		if (!a.block.after_block) {
			odef(o, "%*s}\n", indent, "");
		}
	} break;
	case A_REF: {
		if (indent)
			odef(o, "%*s", indent, "");

		if (fl & F_RETURN)
			odef(o, "return "), bug_if_not(indent);

		odef(o, "%s", a.ref.def->name);

		if (indent)
			odef(o, ";\n");
	} break;
	case A_LIST: {
		bug("list");
	} break;
	case A_CALL: {
		if (indent)
			odef(o, "%*s", indent, "");

		if (fl & F_RETURN)
			odef(o, "return "), bug_if_not(indent);

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
	veach(table->calls, req, i)
	{
		typetable t = new_table();
		add_type(&t, get_type(table, req.ret));
		t.arity = vlen(req.args);
		veach(req.args, argi) { add_type(&t, get_type(table, argi)); }
		compile(ctx, o, &t, req);
	}

	if (fun->flags & FUN_EXTERN)
		return;

	char *ret = show_type(get_type(table, 0));
	char *name = fun->self.name;

	odec(o, "%s %s(", ret, name);
	odef(o, "%s %s(", ret, name);

	veach(fun->args, arg, i)
	{
		if (i != 0)
			odec(o, ", "), odef(o, ", ");
		char *tname = show_type(get_type(table, arg.index));
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

void compile(context *ctx, output *o, typetable *table, callreq req)
{
	vec(function *) candidates = find_candidates(ctx, table, req.name);

	if (vlen(candidates) > 1)
		error("too many candidates: %s", req.name);

	if (vlen(candidates) == 0)
		error("no candidates: %s", req.name);

	function *f = vget(candidates, 0);

	typetable unitable = clone_table(f->table);

	vloop(unitable.types, t, 0, 1 + unitable.arity, i) // TODO loopP
	{
		*vat(unitable.types, i) = unify_types(t, get_type(table, i));
	}

	if (oreg(o, f, unitable)) {
		compile_fn(ctx, o, &unitable, f);
	}
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
	size_t mainret = add_type(&maintable, tint(0));
	vec(size_t) mainargs = avec(size_t);
	callreq mainreq =
	    (callreq){.name = "main", .ret = mainret, .args = mainargs};

	output o = (output){
	    .definitions = adeq(char),
	    .declarations = adeq(char),
	    .implementations = avec(impl),
	};

	odec(&o, "#include<stdint.h>\n");

	compile(&topctx, &o, &maintable, mainreq);

	printf("%s", umd_to_cstr(o.declarations));
	printf("%s", umd_to_cstr(o.definitions));

	log("END");
}
