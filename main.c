/*
 * main.c
 *
 * 20493-01 Computer Architecture
 * Term Project on Implementation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 15, 2023
 *
 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "cache_impl.h"

int num_cache_hits = 0;
int num_cache_misses = 0;

int num_bytes = 0;
int num_access_cycles = 0;

int global_timestamp = 0;

int retrieve_data(void* addr, char data_type) {
    int value_returned = -1; /* accessed data */
    num_access_cycles += CACHE_ACCESS_CYCLE;
    unsigned long int access_addr = (unsigned long int)addr;

    // Check if data is in cache
    int cache_data = check_cache_data_hit((int*)addr, data_type);

    int data_size_bytes = 0;
    // Determine the size of the data based on the data type
    if (data_type == 'b') {
        data_size_bytes = 1;
    }
    else if (data_type == 'h') {
        data_size_bytes = 2;
    }
    else if (data_type == 'w') {
        data_size_bytes = 4;
    }
    else {
        // Handle invalid data type (if needed)
        return -1;
    }
    num_bytes += data_size_bytes;

    if (cache_data != -1) {
        /* Data is in the cache (cache hit) */
        value_returned = cache_data;
        num_cache_hits++;
        printf("=> Hit!\n");
    }
    else {
        /* Cache miss event: access main memory by invoking access_memory() */
        value_returned = access_memory((int*)addr, data_type);
        num_cache_misses++;
        printf("=> Miss!\n");
        printf("MEMORY >> word index = %lu\n", (access_addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE) * DEFAULT_CACHE_BLOCK_SIZE_BYTE / sizeof(int));
    }

    /* If there is no data neither in cache nor memory, return -1, else return data */
    return value_returned;
}


int main(void) {
    FILE* ifp = NULL, * ofp = NULL; //file pointers
    unsigned long int access_addr; /* byte address (located at 1st column) in "access_input.txt" */
    char access_type; /* 'b'(byte), 'h'(halfword), or 'w'(word) (located at 2nd column) in "access_input.txt" */
    int accessed_data; /* This is the data that you want to retrieve first from cache, and then from memory */

    /* initialize memory and cache by invoking init_memory_content() and init_cache_content() */
    init_memory_content();
    init_cache_content();

    /* open input file as reading mode */
    ifp = fopen("access_input.txt", "r");
    if (ifp == NULL) {
        printf("Can't open input file\n");
        return -1;
    }
    /* open output file as writing mode */
    ofp = fopen("access_output.txt", "w");
    if (ofp == NULL) {
        printf("Can't open output file\n");
        fclose(ifp);
        return -1;
    }

    fprintf(ofp, "[Accessed Data]\n");

    /* read each line and get the data in given (address, type) by invoking retrieve_data() */
    while (fscanf(ifp, "%lu %c", &access_addr, &access_type) != EOF) {
        
        /* Print information */
        printf("\n===== addr %lu type %c =====\n", access_addr, access_type);
        printf("CACHE >> block_addr = %lu, byte_offset = %lu, cache_index = %lu, tag = %lu\n",
            access_addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE,
            access_addr % DEFAULT_CACHE_BLOCK_SIZE_BYTE,
            (access_addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE) % CACHE_SET_SIZE,
            (access_addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE) / CACHE_SET_SIZE);
        
        // Retrieve data from cache or memory
       accessed_data = retrieve_data((void*)access_addr, access_type);
       fprintf(ofp, "%lu\t%c\t0x%x\n", (int)access_addr, access_type, accessed_data);

        printf("ENTRY >> \n"); // Show entry for each input (address and type)
        print_cache_entries();
        

    }


    /* Print hit ratio and bandwidth for each cache mechanism as regards to cache association size */
    printf("\n\nHit Ratio = %.2f\n", (num_cache_hits / (float)(num_cache_hits + num_cache_misses)));
    printf("Bandwidth = %.2lf (%d/%d)\n", ((float)num_bytes / (float)num_access_cycles), num_bytes, num_access_cycles);

    fprintf(ofp, "---------------------------------------\n");
    // Determine the type of cache mechanism
    if (DEFAULT_CACHE_ASSOC == 1) {
        fprintf(ofp, "[Direct Mapped Cache performance]\n");
    }
    else if (DEFAULT_CACHE_ASSOC == 2) {
        fprintf(ofp, "[2-way Set Associative Cache performance]\n");
    }
    else if (DEFAULT_CACHE_ASSOC == 4) {
        fprintf(ofp, "[Fully Associative Cache performance]\n");
    }
    else {
        fprintf(ofp, "Unknown Cache Type\n");
        return -1; // Exit with an error code
    }


    fprintf(ofp, "Hit ratio = %.2lf (%d/%d)\n", (((float)num_cache_hits / ((float)num_cache_hits + (float)num_cache_misses))), num_cache_hits, num_cache_hits + num_cache_misses);
    fprintf(ofp, "Bandwidth = %.2lf (%d/%d)\n", ((float)num_bytes / (float)num_access_cycles), num_bytes, num_access_cycles);

    /* close files */
    fclose(ifp);
    fclose(ofp);

    /* print the final cache entries by invoking print_cache_entries */
    printf("FINAL ENTRY >>\n");
    print_cache_entries();
    return 0;
}