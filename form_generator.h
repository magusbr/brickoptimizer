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

#ifndef __INCLUDE_FORM_GENERATOR_H__
#define __INCLUDE_FORM_GENERATOR_H__
#include "parser.h"

typedef struct form_generator_t
{
	FILE* form_file;

	char* ra_str;
	char* rb_str;
	char* rd_str;
	char* rq_str;

	char* ptr_ra_str;
	char* ptr_rb_str;
	char* ptr_rd_str;
	char* ptr_rq_str;
} form_generator_t;

int form_generator_init(form_generator_t* fm, int num_items, long store_id);

int form_generator_add_item(form_generator_t* fm, int index, long store_item_id);

int form_generator_end(form_generator_t* fm);


#endif