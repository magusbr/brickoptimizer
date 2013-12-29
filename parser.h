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

#ifndef __INCLUDE_PARSER_H__
#define __INCLUDE_PARSER_H__
#include <tidy.h>
#include "wanted_list.h"

#define RC_ERR_MALLOC	-2
#define RC_ERR			-1
#define RC_NOT_FOUND	0
#define RC_OK			0
#define RC_EXISTS		1
#define RC_BEST_MATCH	37
#define RC_LOGIN_ERR	-3
#define RC_LOGIN_OK		RC_OK

#define TEMP_STRINGFY_REDIRECTION(macro) 	#macro
#define TEMP_STRINGFY(macro)				TEMP_STRINGFY_REDIRECTION(macro)

#ifdef DEBUG
	#define DBGPRINT(fmt, ... )		\
	{ \
		printf(fmt, ##__VA_ARGS__ ); \
	}
#else
	#define DBGPRINT(fmt, ...)
#endif

FILE* FDBGLIST;

char* wanted_list_name;
char* session_id;

int parse_wanted(TidyDoc doc, TidyNode tnod);
int parse_price(TidyDoc doc, TidyNode tnod, wanted_list_t* wl);
void insert();



#endif