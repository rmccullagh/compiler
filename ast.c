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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ast.h"

BTree* new_empty_node()
{
	BTree* node = malloc(sizeof(BTree));
	return node;
}

Value* value_new_operation(OPType otype)
{
	Value* value = malloc(sizeof(Value));
	if(!value) {
		return NULL;
	}
	
	value->type = IS_OP;
	value->u1.op = otype;
	return value;
}

Value* value_new_variable(const char* v)
{
	Value* value = malloc(sizeof(Value));
	if(!value) {
		return NULL;
	}	
	value->type = IS_IDEN;
	value->u1.str = strdup(v);
	return value;
}

Value* value_new_number(long long int v)
{
	Value* value = malloc(sizeof(Value));
	if(!value) {
		return NULL;
	}	
	value->type = IS_INT;
	value->u1.value = v;
	return value;
}




