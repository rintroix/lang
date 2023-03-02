#ifndef COMMON_H_
#define COMMON_H_

#include <stdlib.h>

#include "data.h"

#define log(...) _log(__VA_ARGS__, "\n")
#define _log(FMT, ...)                                                         \
	printf("%s:%d %s: " FMT "%s", __FILE__, __LINE__, __func__, __VA_ARGS__)

#define _elog(CODE, ...)                                                       \
	do {                                                                   \
		log(__VA_ARGS__);                                              \
		exit(CODE);                                                    \
	} while (0)

#define todo _elog(2, "TODO")
#define bug(...) _elog(127, "compiler bug: " __VA_ARGS__);
#define error(...) _elog(1, "error: " __VA_ARGS__);
#define check(...)                                                             \
	do {                                                                   \
		if (!(__VA_ARGS__))                                            \
			_elog(1, "check failed: %s", #__VA_ARGS__);            \
	} while (0)

#ifdef DEBUG
#  define dbg(...) log("debug: " __VA_ARGS__)
#else
#  define dbg(...)
#endif

typedef struct define {
	char *name;
	char *type;
	struct ast *init;
} define;

enum ast_type {
	A_FN = 1,
	A_LIST,
	A_CALL,
	A_ID,
	A_OPER,
	A_BLOCK,
	A_MARK
};

enum id_type { I_WORD, I_KW, I_OP };


typedef struct ast ast;

typedef struct block {
	vec(define) defs;
	vec(ast) items;
} block;

struct ast {
	enum ast_type type;
	union {
		struct {
			define def;
			vec(define) args;
		} fn;

		struct {
			vec(ast) items;
		} list;

		struct {
			char *name;
			vec(ast) args;
		} call;

		struct {
			char *name;
			enum id_type type;
		} id;

		struct {
			char *name;
			struct ast *left;
			struct ast *right;
		} oper;

		struct block block;
	};
};

#define op(NAME) ((ast){.type = A_ID, .id = {.name = (NAME), .type = I_OP}})

#define kw(NAME) ((ast){.type = A_ID, .id = {.name = (NAME), .type = I_KW}})

#define word(NAME) ((ast){.type = A_ID, .id = {.name = (NAME), .type = I_WORD}})

#define ablock(BLOCK) ((ast){.type = A_BLOCK, .block = (BLOCK)})

#define list(ITEMS) ((ast){.type = A_LIST, .list = {.items = (ITEMS)}})

#define call(NAME, ARGS)                                                       \
	((ast){.type = A_CALL, .call = {.name = (NAME), .args = (ARGS)}})

#define oper(NAME, L, R)                                                       \
	((ast){.type = A_OPER,                                                 \
	       .oper = {.name = (NAME), .left = (L), .right = (R)}})

#define fn(DEF, ARGS)                                                          \
	((ast){.type = A_FN, .fn = {.def = (DEF), .args = (ARGS)}})

#define def(NAME, TYPE, INIT)                                                  \
	((define){.name = NAME, .type = (TYPE), .init = (INIT)})

ast dot(ast l, ast r);
ast list0();
ast list1(ast a);
ast append(ast l, ast a);
void printl_ast(ast *t);

#define W(x) &word(x)
#define L(x) &list(x)
#define K(x) &kw(x)
#define O(x) &op(x)
#define F(x) &fn(def(x, 0, 0))

typedef struct macro {
	char *name;
} macro;

typedef struct scope {
	vec(ast) functions;
	struct scope *next;
} scope;

typedef struct candidate {
	ast *ast;
	vec(struct rule) rules;
} candidate;

#define can(AST, RULES) ((candidate){.ast = (AST), .rules = (RULES)})

typedef struct request {
	struct scope *scope;
	vec(struct rule) args;
	vec(candidate) candidates;
} request;

enum e_rule {
	R_EMPTY = 1,
	R_IS,
	R_CALL,
};

typedef struct rule {
	enum e_rule type;
	union {
		struct {
			char *name;
		} is;
	};
} rule;

#endif // COMMON_H_
