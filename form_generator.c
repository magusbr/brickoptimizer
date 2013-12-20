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
#include "form_generator.h"
#include "wanted_list.h"



static char* form_header = "<HTML> \n\
<HEAD> \n\
 <META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=utf-8\"> \n\
 <META HTTP-EQUIV=\"IMAGETOOLBAR\" CONTENT=\"NO\"> \n\
 <LINK REL=\"STYLESHEET\" TYPE=\"text/css\" HREF=\"/stylesheet.css?13\"> \n\
 <STYLE TYPE=\"text/css\">body { margin: 0 auto; }</STYLE> \n\
 <SCRIPT TYPE=\"text/javascript\" LANGUAGE=\"javascript\" SRC=\"/js/getAjax.js\"></SCRIPT> \n\
 <SCRIPT TYPE=\"text/javascript\" LANGUAGE=\"javascript\" SRC=\"/lytebox/lytebox.js?10\"></SCRIPT> \n\
 <LINK REL=\"STYLESHEET\" HREF=\"/lytebox/lytebox.css?13\" TYPE=\"text/css\" MEDIA=\"screen\" /> \n\
</HEAD> \
<BODY BGCOLOR=\"#FFFFFF\">";

static char* form_tail = "<INPUT TYPE=\"HIDDEN\" NAME=\"BA\" VALUE=\"\"> \n\
<INPUT TYPE=\"HIDDEN\" NAME=\"BB\" VALUE=\"\"> \n\
<INPUT TYPE=\"HIDDEN\" NAME=\"BQ\" VALUE=\"\"> \n\
\n\
<CENTER><INPUT TYPE=\"SUBMIT\" VALUE=\"Add Selected Items to Shopping Cart\" onClick=\"this.disabled=true;this.form.submit()\"></CENTER> \n\
</BODY></HTML>\n";

int form_generator_init(form_generator_t* fm, int num_items, long store_id)
{
	char form_file_name[256] = "";
	sprintf(form_file_name, "form_buy_%lu.html", store_id);
	
	fm->form_file = fopen(form_file_name,"w");
	if (!fm->form_file) return RC_ERR;
	
	fm->ra_str = (char*)malloc(sizeof(char)*9*num_items);
	fm->rb_str = (char*)malloc(sizeof(char)*9*num_items);
	fm->rd_str = (char*)malloc(sizeof(char)*2*num_items);
	fm->rq_str = (char*)malloc(sizeof(char)*2*num_items);
	
	if (!fm->ra_str || !fm->rb_str || !fm->rd_str || !fm->rq_str) return RC_ERR;
	
	fm->ptr_ra_str = fm->ra_str;
	fm->ptr_rb_str = fm->rb_str;
	fm->ptr_rd_str = fm->rd_str;
	fm->ptr_rq_str = fm->rq_str;
	
	fprintf(fm->form_file, "%s\n", form_header);
	fprintf(fm->form_file, "<FORM METHOD=\"POST\" ACTION=\"http://www.bricklink.com/storeCart.asp?a=d&h=%lu&b=-15862789\">\n", store_id);
	
	return RC_OK;
}

int form_generator_add_item(form_generator_t* fm, int index, long store_item_id)
{
	int idx = 0;
	wanted_list_t* wl_tmp = wanted_list_iterator();
	
	while (wl_tmp)
	{
		if (index == idx)
		{
			if (fm->ptr_ra_str == fm->ra_str)
			{
				fm->ptr_ra_str += sprintf(fm->ptr_ra_str,"%lu", store_item_id);
				fm->ptr_rb_str += sprintf(fm->ptr_rb_str,"%i", wl_tmp->wl_qty);
				fm->ptr_rd_str += sprintf(fm->ptr_rd_str,"1");
				fm->ptr_rq_str += sprintf(fm->ptr_rq_str,"0");
			}
			else
			{
				fm->ptr_ra_str += sprintf(fm->ptr_ra_str,",%lu", store_item_id);
				fm->ptr_rb_str += sprintf(fm->ptr_rb_str,",%i", wl_tmp->wl_qty);
				fm->ptr_rd_str += sprintf(fm->ptr_rd_str,",1");
				fm->ptr_rq_str += sprintf(fm->ptr_rq_str,",0");
			}

			fprintf(fm->form_file, "<INPUT STYLE=\"font-size: 10px; font-family: MS Sans Serif,Geneva\" TYPE=\"Text\" SIZE=\"5\" NAME=\"qd%lu\" VALUE=\"%i\"><BR>\n", store_item_id, wl_tmp->wl_qty);
			
			return RC_OK;
		}
		wl_tmp = wanted_list_iterator_next(wl_tmp);
		idx++;
	}
	
	return RC_NOT_FOUND;
}


int form_generator_end(form_generator_t* fm)
{
	fprintf(fm->form_file, "<INPUT TYPE=\"HIDDEN\" NAME=\"RA\" VALUE=\"%s\">\n", fm->ra_str);
	fprintf(fm->form_file, "<INPUT TYPE=\"HIDDEN\" NAME=\"RB\" VALUE=\"%s\">\n", fm->rb_str);
	fprintf(fm->form_file, "<INPUT TYPE=\"HIDDEN\" NAME=\"RD\" VALUE=\"%s\">\n", fm->rd_str);
	fprintf(fm->form_file, "<INPUT TYPE=\"HIDDEN\" NAME=\"RQ\" VALUE=\"%s\">\n", fm->rq_str);
	
	fprintf(fm->form_file, "%s", form_tail);
	
	fclose(fm->form_file);
	
	free(fm->ra_str);
	free(fm->rb_str);
	free(fm->rd_str);
	free(fm->rq_str);
	fm->ra_str = NULL;
	fm->rb_str = NULL;
	fm->rd_str = NULL;
	fm->rq_str = NULL;
	
	return RC_OK;
}