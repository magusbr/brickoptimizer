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

#ifndef __INCLUDE_MASK_H__
#define __INCLUDE_MASK_H__
#include <inttypes.h>

#define MASK_T_SIZE	10
typedef struct mask_t
{
	uint64_t m[MASK_T_SIZE];
} mask_t;

mask_t mask_bitwise_or(mask_t* m1, mask_t* m2);
int mask_bitwise_or_compare(mask_t* m1, mask_t* m2, mask_t* m3);
mask_t mask_bitwise_shift_one_left(int shift);
mask_t mask_bitwise_shift_mask_left(mask_t* ptr, int shift);
mask_t mask_bitwise_set_bits(int value);
void mask_bitwise_print(mask_t* ptr);
void mask_bitwise_print_missing(int num_items);


#endif
