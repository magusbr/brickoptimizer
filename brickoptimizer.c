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
/*
set PATH=C:\Users\gustavo\Downloads\CodeBlocks-EP\MinGW\bin;%PATH%
*/

#include <stdio.h>
#include <tidy.h>
#include <buffio.h>
#include <curl/curl.h>
#include "parser.h"
#include "wanted_list.h"
#include "optimizer.h"
#include "mask.h"
#ifdef DEBUG
#include "insert.c"
#endif

CURL *curl;
CURLcode res;
char curl_errbuf[CURL_ERROR_SIZE];

TidyDoc tdoc;
TidyBuffer docbuf = {0};
TidyBuffer tidy_errbuf = {0};

/* curl write callback, to fill tidy's input buffer...  */ 
size_t write_cb( char *in, size_t size, size_t nmemb, void *out)
{
  uint r;
  r = size * nmemb;
  tidyBufAppend( (TidyBuffer*)out, in, r );
  return(r);
}

int login(char* username, char* password)
{
  if(curl) {
    char* redir_url;
	char data[256] = "a=a&logFrmFlag=Y&frmUsername=";
	strcat(data, username);
	strcat(data, "&frmPassword=");
	strcat(data, password);
	curl_easy_setopt(curl, CURLOPT_URL, "https://www.bricklink.com/login.asp?logInTo=&logFolder=p&logSub=w");
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

    res = curl_easy_perform(curl);
    if ( !res ) {
	  curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &redir_url);
	  if (!strcmp(redir_url,"https://www.bricklink.com/pageRedirect.asp?p=my.asp"))
		return RC_LOGIN_OK;
    }
    else
      fprintf(stderr, "%s\n", curl_errbuf);
  }
  return RC_LOGIN_ERR;
}

int get_wanted_list()
{
  if(curl) {
    //curl_easy_setopt(curl, CURLOPT_URL, "file://BrickLink My Wanted List.htm");
	curl_easy_setopt(curl, CURLOPT_URL, "http://www.bricklink.com/wantedDetail.asp?viewFrom=wantedSearch&wantedSize=10000");
    
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    if ( !res ) {
      res = tidyParseBuffer(tdoc, &docbuf); /* parse the input */ 
      if ( res >= 0 ) {
        res = tidyCleanAndRepair(tdoc); /* fix any problems */ 
        if ( res >= 0 ) {
          res = tidyRunDiagnostics(tdoc); /* load tidy error buffer */ 
          if ( res >= 0 ) {
            if (parse_wanted( tdoc, tidyGetRoot(tdoc)) == RC_OK)
				return RC_OK;
          }
        }
      }
    }
    else
      fprintf(stderr, "%s\n", curl_errbuf);
 
  }

  return RC_ERR;
}
 
int get_item_price(wanted_list_t* wl)
{ 
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, wanted_list_part_url(wl));
    //curl_easy_setopt(curl, CURLOPT_URL, "file://BrickLink Price Guide - Part 30602 in Black Color.htm");
	//curl_easy_setopt(curl, CURLOPT_URL, "http://www.bricklink.com/catalogPG.asp?P=41770&colorID=86");
    
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    if ( !res ) {
      res = tidyParseBuffer(tdoc, &docbuf); /* parse the input */ 
      if ( res >= 0 ) {
        res = tidyCleanAndRepair(tdoc); /* fix any problems */ 
        if ( res >= 0 ) {
          res = tidyRunDiagnostics(tdoc); /* load tidy error buffer */ 
          if ( res >= 0 ) {
			if (parse_price(tdoc, tidyGetRoot(tdoc), wl) == RC_OK)
				return RC_OK;
          }
        }
      }
    }
    else
      fprintf(stderr, "%s\n", curl_errbuf);
 
  }
  return RC_ERR;
}

