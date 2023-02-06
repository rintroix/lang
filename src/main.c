#include <stdio.h>
#include <assert.h>

#define TB_TRACE_MODULE_NAME "impl"
#define TB_TRACE_MODULE_DEBUG (1)
#include "tbox/tbox.h"

#include "common.h"

token* make_token(char *name) {
	token *new = tb_malloc0(sizeof(token));
	*new = (token){ .type = TOKEN_ATOM, .name = name };
	tb_trace_d("new token %d %s %p", new->type, new->name, new);
	return new;
}

void push_atom(head *head, char *name) {
	token *new = tb_malloc0(sizeof(token));
	*new = (token){ .type = TOKEN_ATOM, .name = name };
	new->next = head->token;
	head->token = new;
}

void push_open(head *head) {
	token *new = tb_malloc0(sizeof(token));
	*new = (token){ .type = TOKEN_OPEN };
	new->next = head->token;
	head->token = new;
}

void push_dot(head *head) {
	token *new = tb_malloc0(sizeof(token));
	*new = (token){ .type = TOKEN_DOT };
	new->next = head->token;
	head->token = new;
}

void push_list(head *head, token *children) {
	token *new = tb_malloc0(sizeof(token));
	*new = (token){ .type = TOKEN_LIST, .children = children };
	new->next = head->token;
	head->token = new;
}

void collect_list(head *head) {
	assert(head->token);
	token *children = 0;
	while (head->token->type != TOKEN_OPEN) {
		if (head->token->type == TOKEN_DOT) {
			head->token = head->token->next;
			push_list(head, children);
			children = 0;
		} else {
			token *tmp = head->token->next;
			head->token->next = children;
			children = head->token;
			head->token = tmp;
		}
	}
	head->token = head->token->next;

	push_list(head, children);
}

void print_token(token *token) {
	switch(token->type) {
	case TOKEN_LIST:
		printf("(");
		struct token *child = token->children;
		while (child) {
			print_token(child);
			child = child->next;
			if (child)
				printf(" ");
		}
		printf(")");
		break;

	case TOKEN_ATOM:
		printf("%s", token->name);
		break;

	default:
		assert(2 == 3);
		break;
	}
}

void token_free(tb_element_ref_t element, tb_pointer_t data) {
	(void)element;
	token *t = *(token**)data;
	// printf("freeing token %d: %s: %p\n", t->type, t->name, t);
	tb_free(t);
}

void start() {
	int i = tb_init(tb_null, tb_null);
	tb_trace_d("init: %d", i);

	tb_element_t token_element = tb_element_ptr(token_free, 0);
	tb_stack_ref_t stack = tb_stack_init(128, token_element);

	tb_trace_d("Stack %p", stack);
	tb_stack_put(stack, (tb_cpointer_t)make_token("a"));
	tb_stack_put(stack, (tb_cpointer_t)make_token("b"));
	tb_stack_put(stack, (tb_cpointer_t)make_token("c"));

	tb_assert(tb_stack_size(stack) == 3);
	tb_stack_clear(stack);
	tb_stack_exit(stack);
}

void finish(head *head) {
	while (head->token) {
		assert(head->token->type == TOKEN_LIST);
		print_token(head->token);
		puts("");
		head->token = head->token->next;	
	}
	tb_trace_d("END");
	tb_exit();
}

#include "parser.h"

int main() {
	start();
	head head = {0};
	pcc_context_t *ctx = pcc_create(&head);
	pcc_parse(ctx, NULL);
	pcc_destroy(ctx);
	finish(&head);
}
