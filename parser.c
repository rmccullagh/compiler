/*
 *  Copyright (c) 2015 Ryan McCullagh <me@ryanmccullagh.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "parser.h"
#include "lexer.h"
#include "ast.h"
#include "stack.h"

#define D_NAME_SIZE 16

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))



static const char* const keywords[] = {
	"if",
	"while"
};

long long int getNumber(parser_t* self)
{
	int buffer_size = D_NAME_SIZE;
	long long int out = 0;
	int i = 0;
	char* buffer = malloc(D_NAME_SIZE);
	if(buffer == NULL) {
		return 0;
	}
	if(!isdigit((int)self->look)) {
		expected(self, "integer");
	} else {
		while(isdigit((int)self->look)) {
			if(i == buffer_size) {
				buffer_size *= 2;
				buffer = realloc(buffer, buffer_size);
				if(buffer == NULL) {
					return 0;
				}
			}
			buffer[i++] = self->look;
			parser_next(self);
		}
		buffer[i] = '\0';
		out = strtoll(buffer, NULL, 10);
	}
	free(buffer);
	return out;
}

void NORETURN expected(parser_t* self, const char* s)
{
	switch(self->look) {
		case '\t':
			printf("fatal: expecting \"%s\", but got an unexpected \"\\t\" on line %d\n", s, self->line);
		break;
		case '\r':
			printf("fatal: expecting \"%s\", but got an unexpected \"\\r\" on line %d\n", s, self->line);
		break;
		case '\0':
			printf("fatal: expecting \"%s\", but got an unexpected end of file on line %d\n", s, self->line);
        break;
		default:
			printf("fatal: expecting \"%s\", but got an unexpected \"%c\" on line %d\n", s, self->look, self->line);
		break;
	}
	parser_shutdown(self);
	exit(1);
}

int match(parser_t* self, char c)
{
	if(self->look == c) {
		parser_next(self);
		return 1;
	} else {
		char buffer[2];
		int n = snprintf(buffer, sizeof(buffer), "%c", c);
		if(n != (sizeof(buffer) -1)) {
			printf("fatal: snprintf failed\n");
			parser_shutdown(self);
		}
		expected(self, buffer);
	}
}


static void parse_factor(parser_t* self, BTree* ast);
static BTree* make_binary_op(OPType op, BTree* left, BTree* right);
static void parse_term(parser_t* self, BTree* ast);

/*
 * {Expression  Term {+Term} }
 */
static void parse_expression(parser_t* self, BTree* ast);

static bool parser_is_keyword(char* name) 
{
	size_t i;
	for(i = 0; i < ARRAY_SIZE(keywords); i++) {
		if(strcmp(name, keywords[i]) == 0) {
			return true;
		}
	}	
	
	return false;

}

int parser_init(parser_t* self, char* text)
{
	self->text = strdup(text);
	if(self->text == NULL) {
		return 1;
	}
	self->pos = 0;
	self->len = strlen(text);
	self->look = EOF;
	self->status = 0;
	self->line = 1;
	self->current_token = T_ILLEGAL;
	self->last_token = T_ILLEGAL;
	self->ast = NULL;
	stack_init(&self->stack);
	return 0;
}

void parser_next(parser_t* self)
{
	if(self->pos >= self->len) {
		self->status = EOF;
		self->look = '\0';
	} else {
		self->look = self->text[self->pos++];
		if(!((self->look >= 20) && (self->look <= 126))) {
			if(self->look == '\r' || self->look == '\n' || self->look == '\t') {
				return;
			}
			printf("error: unexpected illegal character on line %d\n", self->line);
			parser_shutdown(self);
			exit(1);
		}
	}
}

static inline void parser_syntax_error(parser_t* self, const char* s)
{
	printf("syntax error: %s\n", s);
	parser_shutdown(self);
	exit(1);
}

/*
 * <ident> ::= <letter> [ <letter> | <digit> ]*
 */
static char* parser_parse_name(parser_t* self)
{
	int buffer_size = D_NAME_SIZE;
	int i = 0;
	char* buffer = malloc(D_NAME_SIZE);
	if(buffer == NULL) {
		return NULL;
	}
	if(!isalpha((int)self->look)) {
		expected(self, "identifier");
	} 
	while(isalnum((int)self->look)) {
		if(i == buffer_size) {
			buffer_size *= 2;
			buffer = realloc(buffer, buffer_size);
			if(buffer == NULL) {
				return NULL;
			}
		}
		buffer[i++] = self->look;
		parser_next(self);
	}
	buffer[i] = '\0';
	return buffer;
}

/*
 * <number ::= [<digit>]+
 */
static inline void parser_parse_number(parser_t* self)
{
	while(isdigit((int)self->look)) {
		parser_next(self);
	}
}

static inline void parser_skip_comma(parser_t* self)
{
	/* skip_white(); */
	if(self->look == ',') {
		parser_next(self);
		/* skip_white */
	}
}

/*
 * {Assignment  Variable = Expression}
 *  <Ident> = <Expression>
 */

static BTree* make_const_value(parser_t* self)
{
	BTree* node = new_empty_node();
	Value* value = value_new_number(getNumber(self));
	node->left = NULL;
	node->right = NULL;
	node->value = value;
	return node;
}

static BTree* make_const_identifer(parser_t* self)
{
	BTree* node = new_empty_node();
	Value* value = value_new_variable(parser_parse_name(self));
	node->left = NULL;
	node->right = NULL;
	node->value = value;
	return node;
}

static BTree* do_parse_factor(parser_t* self)
{
	if(self->look == '(') {
		match(self, '(');
		BTree* exp = do_expression(self);
		match(self, ')');
		return exp;
	} else if(isdigit((int)self->look)) {
		return make_const_value(self);
	} else {
		return make_const_identifer(self);
	}	
}