void brick_process()
{
	int num_items;
	int sum_items = 0;
	int num_processed;
	wanted_list_t* wl;
  
#ifdef DEBUG
	FDBGLIST = fopen("insert.c", "w");
	fprintf(FDBGLIST, "insert() {\n");
#endif

	printf("Logged in...\n");
	if (get_wanted_list() != RC_OK)
	{
		printf("Error getting Wanted List. Aborting...\n");
	}
	else
	{
		num_items = wanted_list_size();
		sum_items = wanted_list_total_qty();

#ifdef DEBUG
		fprintf(FDBGLIST, "store_init(%i);\n", num_items);
#endif
		
		if (num_items > 64*MASK_T_SIZE)
		{
			printf("Number of items [%i] is greater than mask limit [%i].\nPlease increase the mask array size an recompile.\n", sum_items, 64*MASK_T_SIZE);
			return;
		}
		
		printf("Wanted List items parsed. Calculating price...\n");
		store_init(num_items);
		
		num_processed = 0;
		wl = wanted_list_iterator();

		while (wl)
		{
			if ((wanted_list_part_url(wl) == NULL) || (wanted_list_part_qty(wl) == RC_ERR) || (wanted_list_part_code(wl) == NULL))
			{
				printf("Invalid item found. Aborting...\n");
				break;
			}
			
			printf("Processing item [%i] of [%i]\n", num_processed, num_items);
			DBGPRINT("Get prices for url: %s\n", wanted_list_part_url(wl));
			
			if (get_item_price(wl) != RC_OK)
			{
				printf("Unable to get item list of prices. Aborting...\n");
				break;
			}
			
			num_processed++;
			
			wl = wanted_list_iterator_next(wl);
		}
		
		printf("All lots [%i] with [%i] items price calculated. Lucky you!\n", num_items, sum_items);

		
		mask_t mask = {{0}};
		if (store_best_match (0, mask, NULL) == RC_BEST_MATCH)
		{
			printf("Best match found! Amazing!\n");
		}
		else
		{
			printf("Something went wrong. Could not find the best match\n");
		}
		DBGPRINT("num stores: %i\nnum items: %i\n",store_num(), num_items);
		
		wanted_list_clear();
		store_end();
	}

#ifdef DEBUG
	fprintf(FDBGLIST, "}\n");
	fclose(FDBGLIST);
#endif

}
 
int main(int argc, char **argv )
{ 
	if (argc < 5)
	{
		printf("usage: brickoptimizer <username> <password> <wanted list name> <session_id>\n");
		return 0;
	}
	
	tdoc = tidyCreate();
	tidyOptSetBool(tdoc, TidyForceOutput, yes); /* try harder */ 
	tidyOptSetInt(tdoc, TidyWrapLen, 4096);
	tidySetErrorBuffer( tdoc, &tidy_errbuf );
	tidyBufInit(&docbuf);

	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &docbuf);

	/* what URL that receives this POST */ 
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0");
	curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1 );
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1 );

	curl_easy_setopt(curl,CURLOPT_CAINFO,"cacert.pem");
	curl_easy_setopt(curl,CURLOPT_CAPATH,"cacert.pem");
	curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1L);
	//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");

	tidyOptSetBool( tdoc, TidyMark, no);
	
#if 0
	insert();
	store_end();
	wanted_list_clear();
#endif

	if (login(argv[1], argv[2]) == RC_LOGIN_OK)
	{
		wanted_list_name = argv[3];
		session_id = argv[4];
		brick_process();
	}
	else
	{
		printf("Login failed.\nPlease check your username and password.\n");
	}
  
  curl_easy_cleanup(curl);

  tidyBufFree(&docbuf);
  if (tidy_errbuf.allocator)
	tidyBufFree(&tidy_errbuf);
  tidyRelease(tdoc);

  return(0);
}



#if 0
//unused options
//nbsp ? 
/*
char-encoding: latin1
quote-nbsp: yes
quote-ampersand: yes
break-before-br: no
>drop-empty-paras: yes
>drop-font-tags: yes
>write-back: yes
>quiet: yes
>markup: yes
>indent: yes
>logical-emphasis: yes
>hide-endtags: no
>clean: no
>numeric-entities: no
>tidy-mark: no
*/

  //tidySetCharEncoding( tdoc, "latin1" );
  //tidyOptSetBool( tdoc, TidyCharEncoding, yes );
  //tidyOptSetBool( tdoc, TidyQuoteNbsp, no );
  //tidyOptSetBool( tdoc, TidyQuoteAmpersand, yes );
  //tidyOptSetBool( tdoc, TidyNumEntities, no );
  //tidyOptSetBool( tdoc, TidyBreakBeforeBR, no);
  //tidyOptSetBool( tdoc, TidyMakeBare, yes);
  //tidyOptSetBool( tdoc, TidyDropEmptyParas, yes);
  //tidyOptSetBool( tdoc, TidyPreserveEntities, yes);
#endif
