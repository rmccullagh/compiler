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

#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexer.h"
#include "ast.h"
#include "stack.h"

#define NORETURN __attribute__ ((noreturn))

typedef struct parser_t {
	char* text;
	size_t pos;
	size_t len;
	char look;
	int status;
	int line;
	Token last_token;
	Token current_token;
	BTree* ast;
	Stack stack;
} parser_t;

BTree* do_assignment(parser_t* self);
BTree* do_expression(parser_t* self);
void NORETURN expected(parser_t* self, const char* s);
int  parser_init(parser_t*, char*);
void parser_next(parser_t*);
void parse_assignment(parser_t* self);
void parser_parse(parser_t*); 
void parser_shutdown(parser_t*);

#endif /* __PARSER_H__ */


