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
#ifndef _B_TREE_H
#define _B_TREE_H

#include <stdbool.h>

typedef enum OPType {
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_ASSIGN
} OPType;

typedef enum ValueType {
	IS_OP,
	IS_INT,
	IS_IDEN
} ValueType;

typedef struct BTree BTree;
typedef struct Value {
	ValueType type;
	union {
		OPType op;
		long long int value;
		char* str;
	} u1;
} Value;

typedef struct BTree {
	struct BTree* left;
	struct BTree* right;
	struct Value* value;
} BTree;

BTree* new_empty_node();
Value* value_new_operation(OPType otype);
Value* value_new_number(long long int value);
Value* value_new_variable(const char* v);

#endif


