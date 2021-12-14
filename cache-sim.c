#include "cache-sim.h"


static status_t cache(cache_t *cache_ptr, unsigned int set_index, unsigned int tag);
/*
 * Function: cache
 * ---------------
 *  Perform the fetch operation in cache for the desired set index and tag
 * 
 *      cache_ptr: a pointer to the cache
 *      set_index: the index of the set which the instruction try to access
 *      tag: the tag of the line which the instruction try to access
 * 
 *      return: the result of the access attempt
 *              MISS_AND_REPLACE if the caches missed and evicted a line with the targted one
 *              MISS if the cache missed and no eviction happened
 *              HIT if the cache hit
 */


static void parseAddressInfo(cache_t *cache_ptr, unsigned long long address, unsigned int *set_index_ptr, unsigned int *tag_ptr);
/*
 * Function: parseAddressInfo
 * --------------------------
 * Extract the info embedded in the address.
 *      
 *      cache_ptr: a pointer to the cache
 *      address: the address to extract info from
 *      set_idnex_ptr: a pointer to the variable set_index in which the extracted index will be stored
 *      tag_ptr: a pointer to the variable tag in which the extracted tag will be stored
 * 
 *      return: void
 */


static int isLineExist(cache_t *cache_ptr, unsigned int set_index, unsigned int tag);
/*
 * Function: isLineExist
 * ---------------------
 *  Search for a line in a set.
 *      
 *      cache_ptr: a pointer to the cache
 *      set_index: the index of the target set
 *      tag: the tag of the target line
 * 
 *      return: the index of the line in the set if exists
 *              NOT_FOUND if not
 */


static int findEmptyLine(cache_t *cache_ptr, unsigned int set_index);
/*
 * Function: findEmptyLine
 * -----------------------
 *  Search for an empty line in a set.
 * 
 *      cache_ptr: a pointer to the cache
 *      set_index: the index of the targer set
 * 
 *      return: the index of the empty line in the set if exists
 *              NOT_FOUND if not
 */


static int findVictimLine(cache_t *cache_ptr, unsigned int set_index);
/*
 * Function: findVictimLine
 * ------------------------
 *  Search for a victem line to be evicted using LRU policy.
 *      
 *      cache_ptr: a pointer to the cache
 *      set_index: the index of the targer set
 * 
 *      return: the index of the selected victim
 */


static void printHelp();
/*
 * Function: printHelp
 * -------------------
 *  Print the help message.
 *      return: void
 */


int parseArguments(int argc, char **argv,
                    unsigned int *set_bits, unsigned int *lines_per_set, unsigned int *block_bits,
                    FILE **trace)
{
    char opt = 0;
    int verbose = 0;
    while ((opt = getopt(argc, argv, "s:E:b:t:vh")) != EOF)
    {
        switch (opt)
        {
        case 's':
            *set_bits = atoi(optarg);
            break;

        case 'E':
            *lines_per_set = atoi(optarg);
            break;

        case 'b':
            *block_bits = atoi(optarg);
            break;

        case 't':
            *trace = fopen(optarg, "r");
            break;
        
        case 'v':
            verbose = 1;
            break;

        case 'h':
            printHelp();
            exit(0);
            break;

        default:
            printf("invalid arguments\n");
            exit(-1);
        }
    }
    return verbose;
}

cache_t *createCache(unsigned int s, unsigned int E, unsigned int b)
{
    cache_t *cache_ptr = malloc(sizeof(cache_t));

    cache_ptr->block_bits = b;
    cache_ptr->block_size = 1 << b;
    cache_ptr->lines_per_set = E;

    cache_ptr->set_bits = s;
    cache_ptr->set_number = 1 << s;
    cache_ptr->sets = malloc(cache_ptr->set_number * sizeof(set_t));

    for (unsigned int i = 0; i < cache_ptr->set_number; i++)
    {
        cache_ptr->sets[i].most_recent_order = 0;
        cache_ptr->sets[i].lines = malloc(cache_ptr->lines_per_set * sizeof(line_t));
        for (unsigned int j = 0; j < cache_ptr->lines_per_set; j++)
        {
            cache_ptr->sets[i].lines[j].tag = 0;
            cache_ptr->sets[i].lines[j].valid = 0;
            cache_ptr->sets[i].lines[j].order = 0;
        }
    }
    return cache_ptr;
}

