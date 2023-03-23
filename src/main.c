#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "data.h"
#include "parser.h"
#include "um/um.h"

// TODO variadic c functions
// TODO lets
// TODO propagate through assignment
// TODO call fetches actual generated fn

// decls
static int is(ast a, ast *pat);
static ir parse_ir(typetable *table, vec(ir) body, ast a);
// static ast parse(context *ctx, typetable *table, ast a);
static void compile(context *ctx, typetable *table, callreq req);
static callreq *get_callreqp(typetable *table, size_t index);

static inline type tint(size_t bits)
{
	return (type){.tag = T_INTEGER, .integer = {.bits = bits}};
}

static int add_include(context *ctx, char *name)
{
	for (; ctx; ctx = ctx->next) {
		if (!ctx->includes)
			continue;

		push(ctx->includes, name);
		return 1;
	}

	bug("no includes");
}

static ast buggyast = (ast){.tag = A_KW, .kw = {.name = "!BUG!"}};

#define lift(...)                                                              \
	_Generic((__VA_ARGS__), ast : alift, type : tlift)(__VA_ARGS__)

#define trace(...)                                                             \
	_Generic((__VA_ARGS__), ast * : aptrace, type : ttrace)(__VA_ARGS__)

typedef enum e_flags {
	F_RETURN = 1 << 0,
} flags;

static inline enum e_flags noret(enum e_flags f) { return f & ~F_RETURN; }

static int _match(vec(ast) list, ast **patterns, size_t n)
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

static void _show_ast(deq(char) s, ast a)
{
	switch (a.tag) {
	case A_STR: {
		dprint(s, "\"%s\"", a.str.value);
	} break;
	case A_FLOAT: {
		todo;
	} break;
	case A_INT: {
		todo;
	} break;
	case A_DIAMOND: {
		todo;
	} break;
	case A_KW: {
		todo;
	} break;
	case A_BLOCK: {
		todo;
	} break;
	case A_REF: {
		char *name = a.ref.def ? a.ref.def->name : a.ref.name;
		dprint(s, "%s", name);
	} break;
	case A_LIST: {
		dprint(s, "(");
		veach(a.list.items, it, i)
		{
			if (i != 0)
				dprint(s, " ");
			_show_ast(s, it);
		}
		dprint(s, ")");
	} break;

	case A_CALL: {
		dprint(s, "%s", a.call.name);
		dprint(s, "(");
		veach(a.call.args, arg, i)
		{
			if (i != 0)
				dprint(s, " ");
			_show_ast(s, arg);
		}
		dprint(s, ")");
	} break;

	case A_OPER: {
		dprint(s, "(");
		_show_ast(s, *a.oper.left);
		dprint(s, " %s ", a.oper.name);
		_show_ast(s, *a.oper.right);
		dprint(s, ")");
	} break;
	}
}

static char *_show_ir(typetable *t, ir e)
{
	char *out;
	switch (e.tag) {
	case I_DEF: {
		asprintf(&out, "DEF   %2zu %s", e.def.index.value,
			 e.def.name ? e.def.name : ".");
	} break;
	case I_REF: {
		asprintf(&out, "REF   %2zu", e.ref.index);
	} break;
	case I_USE: {
		asprintf(&out, "USE   %2zu %s", e.use.index, e.use.name);
	} break;
	case I_CALL: {
		asprintf(&out, "CALL  %2zu %s", e.call.count,
			 t ? get_callreqp(t, e.call.index)->name : "");
	} break;
	case I_OPER: {
		todo;
	} break;
	case I_LINT: {
		asprintf(&out, "LINT  %2zu %d", e.lint.index.value, e.lint.value);
	} break;
	case I_LSTR: {
		todo;
	} break;
	case I_LFLT: {
		todo;
	} break;
	case I_SKIP: {
		asprintf(&out, "SKIP  %2zu", e.skip.count);
	} break;
	case I_OPEN: {
		asprintf(&out, "OPEN");
	} break;
	case I_CLOSE: {
		asprintf(&out, "CLOSE");
	} break;
	case I_SET: {
		asprintf(&out, "SET   %2zu", e.set.index);
	} break;
	case I_RET: {
		asprintf(&out, "RET");
	} break;
	}
	return out;
}

static char *show_ir(ir e) {
	return _show_ir(NULL, e);
}

static void dump_ir_full(typetable *table, vec(ir) items)
{
	veach(items, item, i) { log("\t%03d %s", i, _show_ir(table, item)); }
}

static void dump_ir(vec(ir) items)
{
	dump_ir_full(NULL, items);
}

static type make_simple_type(char *name)
{
	if (0 == strcmp("int", name))
		return tint(0);

	if (0 == strcmp("i32", name))
		return tint(32);

	if (0 == strcmp("float", name))
		return (type){.tag = T_FLOATING, .integer = {.bits = 0}};

	error("type not found: %s", name);
}

