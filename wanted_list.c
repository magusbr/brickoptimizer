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

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "wanted_list.h"

#define NULL_CHAR_SIZE	1

wanted_list_t* wanted_list = NULL;
int iterator_index;

int wanted_list_new(char* code, char* url, int qty, int color)
{
	wanted_list_t* wl_tmp = NULL;
	//verify if item already exists
	wl_tmp = wanted_list;
	while (wl_tmp)
	{
		if (!strcmp(wl_tmp->wl_cod, code) && !strcmp(wl_tmp->wl_url, url) && (wl_tmp->wl_col == color))
		{
			wl_tmp->wl_qty += qty;
			return RC_EXISTS;
		}
		wl_tmp = wl_tmp->wl_nxt;
	}
	
	wl_tmp = calloc(1, sizeof(wanted_list_t));
	if (!wl_tmp) return RC_ERR_MALLOC;
	
	wl_tmp->wl_nxt = wanted_list;
	wanted_list = wl_tmp;
	
	wanted_list->wl_cod = (char*)malloc(strlen(code)+NULL_CHAR_SIZE);
	wanted_list->wl_url = (char*)malloc(strlen(url)+NULL_CHAR_SIZE);
	if (!wanted_list->wl_cod || !wanted_list->wl_url) return RC_ERR_MALLOC;
	
	strcpy(wanted_list->wl_cod, code);
	strcpy(wanted_list->wl_url, url);
	wanted_list->wl_qty = qty;
	wanted_list->wl_col = color;
	
	return RC_OK;
}

int wanted_list_del(wanted_list_t* wl)
{
	if (wl)
	{
		free(wl->wl_cod);
		free(wl->wl_url);
		free(wl);
	}
	else return RC_ERR;
	
	return RC_OK;
}

int wanted_list_clear()
{
	wanted_list_t* wl_tmp = wanted_list;
	while (wl_tmp)
	{
		wanted_list = wanted_list->wl_nxt;
		wanted_list_del(wl_tmp);
		wl_tmp = wanted_list;
	}
	
	return RC_OK;
}

wanted_list_t* wanted_list_iterator()
{
	iterator_index = 0;
	return wanted_list;
}

wanted_list_t* wanted_list_iterator_next(wanted_list_t* wl)
{
	if (wl)
	{
		iterator_index++;
		return wl->wl_nxt;
	}
	
	return NULL;
}

int wanted_list_iterator_index(wanted_list_t* wl)
{
	return iterator_index;
}

char* wanted_list_part_code(wanted_list_t* wl)
{
	if (wl)
		return wl->wl_cod;
	else
		return NULL;
}

int wanted_list_part_color(wanted_list_t* wl)
{
	if (wl)
		return wl->wl_col;
	else
		return RC_ERR;
}

char* wanted_list_part_url(wanted_list_t* wl)
{
	if (wl)
		return wl->wl_url;
	else
		return NULL;
}

int wanted_list_part_qty(wanted_list_t* wl)
{
	if (wl)
		return wl->wl_qty;
	else
		return RC_ERR;
}

int wanted_list_total_qty()
{
	int size = 0;
	wanted_list_t* wl_tmp = wanted_list;
	
	while (wl_tmp)
	{
		size += wl_tmp->wl_qty;
		wl_tmp = wl_tmp->wl_nxt;
	}
	
	return size;
}

int wanted_list_size()
{
	int size = 0;
	wanted_list_t* wl_tmp = wanted_list;
	
	while (wl_tmp)
	{
		size++;
		wl_tmp = wl_tmp->wl_nxt;
	}
	
	return size;
}

int wanted_list_print_store_url_by_index(int index, long store_id, long store_item_id)
{
	int idx = 0;
	wanted_list_t* wl_tmp = wanted_list;
	
	while (wl_tmp)
	{
		if (index == idx)
		{
			printf("http://www.bricklink.com/store.asp?sID=%lu&itemID=%lu - %i units\n", store_id, store_item_id, wl_tmp->wl_qty);
		}
		wl_tmp = wl_tmp->wl_nxt;
		idx++;
	}
	
	return RC_NOT_FOUND;
}

char* wanted_list_part_url_by_index(int index)
{
	int idx = 0;
	wanted_list_t* wl_tmp = wanted_list;
	
	while (wl_tmp)
	{
		if (index == idx)
		{
			return wl_tmp->wl_url;
		}
		wl_tmp = wl_tmp->wl_nxt;
		idx++;
	}
	
	return RC_NOT_FOUND;
}