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
#include <string.h>
#include <inttypes.h>
#include "mask.h"

/* the mask size is what regulates the maximum number of items allowed */
/* if you want to increase this number, just increase the mask size by */
/* adding more positions to the array and follow the lead in the functions */
/* for the new index positions */

/* IMPORTANT: */
/* array mask has MASK_T_SIZE positions: 0,1,2,3,4...,MASK_T_SIZE-2,MASK_T_SIZE-1 */
/* but when we want to set bits we set from right to left, so we start at the greter indexes */
/* i.e.: one item in the mask means the bit mask[MASK_T_SIZE-1] = 1 */

mask_t msk_best = {{0}};

mask_t mask_bitwise_or(mask_t* m1, mask_t* m2)
{
	int i = 0;
	/* bitwise or each position of the mask array */
	mask_t mt = {{0}};
	mt.m[0] = m1->m[0] | m2->m[0];
	mt.m[1] = m1->m[1] | m2->m[1];
	mt.m[2] = m1->m[2] | m2->m[2];
	mt.m[3] = m1->m[3] | m2->m[3];
	mt.m[4] = m1->m[4] | m2->m[4];
	mt.m[5] = m1->m[5] | m2->m[5];
	mt.m[6] = m1->m[6] | m2->m[6];
	mt.m[7] = m1->m[7] | m2->m[7];
	mt.m[8] = m1->m[8] | m2->m[8];
	mt.m[9] = m1->m[9] | m2->m[9];
	
	/* get best mask found so far */
	while ((i < MASK_T_SIZE) && (mt.m[i]) >= msk_best.m[i]) i++;
	if (i == MASK_T_SIZE)
		memcpy(msk_best.m, mt.m, sizeof(uint64_t)*MASK_T_SIZE);

	return mt;
}

mask_t mask_bitwise_shift_mask_left(mask_t* ptr, int shift)
{
	/* new mask is initialized with the carryover from the previous index */
	mask_t mt = {{0}};
	mt.m[0] = ptr->m[1] >> (64 - shift);
	mt.m[1] = ptr->m[2] >> (64 - shift);
	mt.m[2] = ptr->m[3] >> (64 - shift);
	mt.m[3] = ptr->m[4] >> (64 - shift);
	mt.m[4] = ptr->m[5] >> (64 - shift);
	mt.m[5] = ptr->m[6] >> (64 - shift);
	mt.m[6] = ptr->m[7] >> (64 - shift);
	mt.m[7] = ptr->m[8] >> (64 - shift);
	mt.m[8] = ptr->m[9] >> (64 - shift);
	
	/* then new mask is added with the shift that remains in the same index */
	mt.m[0] |= ptr->m[0] << shift;
	mt.m[1] |= ptr->m[1] << shift;
	mt.m[2] |= ptr->m[2] << shift;
	mt.m[3] |= ptr->m[3] << shift;
	mt.m[4] |= ptr->m[4] << shift;
	mt.m[5] |= ptr->m[5] << shift;
	mt.m[6] |= ptr->m[6] << shift;
	mt.m[7] |= ptr->m[7] << shift;
	mt.m[8] |= ptr->m[8] << shift;
	mt.m[9] |= ptr->m[9] << shift;
	
	return mt;
}

mask_t mask_bitwise_shift_one_left(int shift)
{
	mask_t mt = {{0}};
	/* find which index of the mask will be the only one affected by the shift by making a integer division */
	int index = (int)(shift/64);
	
	/* new mask is 1LL shifted "shift" times mod 64 */
	mt.m[MASK_T_SIZE-1-index] = 1LL << ((shift > 0)?(shift % 64):0);
	//printf("%i %i\n",MASK_T_SIZE-1-index,shift % 64);
	
	return mt;
}

mask_t mask_bitwise_set_bits(int value)
{
	mask_t mt = {{0}};
	int i = 0;
	int num_indexes = 0;
	
	/* array mask has MASK_T_SIZE positions: 0,1,2,3,4...,MASK_T_SIZE-2,MASK_T_SIZE-1 */
	/* but when we want to set bits we set from right to left, so we start at the greter indexes */
	/* i.e.: one item in the mask means the bit mask[MASK_T_SIZE-1] = 1 */
	
	/* find which index of the mask will be the last affected */
	num_indexes = value/64;
	
	/* from the last position to MASK_T_SIZE-1, set all bits to 1: (2^63)-1 */
	for (i = 0; i < num_indexes; i++)
	{
		mt.m[MASK_T_SIZE-1-i] = (uint64_t)(2ULL << 63)-1;
	}
	
	/* set only the value mod 64 bits on the MASK_T_SIZE-1-num_indexes position */
	mt.m[MASK_T_SIZE-1-num_indexes] = (uint64_t)((2LL << ((value % 64)-1))-1);
	
	return mt;
}

int mask_bitwise_or_compare(mask_t* m1, mask_t* m2, mask_t* m3)
{
	mask_t mt = {{0}};
	
	/* gets a new bitmap with the bitwise or for the two masks (m1 and m2 are "merged") */
	mt = mask_bitwise_or(m1, m2);
	
	/* if the new mask is identical to m3, return true */
	if ((mt.m[0] == m3->m[0]) && 
		(mt.m[1] == m3->m[1]) && 
		(mt.m[2] == m3->m[2]) && 
		(mt.m[3] == m3->m[3]) &&
		(mt.m[4] == m3->m[4]) &&
		(mt.m[5] == m3->m[5]) &&
		(mt.m[6] == m3->m[6]) &&
		(mt.m[7] == m3->m[7]) &&
		(mt.m[8] == m3->m[8]) &&
		(mt.m[9] == m3->m[9])
		)
		return 1;
	else
		return 0;
}

void mask_bitwise_print(mask_t* ptr)
{
	int i;
	
	for (i = 0; i < MASK_T_SIZE; i++)
	{
		printf("%" PRIu64, ptr->m[i]);
	}
	
	printf("\n");
}

void mask_bitwise_print_missing(int num_items)
{
	int i, j;
	mask_t the_mask = mask_bitwise_set_bits(num_items);
	
	/* loop through all array positions and bits */
	for (i = MASK_T_SIZE-1; i >= 0; i--)
	{
		for (j = 0; j < 64; j++)
		{
			/* if objective mask is on and best mask is off, item is missing */
			if ((the_mask.m[i] & (1ULL << j)) && ((~msk_best.m[i]) & (1ULL << j)))
				printf("item %i missing\n", ((MASK_T_SIZE-1-i)*64)+(j));
		}
	}
	
	printf("\n");
}
