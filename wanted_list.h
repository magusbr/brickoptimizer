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

#ifndef __INCLUDE_WANTED_LIST_H__
#define __INCLUDE_WANTED_LIST_H__

typedef struct wanted_list_t
{
	int wl_qty;
	int wl_col;
	char* wl_cod;
	char* wl_url;
	struct wanted_list_t* wl_nxt;
} wanted_list_t;

int wanted_list_new(char* code, char* url, int qty, int color);
int wanted_list_del(wanted_list_t* wl);
int wanted_list_clear();

wanted_list_t* wanted_list_iterator();
wanted_list_t* wanted_list_iterator_next(wanted_list_t* wl);
int wanted_list_iterator_index(wanted_list_t* wl);
char* wanted_list_part_code(wanted_list_t* wl);
char* wanted_list_part_url(wanted_list_t* wl);
int wanted_list_part_color(wanted_list_t* wl);
int wanted_list_part_qty(wanted_list_t* wl);
int wanted_list_size();
int wanted_list_print_store_url_by_index(int index, long store_id, long store_item_id);
int wanted_list_total_qty();


#endif