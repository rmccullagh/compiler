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
#include <stdio.h>

#include "ast.h"

void process_node(Value* node);

//Infix notation: X + Y
//An infix expression is produced by the inorder traversal
void inorder(BTree* node)
{
	if(node == NULL) {
		return;
	}
	if(node->value->type == IS_OP) {
		printf("(");	
	}
	inorder(node->left);
	process_node(node->value);	
	inorder(node->right);
	if(node->value->type == IS_OP) {
		printf(")");	
	}
	
}

void preorder(BTree* node)
{
	if(node == NULL) {
		return;
	}

	process_node(node->value);		
	if(node->value->type == IS_OP) {
		printf("(");	
	}
	preorder(node->left);
	if(node->value->type == IS_OP) {
		printf(")");	
	}
	if(node->value->type == IS_OP) {
		printf("(");	
	}
	preorder(node->right);
	if(node->value->type == IS_OP) {
		printf(")");	
	}
}


long long int eval(BTree* node)
{
	if(node->left == NULL && node->right == NULL) {
		if(node->value->type == IS_INT) {
			return node->value->u1.value;
		} else if(node->value->type == IS_IDEN) {
			// look up value in hash table
			printf("warning: undefined variable '%s'\n", node->value->u1.str);
			return 0;
		} else {
			printf("fatal: invalid left hand value\n");
			exit(1);
		}
	} else {
		long long int temp1;
		long long int temp2;
		switch(node->value->type) {
			case IS_OP:
				switch(node->value->u1.op) {
					case OP_ADD:
						return eval(node->left) + eval(node->right);
					break;
					case OP_SUB:
						return eval(node->left) - eval(node->right);
					break;
					case OP_MUL:
						return eval(node->left) * eval(node->right);
					break;
					case OP_DIV:
						temp1 = eval(node->left);
						temp2 = eval(node->right);
						if(temp2 == 0) {
							printf("fatal: cannot divide by 0\n");
							exit(1);
						}
						return temp1 / temp2;
					break;
					case OP_ASSIGN:
						return eval(node->right);
					break;
					default:
						printf("invalid operation\n");
						return 0;
					break;	
				}
			break;
			default:
				printf("invalid operation\n");
				return 0;
			break;	
		}
	}
	return 0;
}

void process_node(Value* value)
{
	if(value == NULL) {
		return;
	}
	switch(value->type) {
		case IS_OP:
			switch(value->u1.op) {
				case OP_ADD:
					printf("+");
				break;
				case OP_SUB:
					printf("-");
				break;
				case OP_MUL:
					printf("*");
				break;
				case OP_DIV:
					printf("/");
				break;
				case OP_ASSIGN:
					printf("=");
				break;
				default:
					printf("invalid operation\n");
				break;
			}
		break;
		case IS_INT:
			printf("%lli", value->u1.value);
		break;
		case IS_IDEN:
			printf("%s", value->u1.str);
		break;
		default:
			printf("invalid value type\n");
		break;
	}
}


#include "parser.h"

int main(int argc, char** argv)
{
	if(argc != 2) {
		return 1;
	}
	parser_t* parser = malloc(sizeof(parser_t));
	if(!parser) {
		return 1;
	}
	int status = parser_init(parser, argv[1]);
	if(status == 1) {
		free(parser);
		return 1;
	}

	parser_next(parser);
	while(parser->status != EOF) {
		BTree* node = do_assignment(parser);
		printf("\n** pre order **\n");
		preorder(node);
		printf("\n** end **\n");
		printf("\n%lli\n", eval(node));
	}

	parser_shutdown(parser);
	return 0;
}