static type list2type(vec(ast) items, size_t start, size_t end)
{
	check(end > start);

	if (end - start == 1) {
		ast *head = vat(items, start);
		check(head->tag == A_REF);
		char *name = head->ref.name;
		return make_simple_type(name);
	}

	todo; // compound and funs
}

static char *show_ast(ast a)
{
	deq(char) s = adeq(char);
	_show_ast(s, a);
	return umd_to_cstr(s);
}

static context chain_context(context *ctx)
{
	return (context){.out = ctx->out, .next = ctx};
}

static char *show_type(type t)
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

	bug("unreachable");
}

static int same_type(type a, type b)
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

static type *_unify_types(type *a, type *b)
{
	if (a->tag == T_UNKNOWN)
		return b;

	if (b->tag == T_UNKNOWN)
		return a;

	if (a->tag != b->tag)
		return NULL; // TODO promotions

	switch (a->tag) {
	case T_UNKNOWN: {
		bug("unknown handled above");
	} break;
	case T_FLOATING: {
		todo;
		// return a->floating.bits > b->floating.bits ? a : b;
	} break;
	case T_STRING: {
		todo;
	} break;
	case T_COMPOUND: {
		todo;
	} break;
	case T_INTEGER: {
		size_t abits = a->integer.bits;
		size_t bbits = b->integer.bits;
		if (a->solid && b->solid) {
			return abits == bbits ? a : NULL;
		}

		if (a->solid) {
			return abits < bbits ? NULL : a;
		}

		if (b->solid) {
			return abits > bbits ? NULL : b;
		}

		return abits > bbits ? a : b;
	} break;
	}

	bug("unreachable");
}

static inline type *unify_types(type *a, type *b)
{
	type *r = _unify_types(a, b);
	dbg("UNIFY: %s%s + %s%s = %s%s", show_type(*a), a->solid ? "!" : "",
	    show_type(*b), b->solid ? "!" : "", r ? show_type(*r) : "!FAIL!",
	    (r && r->solid) ? "!" : "");
	return r;
}

static typeindex add_type(typetable *table, type t)
{
	return (typeindex){push(table->types, t)};
}

static void change_type(typetable *table, size_t index, type t)
{
	*vat(table->types, index) = t;
}

static typeindex add_unknown(typetable *table)
{
	type t = (type){.tag = T_UNKNOWN, .unknown = {.index = vlen(table->types)}};
	return add_type(table, t);
}

static size_t add_gen(typetable *table, vec(ir) body, type t)
{
	typeindex tindex = add_type(table, t);
	return push(body, idef(NULL, tindex));
}

static size_t add_gen_unknown(typetable *table, vec(ir) body)
{
	typeindex tindex = add_unknown(table);
	return push(body, idef(NULL, tindex));
}

static size_t add_def(typetable *table, vec(ir) body, char *name, type t)
{
	typeindex tindex = add_type(table, t);
	return push(body, idef(name, tindex));
}

static size_t add_def_unknown(typetable *table, vec(ir) body, char *name)
{
	typeindex tindex = add_unknown(table);
	return push(body, idef(name, tindex));
}

static ir add_return(typetable *table, vec(ir) body, ir e)
{
	push(body, (ir){.tag=I_RET});
	push(body, e);
	return iskip(0);
}

static size_t add_opreq(typetable *table, opreq o)
{
	return push(table->ops, o);
}

static size_t add_callreq(typetable *table, callreq c)
{
	return push(table->calls, c);
}

static size_t add_use(typetable *table, size_t index)
{
	return push(table->uses, index);
}

static callreq *get_callreqp(typetable *table, size_t index)
{
	return vat(table->calls, index);
}

static type get_type(typetable *table, size_t index)
{
	return vget(table->types, index);
}

static typetable new_table(size_t arity)
{
	return (typetable){
	    .types = avec(type),
	    .calls = avec(callreq),
	    .ops = avec(opreq),
	    .uses = avec(size_t),
	    .arity = arity,
	};
}

static typetable clone_table(typetable table)
{
	return (typetable){
	    .types = vslice(table.types, 0, vlen(table.types)),
	    .calls = vslice(table.calls, 0, vlen(table.calls)),
	    .ops = vslice(table.ops, 0, vlen(table.ops)),
	    .uses = vslice(table.uses, 0, vlen(table.ops)), // TODO RO
	    .arity = table.arity,
	};
}

static int same_table(typetable a, typetable b)
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

static ast *alift(ast a)
{
	ast *ptr = malloc(sizeof(ast));
	*ptr = a;
	return ptr;
}

static type *tlift(type t)
{
	type *ptr = malloc(sizeof(type));
	*ptr = t;
	return ptr;
}

