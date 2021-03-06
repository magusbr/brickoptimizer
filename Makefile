#Brickoptimizer - optimize the price of Bricklinks Wanted List
#
#   Copyright (C) 2013  Gustavo Miranda Teixeira (gustavo@ice.ufjf.br)
#   
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version provided this disclaimer remains
#   unchanged.
#   
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#   
#   You should have received a copy of the GNU General Public License
#   along with this program; If not, see <http://www.gnu.org/licenses/>.

RM=del
CC=gcc
CFLAGS=-c -Wall -O3
#CFLAGS=-c -Wall -g -DDEBUG
INCS=-IC:\Temp\brickoptimizer\curl-7.33.0-devel-mingw32\include -IC:\Temp\brickoptimizer\tidy\include 
LDFLAGS=-O3 -Wl,--enable-stdcall-fixup -LC:\Temp\brickoptimizer\curl-7.33.0-devel-mingw32\lib -LC:\Temp\brickoptimizer\tidy\lib
LIBS=-lcurldll -ltidy
SOURCES=brickoptimizer.c optimizer.c parser.c wanted_list.c mask.c form_generator.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=brickoptimizer

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCS) $< -o $@
	
clear:
	$(RM) *.o $(EXECUTABLE)
