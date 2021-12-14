#
# Student makefile for Cache Lab
# Note: requires a 64-bit x86-64 system 
#
CC = gcc
CFLAGS = -g -Wall -Werror -std=c99 -m64


csim: csim.c cachelab.c cache-sim.c cachelab.h cache-sim.h
	$(CC) $(CFLAGS) -o csim csim.c cachelab.c cache-sim.c -lm 


#
# Clean the src dirctory
#
clean:
	rm -rf *.o
	rm -f csim
	rm -f trace.all trace.f*
	rm -f .csim_results