static void embed(typetable *table, vec(ir) body, ir e)
{
	switch (e.tag) {
	case I_USE: {
		bug_if_not(e.use.index == 0);
		e.use.index = add_use(table, vlen(body));
		push(body, e);
	} break;
	case I_REF: {
		ir *defp = vat(body, e.ref.index);
		bug_if_not(defp->tag == I_DEF);
		defp->def.count++;
		push(body, e);
	} break;
	case I_LINT: {
		push(body, e);
	} break;
	default:
		bug("bad ir: %s", show_ir(e));
	}
}

static void discard(typetable *table, vec(ir) body, ir e)
{
	switch (e.tag) {
	case I_USE: {
		todo;
	} break;
	case I_REF: {
		ir *defp = vat(body, e.ref.index);
		bug_if_not(defp->tag == I_DEF);
		bug_if(defp->def.count);
		*defp = iskip(0);
	} break;
	case I_LINT: {
		todo;
	} break;
	default:
		bug("bad ir: %s", show_ir(e));
	}
}

static int name_helper(char *pat, char *name)
{
	if (!pat)
		return 1;

	if (0 == strcmp(pat, name))
		return 1;

	return 0;
}

static int is(ast a, ast *pat)
{
	if (!pat)
		return 1;

	if (pat->tag != a.tag)
		return 0;

	switch (pat->tag) {
	case A_STR: {
		if (!pat->str.value)
			return 1;
		return 0 == strcmp(pat->str.value, a.str.value);
	} break;
	case A_FLOAT: {
		todo;
	} break;
	case A_INT: {
		todo;
	} break;
	case A_DIAMOND: {
		return name_helper(pat->diamond.name, a.diamond.name);
	} break;
	case A_KW: {
		todo;
	} break;
	case A_OPER: {
		todo;
	} break;
	case A_REF: {
		return name_helper(pat->ref.name, a.ref.name);
	} break;
	case A_BLOCK: {
		todo;
	} break;
		// case A_ID: {
		// 	if (pat->id.tag != a.id.tag)
		// 		return 0;

		// 	if (!pat->id.name)
		// 		return 1;

		// 	if (0 == strcmp(pat->id.name, a.id.name))
		// 		return 1;

		// 	return 0;
		// } break;

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

static char *show_sig(typetable table)
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

static type ttrace(type t) { todo; }

static int has(vec(ast) list, ast *pattern)
{
	veach(list, it)
	{
		if (is(it, pattern))
			return 1;
	}
	return 0;
}

// #parse

static size_t count_operands(opreq r)
{
	if (!r.isop)
		return 1;

	return count_operands(*r.op.left) + count_operands(*r.op.right);
}

static opreq parse_ir_operator(typetable *table, vec(ir) body, vec(ir) post,
			       vec(ast) items, size_t start);

static ir parse_ir_list(typetable *table, vec(ir) body, vec(ast) items)
{
	if (has(items, O0)) {
		vec(ir) post = avec(ir);
		opreq req = parse_ir_operator(table, body, post, items, 0);
		size_t count = count_operands(req);
		bug_if_not(vlen(post) == count_operands(req));
		size_t reqindex = add_opreq(table, req);
		todo;
		size_t tindex = add_gen_unknown(table, body);
		push(body, ioper(reqindex, count));
		dbg("OPERATOR");
		dump_ir(post);
		todo; // copy over post items
		      // make gen def
		      // return gen def
	}

	ast *head = vat(items, 0);
	bug_if_not(head->tag == A_REF);
	char *name = head->ref.name;

	vec(ir) post = avec(ir);
	vloop(items, item, 1, vlen(items))
	{
		ir e = parse_ir(table, body, item);
		log("CALL PUSH POST %s", show_ir(e));
		push(post, e);
	}

	size_t call_def_index = add_gen_unknown(table, body);
	log("ADD %s FOR %s", show_ir(vget(body, call_def_index)), name);
	size_t call_index = vlen(body);

	size_t reqidx =
	    add_callreq(table, (callreq){.name = name,
					 .defidx = call_def_index,
					 .argsidx = call_index + 1});

	push(body, (ir){.tag = I_CALL,
			.call = {.index = reqidx, .count = vlen(post)}});
	veach(post, item) { embed(table, body, item); }

	return iref(call_def_index);
}

static ir parse_ir_many(typetable *table, vec(ir) body, vec(ast) items,
			size_t start, size_t end)
{
	bug_if(end == start);

	if (end - start == 1)
		return parse_ir(table, body, vget(items, start));

	return parse_ir_list(table, body, vslice(items, start, end));
}

static opreq parse_ir_operator(typetable *table, vec(ir) body, vec(ir) post,
			       vec(ast) items, size_t start)
{
	// TODO opreq, same as callreq
	// TODO better find

	size_t end = vlen(items);
	size_t pos = end;

	if (start == end)
		bug("empty");

	vloop(items, it, start, end, index)
	{
		if (is(it, O0)) {
			pos = index;
			break;
		}
	}

	if (pos == end) {
		// start != end, so not empty
		ir e = parse_ir_many(table, body, items, start, end);
		todo; // embed ir expr
		size_t index = push(post, e);
		return (opreq){.isop = 0, .val = {.index = index}};
	}

	if (pos == start) {
		bug("operator without left side");
	}

	todo;
	// assert(pos > start);

	// ast *op = vat(list, pos);
	// assert(op->tag == A_OPER);
	// char *name = op->oper.name;

	// ast left = parse(ctx, table, atom_or_list(list, start, pos));
	// ast right = parse_operator(ctx, table, list, pos + 1);
	// ast out = aoper(name, lift(left), lift(right));
	// out.index = add_unknown(table);
	// out.oper.index = add_opreq(table, (opreq){.name = name,
	// 					  .ret = out.index,
	// 					  .left = left.index,
	// 					  .right = right.index});
	// return out;
}

static ir parse_ir(typetable *table, vec(ir) body, ast a)
{
	switch (a.tag) {
	case A_STR: {
		todo;
	} break;
	case A_FLOAT: {
		todo;
	} break;
	case A_INT: {
		typeindex index = add_type(table, tint(0));
		return ilint(index, a.integer.value);
	} break;
	case A_DIAMOND: {
		todo;
	} break;
	case A_KW: {
		todo;
	} break;
	case A_REF: {
		return iuse(a.ref.name, 0);
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
		return parse_ir_list(table, body, a.list.items);
	} break;
	}

	bug("unreachable");
}

static ir parse_ir_block(typetable *table, vec(ir) body, vec(ast) items,
			 int ret)
{
	// TODO compactor first
	bug_if_not(vlen(items));

	size_t openidx = push(body, (ir){.tag = I_OPEN});
	size_t reservedidx = push(body, iskip(1));
	push(body, iskip(0)); // DEF no assign

	// size_t dindex = add_gen_unknown(table, body);
	veach(items, a, i)
	{
		ir e = parse_ir(table, body, a);

		if (i + 1 == vlen(items)) {

			if (ret) {
				return add_return(table, body, e);
			} else {
				size_t idx = push(
				    body, (ir){.tag = I_CLOSE,
					       .close = {.index = openidx}});
				switch (e.tag) {
				case I_USE: {
					return e;
				} break;
				case I_REF: {
					if (e.ref.index < openidx)
						bug("outer def");
					ir *defp = vat(body, e.ref.index);
					ir *next = vat(body, e.ref.index + 1);
					bug_if_not(defp->tag == I_DEF);
					bug_if(defp->def.count);
					if (next->tag == I_SKIP) {
						todo;
					} else {
						ir *reserved =
						    vat(body, reservedidx);
						*reserved = *defp;
						reserved->def.count++;
						*defp = iset(reservedidx);
						return iref(reservedidx);
					}
				} break;
				case I_LINT: {
					return e;
				} break;
				default:
					bug("bad ir: %s", show_ir(e));
				}
			}
		} else {
			discard(table, body, e);
		}
	}

	bug("unreachable");
}

static void ir_funargs(typetable *table, vec(ir) body, vec(ast) args)
{
	veach(args, arg)
	{
		switch (arg.tag) {
		case A_REF: {
			add_def_unknown(table, body, arg.ref.name);
		} break;
		case A_LIST: {
			vec(ast) items = arg.list.items;
			check(vlen(items) > 1);
			ast head = vget(items, 0);
			check(head.tag == A_REF);
			char *name = head.ref.name;
			type t = list2type(items, 1, vlen(items));
			t.solid = 1;
			add_def(table, body, name, t);
		} break;
		default:
			error("unexpected arg: %s", show_ast(arg));
		}
	}
}

static ir_function make_ir_function(char *name, type *fret, vec(ast) args,
				    vec(ast) rest)
{
	typetable table = new_table(vlen(args));

	typeindex retindex = fret ? add_type(&table, *fret) : add_unknown(&table);

	vec(ir) body = avec(ir); // TODO array to be able to jump
	push(body, iskip(vlen(args)));

	ir_funargs(&table, body, args);
	parse_ir_block(&table, body, rest, 1);
	// embed(&table, body, e);

	// TODO return 
	// TODO back propagate
	// back_ir_propagate(&table, &init, 0);

	// type iret = get_type(&table, init.index);
	// if (fret) {
	// 	type *ret = unify_types(fret, &iret);
	// 	check(ret);
	// 	change_type(&table, 0, *ret);
	// }

	// define self = def(name, retindex, init);

	return (ir_function){.name = name, .table = table, .body = body};
}

static ir_function make_ir_extern(char *name, type ret, vec(ast) args)
{
	typetable table = new_table(vlen(args));
	vec(ir) body = avec(ir); // TODO array to be able to jump
	push(body, iskip(vlen(args)));

	ir_funargs(&table, body, args);
	ir_function f =
	    (ir_function){.name = name, .table = table, .body = body};
	f.flags = FUN_EXTERN;
	return f;
}

static int parse_ir_top_one(context *ctx, vec(ast) items)
{
	if (match(items, R("fn"), R0, R0, L0)) {
		char *name = vat(items, 1)->ref.name;
		// TODO extraction hack
		type t = list2type(items, 2, 3);
		t.solid = 1;
		vec(ast) args = vat(items, 3)->list.items;
		vec(ast) body = vslice(items, 4, vlen(items));
		push(ctx->irfns, make_ir_function(name, &t, args, body));
		return 1;
	}

	if (match(items, R("fn"), R0, L0)) {
		char *name = vat(items, 1)->ref.name;
		vec(ast) args = vat(items, 2)->list.items;
		vec(ast) body = vslice(items, 3, vlen(items));
		push(ctx->irfns, make_ir_function(name, NULL, args, body));
		return 1;
	}

	if (match(items, R("let"))) {
		todo;
		// if def, put to defs
		return 1;
	}

	if (match(items, R("extern"), R("fn"), R0, R0, L0)) {
		char *name = vat(items, 2)->ref.name;
		// TODO extraction hack
		type t = list2type(items, 3, 4);
		t.solid = 1;
		vec(ast) args = vat(items, 4)->list.items;
		push(ctx->irfns, make_ir_extern(name, t, args));
		return 1;
	}

	if (match(items, R("include"), &adiamond(NULL))) {
		todo;
		// char *name = vat(items, 1)->diamond.name;
		// deq(char) inc = adeq(char);
		// dprint(inc, "<%s>", name);
		// return add_include(ctx, umd_to_cstr(inc));
	}

	if (match(items, R("include"), &astr(NULL))) {
		char *name = vat(items, 1)->str.value;
		deq(char) inc = adeq(char);
		dprint(inc, "\"%s\"", name);
		return add_include(ctx, umd_to_cstr(inc));
	}

	return 0;
}

static context parse_ir_top(vec(ast) items)
{
	context current = (context){
	    .irfns = avec(ir_function), .includes = avec(char *),
	    // TODO globals/consts
	};

	veach(items, it)
	{
		if (it.tag != A_LIST)
			error("not a list: %s", show_ast(it));

		if (!parse_ir_top_one(&current, it.list.items))
			error("bad top ast: %s", show_ast(it));
	}

	return current;
}

static void cadd(vec(char) out, char *s)
{
	while (*s) {
		push(out, *s);
		s++;
	}
}

static void print_ast(ast a) { printf("%s", show_ast(a)); }

static void printl_ast(ast a)
{
	print_ast(a);
	printf("\n");
}

// static vec(define) funargs(context *ctx, typetable *table, vec(ast) list)
// {
// 	vec(define) out = avec(define);

// 	veach(list, it)
// 	{
// 		switch (it.tag) {
// 		case A_REF: {
// 			typeindex index = add_unknown(table);
// 			push(out,
// 			     (define){.name = it.ref.name, .index = index});
// 		} break;

// 		case A_LIST: {
// 			vec(ast) items = it.list.items;
// 			check(vlen(items) > 1);
// 			ast head = vget(items, 0);
// 			check(head.tag == A_REF);
// 			char *name = head.ref.name;
// 			type t = list2type(items, 1, vlen(items));
// 			t.solid = 1;
// 			size_t index = add_type(table, t);
// 			push(out, (define){.name = name, .index = index});
// 		} break;

// 		default: {
// 			error("unexpected arg: %s", show_ast(it));
// 		} break;
// 		}
// 	}

// 	return out;
// }

static ast atom_or_list(vec(ast) iter, size_t start, size_t end)
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

// static ast parse_operator(context *ctx, typetable *table, vec(ast) list,
// 			  size_t start)
// {
// 	// TODO opreq, same as callreq
// 	// TODO better find
// 	size_t pos = vlen(list);
// 	vloop(list, it, start, vlen(list), index)
// 	{
// 		if (is(it, O0)) {
// 			pos = index;
// 			break;
// 		}
// 	}

// 	size_t end = vlen(list);

// 	if (start == end)
// 		bug("received empty");

// 	if (pos == end) {
// 		// TODO check empty list
// 		return parse(ctx, table, atom_or_list(list, start, end));
// 	}

// 	if (pos == start) {
// 		bug("%s: operator without left side", __func__);
// 	}

// 	assert(pos > start);

// 	ast *op = vat(list, pos);
// 	assert(op->tag == A_OPER);
// 	char *name = op->oper.name;

// 	ast left = parse(ctx, table, atom_or_list(list, start, pos));
// 	ast right = parse_operator(ctx, table, list, pos + 1);
// 	ast out = aoper(name, lift(left), lift(right));
// 	out.index = add_unknown(table);
// 	todo;
// 	// out.oper.index = add_opreq(table, (opreq){.name = name,
// 	// 					  .ret = out.index,
// 	// 					  .left = left.index,
// 	// 					  .right = right.index});
// 	return out;
// }

// static ast parse_block(context *ctx, typetable *table, vec(ast) list,
// 		       size_t start, size_t end)
// {
// 	// TODO compactor first

// 	check(end >= start);

// 	vec(define) defs = avec(define);
// 	vec(ast) items = avec(ast);

// 	// TODO defs in ctx

// 	vloop(list, it, start, end)
// 	{
// 		ast b = parse(ctx, table, it);
// 		push(items, b);
// 	}

// 	return ablock(NULL, defs, items); // TODO functions
// }

// static void back_propagate(typetable *table, ast *a, size_t index)
// {
// 	// TODO verify types are upgradable
// 	a->index = index;
// 	switch (a->tag) {
// 	case A_STR: {
// 		todo;
// 	} break;
// 	case A_FLOAT: {
// 		todo;
// 	} break;
// 	case A_INT: {
// 		a->index = index;
// 	} break;
// 	case A_DIAMOND: {
// 		todo;
// 	} break;
// 	case A_KW: {
// 		todo;
// 	} break;
// 	case A_REF: {
// 		a->ref.def->index = index;
// 	} break;
// 	case A_BLOCK: {
// 		vec(ast) items = a->block.items;
// 		bug_if_not(vlen(items));
// 		back_propagate(table, vat(items, vlen(items) - 1), index);
// 	} break;
// 	case A_OPER: {
// 		bug("oper");
// 	} break;
// 	case A_CALL: {
// 		callreq *cr = get_callreqp(table, a->call.req);
// 		cr->ret = index;
// 	} break;
// 	case A_LIST: {
// 		bug("list");
// 	} break;
// 	}

// 	return;
// }

// static int make_fn(context *ctx, char *name, type *fret, vec(ast) args,
// 		   vec(ast) body)
// {
// 	typetable table = new_table(vlen(args));
// 	size_t retindex = fret ? add_type(&table, *fret) : add_unknown(&table);

// 	// TODO funargs require context as well, inits might need it
// 	vec(define) defs = funargs(ctx, &table, args);

// 	context local = chain_context(ctx);
// 	local.defines = defs;

// 	ast init = parse_block(&local, &table, body, 0, vlen(body)); // TODO
// 	back_propagate(&table, &init, 0);
// 	type iret = get_type(&table, init.index);
// 	if (fret) {
// 		type *ret = unify_types(fret, &iret);
// 		check(ret);
// 		change_type(&table, 0, *ret);
// 	}

// 	define self = def(name, retindex, init);

// 	push(ctx->functions, fn(self, defs, table));
// 	return 1;
// }

// static int make_extern(context *ctx, char *name, type ret, vec(ast) args)
// {
// 	for (; ctx; ctx = ctx->next) {
// 		if (!ctx->functions)
// 			continue;

// 		typetable table = new_table(vlen(args));
// 		size_t index = add_type(&table, ret);
// 		define self =
// 		    (define){.name = name, .index = index, .init = buggyast};
// 		vec(define) defs = funargs(ctx, &table, args);

// 		function f = fn(self, defs, table);
// 		f.flags = FUN_EXTERN;

// 		push(ctx->functions, f);

// 		return 1;
// 	}

// 	bug("no externs");
// }

// static int parse_top_one(context *ctx, vec(ast) items)
// {
// 	if (match(items, R("fn"), R0, R0, L0)) {
// 		char *name = vat(items, 1)->ref.name;
// 		// TODO extraction hack
// 		type t = list2type(items, 2, 3);
// 		t.solid = 1;
// 		vec(ast) args = vat(items, 3)->list.items;
// 		vec(ast) body = vslice(items, 4, vlen(items));
// 		return make_fn(ctx, name, &t, args, body);
// 	}

// 	if (match(items, R("fn"), R0, L0)) {
// 		char *name = vat(items, 1)->ref.name;
// 		vec(ast) args = vat(items, 2)->list.items;
// 		vec(ast) body = vslice(items, 3, vlen(items));
// 		return make_fn(ctx, name, NULL, args, body);
// 	}

// 	if (match(items, R("let"))) {
// 		todo;
// 		// if def, put to defs
// 		return 1;
// 	}

// 	if (match(items, R("extern"), R("fn"), R0, R0, L0)) {
// 		char *name = vat(items, 2)->ref.name;
// 		// TODO extraction hack
// 		type t = list2type(items, 3, 4);
// 		t.solid = 1;
// 		vec(ast) args = vat(items, 4)->list.items;
// 		return make_extern(ctx, name, t, args);
// 	}

// 	if (match(items, R("include"), &adiamond(NULL))) {
// 		char *name = vat(items, 1)->diamond.name;
// 		deq(char) inc = adeq(char);
// 		dprint(inc, "<%s>", name);
// 		return add_include(ctx, umd_to_cstr(inc));
// 	}

// 	if (match(items, R("include"), &astr(NULL))) {
// 		char *name = vat(items, 1)->str.value;
// 		deq(char) inc = adeq(char);
// 		dprint(inc, "\"%s\"", name);
// 		return add_include(ctx, umd_to_cstr(inc));
// 	}

// 	return 0;
// }

static int types_compatible(type a, type b)
{
	return NULL != unify_types(&a, &b);
}

static int table_compatible(typetable a, typetable b)
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

static vec(function *)
    find_candidates(context *ctx, typetable *table, char *name)
{
	vec(function *) out = avec(function *);

	dbg("find candidates: %s %s", name, show_sig(*table));
	for (; ctx; ctx = ctx->next) {
		if (!ctx->functions)
			continue;

		veach(ctx->functions, f, i, fp)
		{
			if (0 != strcmp(name, f.self.name))
				continue;

			dbg("maybe: %s %s", name, show_sig(f.table));

			if (table_compatible(*table, f.table)) {
				push(out, fp);
				dbg("YUP");
			} else {
				dbg("NAH");
			}
		}
	}

	return out;
}

// static context parse_top(context *upper, vec(ast) items)
// {
// 	context current = chain_context(upper);
// 	current.functions = avec(function);
// 	current.defines = avec(define);
// 	current.includes = avec(char *);

// 	veach(items, it)
// 	{
// 		if (it.tag != A_LIST)
// 			error("not a list: %s", show_ast(it));

// 		if (!parse_top_one(&current, it.list.items))
// 			error("bad top ast: %s", show_ast(it));
// 	}

// 	return current;
// }

// static ast parse_list(context *ctx, typetable *table, vec(ast) items)
// {
// 	if (has(items, O0)) {
// 		return parse_operator(ctx, table, items, 0);
// 	}

// 	ast *head = vat(items, 0);
// 	assert(head->tag == A_REF);
// 	char *name = head->ref.name;

// 	vec(ast) args = avec(ast);
// 	vec(size_t) arg_indices = avec(size_t);
// 	vloop(items, item, 1, vlen(items))
// 	{
// 		ast arg = parse(ctx, table, item);
// 		push(args, arg);
// 		push(arg_indices, arg.index);
// 	}

// 	ast out = acall(name, args);
// 	out.index = add_unknown(table);
// 	out.call.req = add_callreq(
// 	    table,
// 	    (callreq){.name = name, .ret = out.index, .args = arg_indices});

// 	return out;
// }

// ast parse(context *ctx, typetable *table, ast a)
// {
// 	switch (a.tag) {
// 	case A_STR: {
// 		todo;
// 	} break;
// 	case A_FLOAT: {
// 		todo;
// 	} break;
// 	case A_INT: {
// 		a.index = add_type(table, tint(0));
// 		return a;
// 	} break;
// 	case A_DIAMOND: {
// 		todo;
// 	} break;
// 	case A_KW: {
// 		todo;
// 	} break;
// 	case A_REF: {
// 		a.ref.def = find_def(ctx, a.ref.name);
// 		a.index = a.ref.def->index;
// 		return a;
// 	} break;
// 	case A_BLOCK: {
// 		bug("block");
// 	} break;
// 	case A_OPER: {
// 		bug("oper");
// 	} break;
// 	case A_CALL: {
// 		bug("call");
// 	} break;
// 	case A_LIST: {
// 		return parse_list(ctx, table, a.list.items);
// 	} break;
// 	}
// }

__attribute__((format(printf, 2, 3))) static void
odec(output *o, const char *restrict fmt, ...)
{
	va_list base;
	va_start(base, fmt);
	check(0 <= umd_append_vfmt(o->declarations, fmt, base));
	va_end(base);
}

__attribute__((format(printf, 2, 3))) static void
odef(output *o, const char *restrict fmt, ...)
{
	va_list base;
	va_start(base, fmt);
	check(0 <= umd_append_vfmt(o->definitions, fmt, base));
	va_end(base);
}

static void emit_includes(context *ctx)
{
	bug_if_not(ctx->includes);

	veach(ctx->includes, inc) { odec(ctx->out, "#include %s\n", inc); }
}

static int oreg(output *o, function *fp, typetable table)
{
	veach(o->implementations, imp)
	{
		if (fp != imp.fp)
			continue;

		if (same_table(table, imp.table))
			return 0;
	}

	char *name = fp->self.name;
	size_t count = 0;
	veach(o->implementations, imp)
	{
		bug_if_not(imp.name);
		if (0 == strcmp(name, imp.name))
			count += 1;
	}

	if (count) {
		char *tmp = name;
		int written = asprintf(&name, "%s%zu", tmp, count);
		check(written > 0);
	}

	push(o->implementations,
	     (impl){.fp = fp, .table = table, .name = name});
	return 1;
}

static void compile_ast(output *o, typetable *table, flags fl, ast a,
			int indent)
{
	switch (a.tag) {
	case A_STR: {
		todo;
	} break;
	case A_FLOAT: {
		todo;
	} break;
	case A_INT: {
		if (indent)
			odef(o, "%*s", indent, "");

		if (fl & F_RETURN)
			odef(o, "return "), bug_if_not(indent);

		odef(o, "%d", a.integer.value);

		if (indent)
			odef(o, ";\n");
	} break;
	case A_DIAMOND: {
		todo;
	} break;
	case A_KW: {
		todo;
	} break;
	case A_BLOCK: {
		// TODO emit includes?
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
	case A_OPER: {
		todo;
	} break;
	}
}

static ast returning(size_t tindex, ast a)
{
	switch (a.tag) {
	case A_STR: {
		todo;
	} break;
	case A_INT: {
		todo;
	} break;
	case A_FLOAT: {
		todo;
	} break;
	case A_DIAMOND: {
		todo;
	} break;
	case A_KW: {
		todo;
	} break;
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
	case A_OPER: {
		todo;
	} break;
	}
}

static char *compile_type(type t)
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
			return "int16_t";
		case 32:
			return "int32_t";
		case 64:
			return "int64_t";
		default:
			bug("wrong integer bits");
		}
	} break;
	case T_FLOATING: {
		switch (t.integer.bits) {
		case 0:
			return "float";
		case 16:
			todo;
		case 32:
			return "float";
		case 64:
			return "double";
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

	bug("unreachable");
}

static void compile_fn(context *ctx, typetable *table, function *fun)
{
	output *o = ctx->out;

	veach(table->calls, req, i)
	{
		typetable t = new_table(vlen(req.args));
		add_type(&t, get_type(table, req.ret));
		veach(req.args, argi) { add_type(&t, get_type(table, argi)); }
		compile(ctx, &t, req);
	}

	if (fun->flags & FUN_EXTERN)
		return;

	char *ret = compile_type(get_type(table, 0));
	char *name = fun->self.name;

	odec(o, "%s %s(", ret, name);
	odef(o, "%s %s(", ret, name);

	veach(fun->args, arg, i)
	{
		if (i != 0)
			odec(o, ", "), odef(o, ", ");
		char *tname = compile_type(get_type(table, arg.index));
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

void compile(context *ctx, typetable *table, callreq req)
{
	output *o = ctx->out;
	vec(function *) candidates = find_candidates(ctx, table, req.name);

	if (vlen(candidates) > 1) {
		veach(candidates, c)
		{
			dbg("CAN %s %s", c->self.name, show_sig(c->table));
		}
		error("too many candidates: %s %s", req.name, show_sig(*table));
	}

	if (vlen(candidates) == 0)
		error("no candidates: %s", req.name);

	function *f = vget(candidates, 0);

	typetable unitable = clone_table(f->table);

	vloop(unitable.types, ta, 0, 1 + unitable.arity, i) // TODO loopP
	{
		type tb = get_type(table, i);
		*vat(unitable.types, i) = *unify_types(&ta, &tb);
	}

	if (oreg(o, f, unitable)) {
		compile_fn(ctx, &unitable, f);
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

	context upper = (context){
	    .defines = avec(define),
	    .macros = avec(macro),
	};

	output out = (output){
	    .definitions = adeq(char),
	    .declarations = adeq(char),
	    .implementations = avec(impl),
	};

	// context topctx = parse_top(&upper, tops);
	// topctx.out = &out;
	// typetable maintable = new_table();
	// size_t mainret = add_type(&maintable, tint(0));
	// vec(size_t) mainargs = avec(size_t);
	// callreq mainreq =
	//     (callreq){.name = "main", .ret = mainret, .args = mainargs};

	// odec(&out, "#include <stdint.h>\n");
	// emit_includes(&topctx);
	// compile(&topctx, &maintable, mainreq);

	context topctx = parse_ir_top(tops);

	veach(topctx.irfns, f)
	{
		log("%s", f.name);
		dump_ir_full(&f.table, f.body);
	}

	printf("%s", umd_to_cstr(out.declarations));
	printf("%s", umd_to_cstr(out.definitions));

	log("END");
}
