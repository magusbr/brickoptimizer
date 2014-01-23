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
#include <tidy.h>
#include <buffio.h>
#include <curl/curl.h>
#include <math.h>
#include <float.h>
#include <inttypes.h>
#include "parser.h"
#include "optimizer.h"
#include "mask.h"
#include "form_generator.h"

#define BIT_ARRAY_SIZE	16
#define BIT_ARRAY_BITS	(sizeof(unsigned long)*BIT_ARRAY_SIZE)

store_t* store = NULL;
store_t* store_last = NULL;
store_t* ordered_store = NULL;

int store_count = 0;
int item_count = 0;

store_t** match_last = NULL;
store_t** match_best = NULL;
float match_price = 0;
mask_t the_mask = {{0}};


int store_init(int num_items)
{
	item_count = num_items;
	//get the mask that we are looking for
	the_mask = mask_bitwise_set_bits(num_items);
	
	return RC_OK;
}

store_t* store_find(long id)
{
	store_t* tmp_store = store;
	
	while (tmp_store && (tmp_store->id != id)) tmp_store = tmp_store->next;
	
	if (tmp_store) return tmp_store;
	else return RC_NOT_FOUND;
}


int store_add(long id)
{
	store_t* tmp_store;
	
	if (store_find(id))
	{
		return RC_EXISTS;
	}
	
	tmp_store = (store_t*)calloc(1,sizeof(store_t));
	if (!tmp_store) return RC_ERR_MALLOC;
	tmp_store->items = (float*)calloc(item_count,sizeof(float));
	if (!tmp_store->items) return RC_ERR_MALLOC;
	tmp_store->store_item_id = (long*)calloc(item_count,sizeof(long));
	if (!tmp_store->store_item_id) return RC_ERR_MALLOC;
	
	tmp_store->id = id;
	memset(&(tmp_store->items_mask), 0, sizeof(mask_t));
	tmp_store->next = NULL;
	tmp_store->store_num = store_count + 1;
	
	if (!store)
	{
		store = tmp_store;
		store_last = store;
	}
	else
	{
		store_last->next = tmp_store;
		store_last = tmp_store;
	}
	
	store_count++;
	
	return RC_OK;
}

int store_free(int id)
{
	store_t* tmp_store;
	tmp_store = store_find(id);
	
	if (!tmp_store) return RC_NOT_FOUND;
	
	if (tmp_store->id == store_last->id)
	{
		store_t* tmp_store_last = store;
		while (tmp_store_last && tmp_store_last->next && (tmp_store_last->next->id != id))
			tmp_store_last = tmp_store_last->next;
		if (tmp_store_last && tmp_store_last->next)
			store_last = tmp_store_last;
	}
	
	free(tmp_store->items);
	tmp_store->items = NULL;
	
	free(tmp_store->store_item_id);
	tmp_store->store_item_id = NULL;
	
	free(tmp_store);
	tmp_store = NULL;
	
	store_count--;
	
	return RC_OK;
}

int store_end()
{
	store_t* tmp_store = store;
	
	if (!store) return RC_OK;
	
	store = store->next;
	
	while ((store_free(tmp_store->id) == RC_OK) && store)
	{
		tmp_store = store;
		store = store->next;
	}
	
	return RC_OK;
}

int store_add_item(long store_id, int item_index, long store_item_id, float item_price)
{
	store_t* tmp_store = NULL;
	mask_t tmp_mask = {{0}};
	
	if (item_index >= item_count) return RC_NOT_FOUND;
	
	tmp_store = store_find(store_id);
	if (!tmp_store) return RC_NOT_FOUND;
	
	//since Bricklink offers prices ordered, get only the least price for new items
	//bear in mind that it still consider used items, but only if no new item was found in this store
	if (tmp_store->items[item_index]) return RC_OK;
	
	tmp_store->items[item_index] = item_price;
	tmp_store->store_item_id[item_index] = store_item_id;
	//tmp_store->items_mask = tmp_store->items_mask | (1 << item_index);
	tmp_mask = mask_bitwise_shift_one_left(item_index);
	tmp_store->items_mask = mask_bitwise_or(&tmp_mask,&(tmp_store->items_mask));
	tmp_store->num_items++;
	
	return RC_OK;
}

int store_sum_item (store_t** match_last, int num_fixed)
{
	int i;
	int sum = 0;
	
	for (i = 0; i < num_fixed; i++)
		sum += match_last[i]->num_items;
		
	return sum;
}

