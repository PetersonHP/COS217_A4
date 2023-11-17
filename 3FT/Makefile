#--------------------------------------------------------------------
# Makefile for Assignment 4, Part 3 sample implementation
# Invoke with the command:
# 	make -f Makefile.sampleft
# Author: Christopher Moretti
#--------------------------------------------------------------------

CC=gcc217

TARGETS = ft nodeDebug

all: ft

clean:
	rm -f $(TARGETS) meminfo*.out

clobber: clean
	rm -f dynarray.o path.o ft_client.o ft.o nodeFT.o nodeDebug.o *~

nodeDebug: nodeDebug.o dynarray.o path.o
	gcc217m -g $^ -o $@

ft: dynarray.o path.o ft_client.o nodeFT.o ft.o
	$(CC) -g $^ -o $@

dynarray.o: dynarray.c dynarray.h
	$(CC) -g -c $<

path.o: path.c dynarray.h path.h a4def.h
	$(CC) -g -c $<

ft_client.o: ft_client.c ft.h a4def.h
	$(CC) -g -c $<

nodeFT.o: nodeFT.c dynarray.h nodeFT.h path.h a4def.h
	$(CC) -g -c $<

ft.o: ft.c dynarray.h nodeFT.h ft.h path.h a4def.h
	$(CC) -g -c $<

nodeDebug.o: nodeFT.c nodeFT.h dynarray.h path.h a4def.h
	gcc217m -g -c $< -D DEBUG -o nodeDebug.o
