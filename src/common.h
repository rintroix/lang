#ifndef COMMON_H_
#define COMMON_H_

enum token_type { TOKEN_OPEN, TOKEN_ATOM, TOKEN_LIST, TOKEN_DOT, TOKEN_NODE };
enum node_type { NODE_LIST, NODE_ATOM };
enum ast_type { AST_FN, AST_CALL, AST_REF, AST_DEF };

typedef struct token {
	enum token_type type;
	char *name;
	struct token *children;
	struct token *next;
} token;

typedef struct head {
	struct token *token;
} head;

void push_atom(struct head *head, char *name);
void push_open(struct head *head);
void push_dot(struct head *head);
void collect_list(struct head *head);

#endif // COMMON_H_
