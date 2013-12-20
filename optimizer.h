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

#ifndef __INCLUDE_OPTIMIZER_H__
#define __INCLUDE_OPTIMIZER_H__
#include "mask.h"

typedef struct store_t
{
	long id;
	float* items;
	long* store_item_id;
	mask_t items_mask;
	int store_num;
	int num_items;
	
	struct store_t* next;
} store_t;

int store_init(int num_items);
int store_add(long id);
int store_end();
int store_add_item(long store_id, int item_index, long store_item_id, float item_price);
int store_num();
int store_best_match (int fixed_index, mask_t match, store_t* cur_store);


#endif