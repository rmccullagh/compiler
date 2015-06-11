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
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "stack.h"

void stack_init(Stack* self)
{
	self->sp = 0;
	size_t i;
	for(i = 0; i < STACK_SIZE; i++) {
		self->table[i] = NULL;
	}
}

BTree* stack_pop(Stack* self)
{
	if(self->sp == 0) {
		return self->table[0];
	} else if(self->sp > 0) {
		// TODO test this, valid index ranges, are 0...255. self->sp's max value is 256
		return self->table[--self->sp];
	} else {
		printf("error: stack underflow\n");
		exit(1);
	}
}

void stack_push(Stack* self, BTree* node)
{
	// wrap if neccessary
	self->table[((unsigned char)self->sp++)] = node;
	
}
