/*
 * cache.c
 *
 * 20493-01 Computer Architecture
 * Term Project on Implementation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 15, 2023
 *
 */


#include <stdio.h>
#include <string.h>
#include "cache_impl.h"

extern int num_cache_hits;
extern int num_cache_misses;

extern int num_bytes;
extern int num_access_cycles;

extern int global_timestamp;

cache_entry_t cache_array[CACHE_SET_SIZE][DEFAULT_CACHE_ASSOC];
int memory_array[DEFAULT_MEMORY_SIZE_WORD];


/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_memory_content() {
    unsigned char sample_upward[16] = { 0x001, 0x012, 0x023, 0x034, 0x045, 0x056, 0x067, 0x078, 0x089, 0x09a, 0x0ab, 0x0bc, 0x0cd, 0x0de, 0x0ef };
    unsigned char sample_downward[16] = { 0x0fe, 0x0ed, 0x0dc, 0x0cb, 0x0ba, 0x0a9, 0x098, 0x087, 0x076, 0x065, 0x054, 0x043, 0x032, 0x021, 0x010 };
    int index, i = 0, j = 1, gap = 1;

    for (index = 0; index < DEFAULT_MEMORY_SIZE_WORD; index++) {
        memory_array[index] = (sample_upward[i] << 24) | (sample_upward[j] << 16) | (sample_downward[i] << 8) | (sample_downward[j]);
        if (++i >= 16)
            i = 0;
        if (++j >= 16)
            j = 0;

        if (i == 0 && j == i + gap)
            j = i + (++gap);

        printf("mem[%d] = %#x\n", index, memory_array[index]);
    }
}

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_cache_content() {
    int i, j;

    for (i = 0; i < CACHE_SET_SIZE; i++) {
        for (j = 0; j < DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t* pEntry = &cache_array[i][j];
            pEntry->valid = 0;
            pEntry->tag = -1;
            pEntry->timestamp = 0;
        }
    }
}

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
/* This function is a utility function to print all the cache entries. It will be useful for your debugging */
void print_cache_entries() {
    int i, j, k;

    for (i = 0; i < CACHE_SET_SIZE; i++) {
        printf("[Set %d] ", i);
        for (j = 0; j < DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t* pEntry = &cache_array[i][j];
            printf("V: %d Tag: %#x Time: %d Data: ", pEntry->valid, pEntry->tag, pEntry->timestamp);
            for (k = 0; k < DEFAULT_CACHE_BLOCK_SIZE_BYTE; k++) {
                printf("%#x(%d) ", pEntry->data[k], k);
            }
            printf("\t");
        }
        printf("\n");
    }
}

int check_cache_data_hit(void* addr, char type) {
    unsigned long int address = (unsigned long int)addr;
    int cache_index = (address / DEFAULT_CACHE_BLOCK_SIZE_BYTE) % CACHE_SET_SIZE;
    int tag = ((address / DEFAULT_CACHE_BLOCK_SIZE_BYTE) / CACHE_SET_SIZE);
    int byte_offset = address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;
    int entry_index = 0;

    // Iterate over each entry in the cache set
    for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {
        cache_entry_t* pEntry = &cache_array[cache_index][i];

        // Check if the entry is valid and has a matching tag
        if (pEntry->valid == 1 && pEntry->tag == tag) { // Cache hit
            pEntry->timestamp = global_timestamp++; // Update the timestamp for cache hit
            entry_index = i;

            // Retrieve data based on the data type
            cache_entry_t* pEntry = &cache_array[cache_index][entry_index];
            if (type == 'b') { // Byte
                return pEntry->data[byte_offset];
            }
            else if (type == 'h') { // Halfword (2 bytes)
                int data = pEntry->data[byte_offset] & 0xffff00ff; //extract lower 8 bits of the first byte 0x00ㅁㅁ
                int data1 = (pEntry->data[byte_offset + 1] << 8) & 0xffffff00; //extract upper 8 bits of the second byte, and shift left to get 0xㅁㅁ00
                return data | data1; //combine to make 0xㅁㅁㅁㅁ
            }
            else { // Word (4 bytes)
                int data = pEntry->data[byte_offset] & 0xffff00ff; //extract lower 8 bits of first byte 0x00ㅁㅁ
                int data1 = (pEntry->data[byte_offset + 1] << 8) & 0x0000ff00; //extract upper 8 bits of second byte, and shift left to get 0xㅁㅁ00
                int data2 = (pEntry->data[byte_offset + 2] << 16) & 0x00ff0000; //extract uppoer 8 bits of third byte, and shift left to get 0xㅁㅁ0000
                int data3 = (pEntry->data[byte_offset + 3] << 24) & 0xff000000; //extract upper 8 bits of fourth byte, and shift left to get 0xㅁㅁ000000
                return data | data1 | data2 | data3; //combine to make 0xㅁㅁㅁㅁㅁㅁㅁㅁ
            }
        }
    }
    
    // Cache miss
    return -1;
}