static BTree* do_parse_term(parser_t* self)
{
	stack_push(&self->stack, do_parse_factor(self));
	while(self->look == '*' || self->look == '/') {
		switch(self->look) {
			case '*':
				match(self, '*');
				stack_push(&self->stack, make_binary_op(OP_MUL, stack_pop(&self->stack), do_parse_factor(self)));	
			break;
			case '/':
				match(self, '/');
				stack_push(&self->stack, make_binary_op(OP_DIV, stack_pop(&self->stack), do_parse_factor(self)));	
			break;
			default:
				parser_syntax_error(self, "invalid expression");
			break;
		}
	}
	
	return stack_pop(&self->stack);
	
}

static BTree* make_binary_op(OPType op, BTree* left, BTree* right)
{
	BTree* node = new_empty_node();
	Value* v = value_new_operation(op);
	node->left = left;
	node->right = right;
	node->value = v;
	return node;
}

BTree* do_assignment(parser_t* self)
{
	stack_push(&self->stack, make_const_identifer(self));
	while(self->look == '=') {
		switch(self->look) {
			case '=':
				match(self, '=');
				stack_push(&self->stack, make_binary_op(OP_ASSIGN, stack_pop(&self->stack), do_expression(self)));		
			break;
			default:
				parser_syntax_error(self, "invalid expression");
			break;
		}
	}
	
	return stack_pop(&self->stack);
}

BTree* do_expression(parser_t* self)
{
	stack_push(&self->stack, do_parse_term(self));
	while(self->look == '+' || self->look == '-') {
		switch(self->look) {
			case '+':
				match(self, '+');
				stack_push(&self->stack, make_binary_op(OP_ADD, stack_pop(&self->stack), do_parse_term(self)));				
			break;
			case '-':
				match(self, '-');
				stack_push(&self->stack, make_binary_op(OP_SUB, stack_pop(&self->stack), do_parse_term(self)));				
			break;
			default:
				parser_syntax_error(self, "invalid expression");
			break;
		}
	}
	
	return stack_pop(&self->stack);
}

void parse_assignment(parser_t* self)
{

	BTree* node = new_empty_node();
	Value* value = value_new_variable(parser_parse_name(self));
	node->left = NULL;
	node->right = NULL;
	node->value = value;
	self->ast = node;
	//stack_push(&self->stack, node);
	match(self, '=');
	
	BTree* n = new_empty_node();
	Value* v = value_new_operation(OP_ASSIGN);
	n->value = v;
	n->left = self->ast;	
	//n->left = node;
	//n->left = stack_pop(&self->stack);
	parse_expression(self, self->ast->right);
	n->right = stack_pop(&self->stack);
	self->ast = n;	
	
	
	/*Token t = lexer_lex(self);
	if(t == T_IDEN) {
		BTree* node = new_empty_node();
		Value* value = value_new_variable(parser_parse_name(self));
		node->left = NULL;
		node->right = NULL;
		node->value = value;
		self->ast = node;
		//stack_push(&self->stack, node);
		match(self, '=');
		BTree* n = new_empty_node();
		Value* v = value_new_operation(OP_ASSIGN);
		n->value = v;
		n->left = self->ast;
		
		parse_expression(self, self->ast->right);
		n->right = stack_pop(&self->stack);
		self->ast = n;	
	} else {
		expected(self, "T_IDEN");
		//parser_syntax_error(self, "invalid left hand assignment");
	}*/
}



/*
 * factor = ident | number | "(" expression ")"
 */
static void parse_factor(parser_t* self, BTree* ast)
{
	Token t = lexer_lex(self);
	if(t == T_OPAREN) {
		match(self, '(');
		parse_expression(self, ast);
		match(self, ')');
	} else if(t == T_NUM) {
		BTree* node = new_empty_node();
		Value* value = value_new_number(getNumber(self));
		node->left = NULL;
		node->right = NULL;
		node->value = value;
		stack_push(&self->stack, node);
	} else {
		BTree* node = new_empty_node();
		Value* value = value_new_variable(parser_parse_name(self));
		node->left = NULL;
		node->right = NULL;
		node->value = value;
		stack_push(&self->stack, node);
	}
}

static void parse_term(parser_t* self, BTree* ast)
{
	parse_factor(self, ast);
	while(self->look == '*') {
		match(self, '*');
		BTree* o1 = stack_pop(&self->stack);
		BTree* node = new_empty_node();
		Value* value = value_new_operation(OP_MUL);
		node->value = value;
		node->left = o1;
		
		parse_factor(self, self->ast->right);
		
		BTree* o2 = stack_pop(&self->stack);
		node->right = o2;
		stack_push(&self->stack, node);
	}
}

/*
 * {Expression  Term {+Term} }
 */
static void parse_expression(parser_t* self, BTree* ast)
{
	parse_term(self, ast);
	while(self->look == '+') {
		match(self, '+');
		BTree* node = new_empty_node();
		Value* value = value_new_operation(OP_ADD);
		node->value = value;
		node->left  = stack_pop(&self->stack);
        parse_term(self, self->ast->right);
		node->right = stack_pop(&self->stack);
		stack_push(&self->stack, node);
	}
}

void parser_parse(parser_t* self)
{

}

void parser_shutdown(parser_t* self)
{
	#ifdef DEBUG
		fprintf(stderr, "*** debug info ***\n");
		fprintf(stderr, "total lines   : %d\n", self->line);
		fprintf(stderr, "source length : %zu\n", self->len);
		fprintf(stderr, "last position : %zu\n", self->pos);
		fprintf(stderr, "last status   : %d\n", self->status);
		fprintf(stderr, "*** end debug info ***\n");
	#endif
	free(self->text);
	free(self);
}

