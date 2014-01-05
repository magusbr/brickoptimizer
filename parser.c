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
#include <tidy.h>
#include <buffio.h>
#include <curl/curl.h>
#include "parser.h"
#include "wanted_list.h"
#include "optimizer.h"


int parse_wanted(TidyDoc doc, TidyNode tnod)
{
  static int found_list_name = 0;
  static char img_alt_url[1024];
  char list_name[256];
  int part_color;
  static int part_qty;
  char part_code[256];
  char part_url[256];
  TidyNode child;
  char* attr_value;
  
  for ( child = tidyGetChild(tnod); child; child = tidyGetNext(child) )
  {
    ctmbstr name = NULL;
	
    if (tidyNodeIsIMG(child))
	{
	  attr_value = (char*)tidyAttrValue(tidyAttrGetALT(child));
	  if (attr_value && !strncmp(attr_value, "Part No: ", 9))
	    strcpy(img_alt_url,tidyAttrValue(tidyAttrGetSRC(child)));
	}
	else if (tidyNodeIsINPUT(child))
	{
	  attr_value = (char*)tidyAttrValue(tidyAttrGetTYPE(child));
	  if (attr_value && !strcmp(tidyAttrValue(tidyAttrGetTYPE(child)), "text"))
	    if ((attr_value = (char*)tidyAttrValue(tidyAttrGetVALUE(child))))
	      part_qty = atoi(attr_value);
	}
	else name = tidyNodeGetName( child );
	
	if (!name)
	{
      TidyBuffer buf;
      tidyBufInit(&buf);
      tidyNodeGetText(doc, child, &buf);
	  
	  //last one is a "On My" string, meaning this is the list name
	  if (found_list_name && buf.bp)
	  {
	    //have to remove last 2 characters from buffer \n and some other
		sscanf((char*)buf.bp, "%s", list_name);
		
		if (!strcmp(list_name, wanted_list_name))
		{
			//grab the color code after the first "/"
			sscanf(img_alt_url, "http://img.bricklink.com/P/%i/%[^.]", &part_color, part_code);
		
			DBGPRINT("%s(%i): http://www.bricklink.com/catalogPG.asp?P=%s&colorID=%i\n", list_name, part_qty, part_code, part_color);

			sprintf(part_url, "http://www.bricklink.com/catalogPG.asp?P=%s&colorID=%i", part_code, part_color);
			if (wanted_list_new(part_code, part_url, part_qty, part_color) != RC_OK)
				return RC_ERR;
#ifdef DEBUG
			fprintf(FDBGLIST, "wanted_list_new(%s, %s, %i, %i);\n", part_code, part_url, part_qty, part_color);
#endif
		}
	  }
	  
	  //"On My" string found, then next one is the list name
	  if (buf.bp && !strncmp((char*)buf.bp,"On My",5))
	  {
	    found_list_name=1;
	  }
	  else
	    found_list_name=0;
		
      tidyBufFree(&buf);
    }
	
    if (parse_wanted(doc, child) != RC_OK)
		return RC_ERR;
  }
  
  return RC_OK;
}

int parse_price(TidyDoc doc, TidyNode tnod, wanted_list_t* wl)
{
  static int found_link=0;
  TidyNode child;
  char str[256];
  static char store_item_url[1024];
  static int store_item_qty;
  static float store_item_price;
  char store_currency[10];
  static long store_id;
  static int store_item_id;
  static int found_prices = 0;
  char* attr_value;

  for ( child = tidyGetChild(tnod); child; child = tidyGetNext(child) )
  {
    ctmbstr name = tidyNodeGetName( child );

	if (found_prices)
    {
      if (tidyNodeIsA(child))
	  {
        attr_value = (char*)tidyAttrValue(tidyAttrGetHREF(child));
	    if (attr_value && !strncmp(attr_value, "/store.asp", 10))
	    {
	      found_link=3; //we will need next three lines of text
	      strcpy(store_item_url,attr_value);
        }
	  }
	}
	
    if (!name) {
      /* if it doesn't have a name, then it's probably text, cdata, etc... */ 
      TidyBuffer buf;
      tidyBufInit(&buf);
      tidyNodeGetText(doc, child, &buf);

      if (strstr((char *)buf.bp,"Currently Available")) found_prices=1;
      if (strstr((char *)buf.bp,"Total Lots")) found_prices=0;
      if (found_prices && buf.bp && found_link)
	  {
	    found_link--;
	    strncpy(str, (char*)buf.bp, buf.size);
	    str[buf.size-2]=0;
		
		//fist time it comes with trash
		if (!strcmp(str,"&nbsp;")) continue;
		
		//second time it is the qty
		if (found_link == 1)
		{
			store_item_qty = atoi(str);
		}
		//third time it is price
		else if (found_link == 0)
		{
			if (store_item_qty >= wanted_list_part_qty(wl))
			{
				sscanf(str, "&nbsp;~%[^&]&nbsp;%f", store_currency, &store_item_price);

				sscanf(store_item_url, "/store.asp?sID=%li&itemID=%i", &store_id, &store_item_id);
			
				DBGPRINT("%s %.2f %i https://www.bricklink.com%s %li\n", store_currency, store_item_price, store_item_qty, store_item_url, store_id);
				
				if (store_add(store_id) == RC_ERR_MALLOC)
					return RC_ERR;
				if (store_add_item(store_id, wanted_list_iterator_index(wl), store_item_id, store_item_price * wanted_list_part_qty(wl)) != RC_OK)
					return RC_ERR;
#ifdef DEBUG
				fprintf(FDBGLIST, "store_add(%li);\n",store_id);
				fprintf(FDBGLIST, "store_add_item(%li, %i, %0.2f);\n",store_id, wanted_list_iterator_index(wl), store_item_price);
#endif
			}
		}
	  }
      tidyBufFree(&buf);
    }
	
    if (parse_price(doc, child, wl) != RC_OK)
		return RC_ERR;
  }
  
  return RC_OK;
}
