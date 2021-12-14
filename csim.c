#include "cachelab.h"
#include "cache-sim.h"


int main(int argc, char **argv)
{
    int verbose = 0;
    unsigned int set_bits = 0,
                 lines_per_set = 0,
                 block_bits = 0;

    unsigned int hit_count = 0,
                 miss_count = 0,
                 eviction_count = 0;

    FILE *trace;
    cache_t *cache_ptr;

    verbose = parseArguments(argc, argv, &set_bits, &lines_per_set, &block_bits, &trace);

    cache_ptr = createCache(set_bits, lines_per_set, block_bits);

    accessMemory(trace, cache_ptr, verbose, &hit_count, &miss_count, &eviction_count);

    printSummary(hit_count, miss_count, eviction_count);

    deleteCache(cache_ptr);

    return 0;
}