void accessMemory(FILE *trace, cache_t *cache_ptr, int verbose,
                  unsigned int *hit_count, unsigned int *miss_count, unsigned int *eviction_count)
{
    char op;
    unsigned int size;
    unsigned long long address;

    unsigned int set_index = 0;
    unsigned int tag = 0;

    status_t status;

    while (fscanf(trace, " %c %llx,%u", &op, &address, &size) != EOF)
    {
        parseAddressInfo(cache_ptr, address, &set_index, &tag);

        switch (op)
        {
        case 'M':
            /* modify instruction is considered as a contiguous load & store operation */
            status = cache(cache_ptr, set_index, tag);

            /* hit in both load and store */
            if (status == HIT)
            {
                *hit_count += 2;
                if (verbose)
                    printf("%c %llx,%u hit hit\n", op, address, size);
            }

            /* miss in the load then hit in the following store */
            else if (status == MISS)
            {
                (*miss_count)++;
                (*hit_count)++;
                if (verbose)
                    printf("%c %llx,%u miss hit\n", op, address, size);
            }

            /* miss and evict a line in the load then hit in the following store */
            else if (status == MISS_AND_REPLACE)
            {
                (*miss_count)++;
                (*eviction_count)++;
                (*hit_count)++;
                if (verbose)
                    printf("%c %llx,%u miss eviction hit\n", op, address, size);
            }
            break;

        case 'L':
        case 'S':
            status = cache(cache_ptr, set_index, tag);

            if (status == 1)
            {
                (*hit_count)++;
                if (verbose)
                    printf("%c %llx,%u hit\n", op, address, size);
            }
            else if (status == 0)
            {
                (*miss_count)++;
                if (verbose)
                    printf("%c %llx,%u miss\n", op, address, size);
            }
            else if (status == MISS_AND_REPLACE)
            {
                (*miss_count)++;
                (*eviction_count)++;
                if (verbose)
                    printf("%c %llx,%u miss eviction\n", op, address, size);
            }
            break;
        }
    }
    fclose(trace);
}

void deleteCache (cache_t *cache_ptr)
{
    for (unsigned int i = 0; i < cache_ptr->set_number; i++)
        free(cache_ptr->sets[i].lines);

    free(cache_ptr->sets);
    free(cache_ptr);
}



/* static functions implementation */

static void parseAddressInfo(cache_t *cache_ptr, unsigned long long address, unsigned int *set_index, unsigned int *tag)
{
    *set_index = (address >> cache_ptr->block_bits) & ~(-1 << cache_ptr->set_bits);
    *tag = address >> (cache_ptr->set_bits + cache_ptr->block_bits);
}

static status_t cache(cache_t *cache_ptr, unsigned int set_index, unsigned int tag)
{
    int line_index;
    status_t status;
    if ((line_index = isLineExist(cache_ptr, set_index, tag)) == NOT_FOUND)
    {
        if ((line_index = findEmptyLine(cache_ptr, set_index)) == NOT_FOUND)
        {
            line_index = findVictimLine(cache_ptr, set_index);
            status = MISS_AND_REPLACE;
        }
        else
            status = MISS;

        cache_ptr->sets[set_index].lines[line_index].valid = 1;
        cache_ptr->sets[set_index].lines[line_index].tag = tag;
        cache_ptr->sets[set_index].lines[line_index].order = ++(cache_ptr->sets[set_index].most_recent_order);
    }
    else
    {
        cache_ptr->sets[set_index].lines[line_index].order = ++(cache_ptr->sets[set_index].most_recent_order);
        status = HIT;
    }

    return status;
}

static int isLineExist(cache_t *cache_ptr, unsigned int set_index, unsigned int tag)
{
    int line_index = NOT_FOUND;
    for (int i = 0; i < cache_ptr->lines_per_set; i++)
    {
        if (cache_ptr->sets[set_index].lines[i].tag == tag && cache_ptr->sets[set_index].lines[i].valid == 1)
        {
            line_index = i;
            break;
        }
    }
    return line_index;
}

static int findVictimLine(cache_t *cache_ptr, unsigned int set_index)
{
    int line_index;
    int order = cache_ptr->sets[set_index].most_recent_order + 1;

    for (int i = 0; i < cache_ptr->lines_per_set; i++)
    {
        if (cache_ptr->sets[set_index].lines[i].order < order)
        {
            order = cache_ptr->sets[set_index].lines[i].order;
            line_index = i;
        }
    }
    return line_index;
}

static int findEmptyLine(cache_t *cache_ptr, unsigned int set_index)
{
    int line_index = NOT_FOUND;
    for (unsigned int i = 0; i < cache_ptr->lines_per_set; i++)
    {
        if (cache_ptr->sets[set_index].lines[i].valid == 0)
        {
            line_index = i;
            break;
        }
    }
    return line_index;
}

static void printHelp()
{
    char c;
    FILE *help_msg = fopen("help-msg.txt", "r");
    while ((c = fgetc(help_msg)) != EOF)
        printf("%c", c);
    fclose(help_msg);
}