int find_entry_index_in_set(void* addr, int cache_index) {
    int entry_index;
    unsigned long int byte_addr = (unsigned long int)addr; // Convert address to bytes
    int block_addr = byte_addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE;
    int tag = block_addr / CACHE_SET_SIZE; // Calculate tag based on cache associativity
    int i;

    // Check if the cache is direct-mapped (associativity = 1)
    if (DEFAULT_CACHE_ASSOC == 1) { 
        entry_index = 0; // In direct-mapped cache, there's only one possible entry index (0)
        cache_entry_t* pEntry = &cache_array[cache_index][0];
        pEntry->timestamp = global_timestamp; //Update the timestamp for the only entry
    }
    else {
        // Check the tag of all entries to find a matching entry
        for (i = 0; i < DEFAULT_CACHE_ASSOC; i++) { 
            cache_entry_t* pEntry = &cache_array[cache_index][i];
            if (pEntry->tag == tag) {
                entry_index = i;
                pEntry->timestamp = global_timestamp; // Update the timestamp for the matched entry
                return entry_index; // Return the found entry index
            }
        }
        
        // Check valid bit to find any empty cache space
        for (i = 0; i < DEFAULT_CACHE_ASSOC; i++) { 
            cache_entry_t* pEntry = &cache_array[cache_index][i];
            if (pEntry->valid == 0) { 
                entry_index = i; // If valid bit is 0, entry index in set is i
                pEntry->timestamp = global_timestamp; // Update the timestamp for the empty entry
                return entry_index; // Return the found entry index
            }
        }

        // If all entries are occupied, find the least recently used (LRU) entry
        cache_entry_t* pEntry = &cache_array[cache_index][0];
        int LRU_timestamp = pEntry->timestamp; //set LRU_timestamp as timestamp of first entry
        int min_index = 0;

        for (i = 1; i < DEFAULT_CACHE_ASSOC; i++) { // Check timestamp to find LRU entry
            cache_entry_t* pEntry = &cache_array[cache_index][i];
            if (LRU_timestamp > pEntry->timestamp) {
                LRU_timestamp = pEntry->timestamp; // Update LRU_timestamp with timestamp with the minimum timestamp
                min_index = i; // Update the index of the LRU entry
            }
        }
        entry_index = min_index; // Set entry_index to the index of the LRU entry
        cache_array[cache_index][min_index].timestamp = global_timestamp; // Update the timestamp for the LRU entry
    }
    return entry_index; // Return cache entry index for copying from memory
}

