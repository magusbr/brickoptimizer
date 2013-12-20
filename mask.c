/*
Brickoptimizer - optimize the price of Bricklinks Wanted List

   Copyright (C) 2013  Gustavo Miranda Teixeira (gustavo@ice.ufjf.br)
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version provided this disclaimer remains
   unchanged.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "mask.h"

mask_t mask_bitwise_or(mask_t* m1, mask_t* m2)
{
	mask_t mt = {{0}};
	mt.m[0] = m1->m[0] | m2->m[0];
	mt.m[1] = m1->m[1] | m2->m[1];
	mt.m[2] = m1->m[2] | m2->m[2];
	mt.m[3] = m1->m[3] | m2->m[3];
	return mt;
}

mask_t mask_bitwise_shift_mask_left(mask_t* ptr, int shift)
{
	mask_t mt = {{0}};
	mt.m[0] = ptr->m[1] >> (64 - shift);
	mt.m[1] = ptr->m[2] >> (64 - shift);
	mt.m[2] = ptr->m[3] >> (64 - shift);
	
	mt.m[0] |= ptr->m[0] << shift;
	mt.m[1] |= ptr->m[1] << shift;
	mt.m[2] |= ptr->m[2] << shift;
	mt.m[3] |= ptr->m[3] << shift;
	
	return mt;
}

mask_t mask_bitwise_shift_one_left(int shift)
{
	mask_t mt = {{0}};
	
	int index = (int)(shift/64);
	mt.m[MASK_T_SIZE-1-index] = 1LL << ((shift > 0)?(shift % 64):0);
	//printf("%i %i\n",MASK_T_SIZE-1-index,shift % 64);
	
	return mt;
}

mask_t mask_bitwise_set_bits(int value)
{
	mask_t mt = {{0}};
	int i = 0;
	int num_indexes = 0;
	
	num_indexes = value/64;
	
	for (i = 0; i < num_indexes; i++)
	{
		mt.m[MASK_T_SIZE-1-i] = (uint64_t)(2ULL << 63)-1;
	}
	
	mt.m[MASK_T_SIZE-1-num_indexes] = (uint64_t)((2LL << ((value % 64)-1))-1);

	//for (i = 0; i<64; i++)
	//	printf("numindex %i - %i %" PRIu64 "\n", num_indexes, i, mt.m[2] & (1LL << i));
	
	return mt;
}

int mask_bitwise_or_compare(mask_t* m1, mask_t* m2, mask_t* m3)
{
	mask_t mt = {{0}};
	
	mt = mask_bitwise_or(m1, m2);
	
	if ((mt.m[0] == m3->m[0]) && (mt.m[1] == m3->m[1]) && (mt.m[2] == m3->m[2]) && (mt.m[3] == m3->m[3]))
		return 1;
	else
		return 0;
}

void mask_bitwise_print(mask_t* ptr)
{
	printf("%" PRIu64 "%" PRIu64 "%" PRIu64 "%" PRIu64 "\n", ptr->m[0], ptr->m[1], ptr->m[2], ptr->m[3]);
}