store_t* store_reorder_merge (store_t* list1, store_t* list2)
{
	store_t* store;
	store_t* store_next;
	
	//we start with the biggest element in either list
	//then we advance this very list
	if (list1->num_items > list2->num_items)
	{
		store = list1;
		list1 = list1->next;
	}
	else
	{
		store = list2;
		list2 = list2->next;
	}
	
	//now we can advance both lists at the same time
	//each time the greater element between them is added to the final list
	store_next = store;
	while (list1 && list2)
	{
		if (list1->num_items > list2->num_items)
		{
			store_next->next = list1;
			list1 = list1->next;
		}
		else
		{
			store_next->next = list2;
			list2 = list2->next;
		}
		
		//our final list need to be advanced too
		//but only when the next element exist
		if (store_next->next) store_next = store_next->next;
	}
	
	//if one of the lists has come to an end we just add the other to our final list
	if (list1) store_next->next = list1;
	else if (list2) store_next->next = list2;
	
	return store;
}

store_t* store_reorder_split(store_t* store_ptr)
{
	store_t* slow = store_ptr;
	store_t* fast = store_ptr;

	//single element detected - return!
	if (!fast->next) return fast;
	
	while (fast)
	{
		//advance fast twice as fast as slow to break list exactly into half
		fast = fast->next;
		if (fast && fast->next)
		{
			fast = fast->next;
			slow = slow->next;
		}
		
		//slow hasn't advanced and fast could only advance one position: we have only two elements!
		//compare them between themselves and reorder them if necessary
		else if ((slow == store_ptr) && (slow->next == fast))
		{
			//DBGPRINT("store %ix%i items %ix%i\n", fast->store_num, slow->store_num, fast->num_items, slow->num_items);
			if (slow->num_items < fast->num_items)
			{
				fast->next = slow;
				slow->next = NULL;
				slow = fast;
			}
			return slow;
		}
	}
	
	//break the continuity of the list so that "fast" becomes the last element of first leg
	//and slow becomes the first element of second leg
	fast = slow;
	slow = slow->next;
	fast->next = NULL;
	
	//DBGPRINT("split: %i %i\n", store_ptr->store_num, slow->store_num);
	
	//send second leg to be splitted and first leg (first element of all) to be splitted
	fast = store_reorder_split(slow);
	slow = store_reorder_split(store_ptr);
	
	//the two legs were ordered, now the need to be ordered between themselves
	slow = store_reorder_merge(fast, slow);
	
	return slow;
}


int store_num()
{
	return store_count;
}

void store_print_best_match(store_t** match_best, int num_stores)
{
	int i, j;
	int index_best_store = 0;
	float less;
	form_generator_t** fm = NULL;
	
	fm = (form_generator_t**)malloc(sizeof(form_generator_t*)*num_stores);

	for (i = 0; i < num_stores; i++)
	{
		printf("http://www.bricklink.com/store.asp?sID=%li\n", match_best[i]->id);
		fm[i] = (form_generator_t*)malloc(sizeof(form_generator_t));
		form_generator_init(fm[i], item_count, match_best[i]->id);
	}
		
	
	printf("\nIndividual links to items in their best price store:\n");
	//for each item we have, we loop the list of matched stores
	//to get the best individual price on our match
	for (j = 0; j < item_count; j++)
	{
		less = FLT_MAX ;
		for (i = 0; i < num_stores; i++)
		{
			if (match_best[i]->items[j] < 0.00001) continue;
			if (match_best[i]->items[j] < less)
			{
				less = match_best[i]->items[j];
				index_best_store = i;
			}
		}
		
		form_generator_add_item(fm[index_best_store], j, match_best[index_best_store]->store_item_id[j]);
	}

	for (i = 0; i < num_stores; i++)
	{
		form_generator_end(fm[i]);
		free(fm[i]);
	}
	free(fm);
}


float store_best_price (int match_count)
{
	int i,j;
	float less;
	float last_price = 0;
	
	//for each item we have, we loop the list of matched stores
	//to get the best individual price on our match
	for (j = 0; j < item_count; j++)
	{
		less = FLT_MAX ;
		for (i = 0; i < match_count; i++)
		{
			if (match_last[i]->items[j] < 0.00001) continue;
			if (match_last[i]->items[j] < less)
				less = match_last[i]->items[j];
		}
		last_price += less;
	}
	
	return last_price;
}