int access_memory(void* addr, char type) {
    /* Fetch the data from the main memory and copy them to the cache */
    /* void *addr: addr is byte address, whereas your main memory address is word address due to 'int memory_array[]' */
    unsigned long int address = (unsigned long int)addr;
    int cache_index = (address / DEFAULT_CACHE_BLOCK_SIZE_BYTE) % CACHE_SET_SIZE;
    
    /* Calculate block address in main memory */
    int block_addr = (int)(address / DEFAULT_CACHE_BLOCK_SIZE_BYTE);
    int word_index = block_addr * DEFAULT_CACHE_BLOCK_SIZE_BYTE / WORD_SIZE_BYTE;
    int tag = (address / DEFAULT_CACHE_BLOCK_SIZE_BYTE) / CACHE_SET_SIZE;
    int mem1, mem2;
    int byte_offset = address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;
    
    /* You need to invoke find_entry_index_in_set() for copying to the cache.*/
    // Find the entry index in the set using LRU policy
    int entry_index = find_entry_index_in_set((int*)addr, cache_index);
    cache_entry_t* pEntry = &cache_array[cache_index][entry_index];
    
    // Update cache entry information
    pEntry->valid = 1;
    pEntry->tag = tag;
    pEntry->timestamp = global_timestamp++;

    // Update access cycles for memory access
    num_access_cycles += MEMORY_ACCESS_CYCLE;

    // Check if the word_index is out of bounds
    if (word_index >= DEFAULT_MEMORY_SIZE_WORD) {
        printf("word_index is out of bounds\n");
        return -1;
    }

    //Fetch the contents of the main memory for the specified word index
    mem1 = memory_array[word_index]; 
    //Fetch the contents of the next word in the main memory
    mem2 = memory_array[word_index + 1]; 

    /* Get data from memory and store in cache entry. This operation is performed for two consecutive words
    in memory (mem1 and mem2), loading all 8 bytes of the cache entry.*/
    pEntry->data[0] = mem1 & 0xff;              //Extract lower 8 bits of the first byte
    pEntry->data[1] = (mem1 & 0xff00) >> 8;     //Extract upper 8 bits of the second byte, and shift right
    pEntry->data[2] = (mem1 & 0xff0000) >> 16;  //Extract upper 8 bits of the third byte, and shift right 
    pEntry->data[3] = (mem1 & 0xff000000) >> 24;  //Extract upper 8 bits of the fourth byte, and shift right
    pEntry->data[4] = mem2 & 0xff;              //Extract lower 8 bits of the fifth byte, and shift right
    pEntry->data[5] = (mem2 & 0xff00) >> 8;      //Extract upper 8 bits of the sixth byte, and shift right
    pEntry->data[6] = (mem2 & 0xff0000) >> 16;    //Extract upper 8 bits of the seventh byte, and shift right
    pEntry->data[7] = (mem2 & 0xff000000) >> 24;  //Extract upper 8 bits of the eighth byte, and shift right

    /* Based on data type, return the appropriate value from the cache entry */
    if (type == 'b') {
        return pEntry->data[byte_offset];      //Return the byte at the specified offset
    }
    else if (type == 'h') {
        int data = pEntry->data[byte_offset] & 0xffff00ff;              //Extract lower 8 bits of the first byte
        int data1 = (pEntry->data[byte_offset + 1] << 8) & 0xffffff00;  //Extract upper 8 bits of second byte, and shift left to their correct position
        return data | data1;    // Combine to get halfword (16 bits)
    }
    else if (type == 'w') {
        int data = pEntry->data[byte_offset] & 0x000000ff;              //Extract lower 8 bits of first byte
        int data1 = (pEntry->data[byte_offset + 1] << 8) & 0x0000ff00;  //Extract upper 8 bits of second byte, and shift left
        int data2 = (pEntry->data[byte_offset + 2] << 16) & 0x00ff0000; //Extract upper 8 bits of third byte, and shift left
        int data3 = (pEntry->data[byte_offset + 3] << 24) & 0xff000000; //Extract upper 8 bits of fourth byte, and shift left
        return data | data1 | data2 | data3;    //Combine to get word (32 bits)
    }
    else {
        return -1;  // Invalid data type
    }
}