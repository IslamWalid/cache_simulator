#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>


#define NOT_FOUND -1

/* represents a status flag to determine what happened in cache each memory access */ 
typedef enum status {
    MISS_AND_REPLACE = -1,
    MISS = 0,
    HIT = 1
}status_t;


typedef struct line
{
    unsigned long long tag;
    unsigned int order;
    unsigned char valid;
} line_t;

typedef struct set
{
    unsigned int most_recent_order;
    line_t *lines;
} set_t;

typedef struct cache
{
    set_t *sets;
    unsigned int set_number;
    unsigned int set_bits;
    unsigned int lines_per_set;
    unsigned int block_bits;
    unsigned int block_size;
} cache_t;


int parseArguments(int argc, char **argv,
                   unsigned int *set_bits, unsigned int *lines_per_set, unsigned int *block_bits,
                   FILE **trace);
/*
 * Function: parseArguments
 * ------------------------
 * parse the options given by the user.
 *  
 *      argc: number of arguments passed to main (main's argc)
 *      argv: pointer to the array of arguments (main's argv)
 * 
 *      set_bits: the number of set bits(s) that decodes (2^s) cache set(S)
 *      line_per_set: the number of lines per set
 *      block_bits: the number of block bits(b) that decodes a block 
 *      
 *      trace: a pointer to the pointer of the trace file
 * 
 *      return: 1 if -v option(verbose) is set
 *              0 if -v option(verbose) is not set
 */


cache_t *createCache(unsigned int s, unsigned int E, unsigned int b);
/*
 * Function: createCache
 * ------------------------
 * Allocate and initialize memory for cache.
 * 
 *      s: set bits
 *      E: lines per set
 *      b: block bits
 * 
 *      return: a pointer to the allocated memory for the cache
 */


void accessMemory(FILE *trace, cache_t *cache_ptr, int verbose,
                  unsigned int *hit_count_ptr, unsigned int *miss_count_ptr, unsigned int *eviction_count_ptr);
/*
 * Function: accessMemory
 * ----------------------
 * Simulate the memory access for each address in the trace file and updates counters depending on each access status.
 * 
 *      trace: a pointer to the trace file
 *      cache_ptr: a pointer to the allocated cache
 *      hit_count_ptr: a pointer to the hit counter
 *      miss_count_ptr: a pointer to the miss counter
 *      eviction_count_ptr: a pointer to the eviction counter
 * 
 *      return: void
 */


void deleteCache (cache_t *cache_ptr);
/*
 * Function: deleteCache
 * ---------------------
 *  Free the allocated memory for the cache
 * 
 *      cache_ptr: a pointer to the cache
 * 
 *      return: void
 */