int store_best_match (int fixed_index, mask_t match, store_t* cur_store)
{
	static char indent[100000] = "";
	static char* ptr_indent = &indent[0];
	static int num_fixed = 1;
	static float best_price = 0;
	static int num_fixed_items = 0;
	float last_price;
	
	//reinitialize variables on the first call
	if (fixed_index == 0)
	{
		//reorder list of stores starting with the store with more items in stock
		store = store_reorder_split(store);
		//store_t* tmp = store; while (tmp) { printf("%li %i\n",tmp->id, tmp->num_items); tmp=tmp->next; }

		cur_store = store;
		num_fixed = 1;
		best_price = 0;
		match_last = (store_t**)calloc(store_num(),sizeof(store_t*));
		match_best = (store_t**)calloc(store_num(),sizeof(store_t*));
	}
	
	//we have a fixed number of items. do they together with the variable item sum up to the wanted items?
	num_fixed_items = store_sum_item(match_last, num_fixed-1);
	
	do
	{
		//call for a invalid store return
		if (!cur_store) return RC_NOT_FOUND;
	
		//keep current stores to compare prices when all items are found
		match_last[fixed_index] = cur_store;
		
		//if there are still more positions to be fixed
		if (fixed_index < num_fixed-1)
		{
			//DBGPRINT("%sgo deeper %i\n",ptr_indent, fixed_index);
			//go deeper into recursion to try and match remaining items
			strcat(ptr_indent, "  ");
			if (store_best_match(fixed_index+1, mask_bitwise_or(&match,&cur_store->items_mask), cur_store->next) == RC_BEST_MATCH)
				return RC_BEST_MATCH;
			ptr_indent[strlen(ptr_indent)-2] = 0;
			
			//we have a fixed number of items. do they together with the variable item sum up to the wanted items?
			num_fixed_items = store_sum_item(match_last, num_fixed-1);
		}
		//else DBGPRINT("%snum_fixed=%i - fixed[0]=%i - fixed[1]=%i - store num=%i\n",ptr_indent, num_fixed, (match_last[0]?match_last[0]->store_num:-1), (match_last[1]?match_last[1]->store_num:-1), cur_store->store_num);
		
		//not possible to get all the items with this combo
		if (fixed_index && (num_fixed_items + cur_store->num_items < item_count))
		{
			//DBGPRINT("num_fixed-1=%i: num_fixed_items=%i + cur_store->num_items=%i < item_count=%i?\n", num_fixed-1, num_fixed_items, cur_store->num_items, item_count);
			return RC_NOT_FOUND;
		}

		if (mask_bitwise_or_compare(&match, &cur_store->items_mask, &the_mask))
		{
			//not sure if a best match or just a match - need to compare each item prices
			last_price = store_best_price(fixed_index+1); //number of stores found = fixed_index+1
			if (!best_price || (last_price < best_price))
			{
				DBGPRINT("%slast_price %f\n", ptr_indent,last_price);
				best_price = last_price;
				memcpy(match_best, match_last, sizeof(store_t*)*store_num());
			}
		}
		//else DBGPRINT("%sstore %li not match - fixed index %i\n", ptr_indent, cur_store->id, fixed_index);
		
		//advance to next store
		cur_store = cur_store->next;

		//back to first fixed position at the end of loop, but we can still increase number of fixed positions and restart!
		if ((fixed_index == 0) && (cur_store == NULL) && (num_fixed <= store_num()))
		{
			//unless a best price is already found
			if (best_price)
			{
				printf("Found best price: %.2f for the following stores:\n", best_price);
				store_print_best_match(match_best, num_fixed);
				
				free(match_last);
				match_last = 0;
				free(match_best);
				match_best = 0;
			
				return RC_BEST_MATCH;
			}
			
			//we haven't found any combination of stores with all the items using num_fixed stores
			//reset the mask we have found and increase the number of stores
			memset(&match, 0, sizeof(mask_t));
			num_fixed++;
			
			/* probably it will loop eternally after that, so let's limit the number of stores */
			if (num_fixed == MAX_STORES+1) return RC_NOT_FOUND;
			
			printf("Trying to match with %i stores\n", num_fixed);
			cur_store = store;
		}

	} while (cur_store != NULL);
	
	return RC_NOT_FOUND;
}