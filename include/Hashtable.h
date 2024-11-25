#pragma once
#include <stdlib.h>
#include <string.h>

typedef struct HashNode {
    char* key;
    int value;
    struct HashNode* next;
} HashNode;

typedef struct HashTable {
    HashNode** buckets; 
    unsigned size;
} HashTable;

typedef struct {
    char* word;
    int frequency;
} WordFreq;

// Create a new hash table
HashTable* hashtableCreate(unsigned size);

// Compute the hash of a key
unsigned int hashtableHash(HashTable* table, const char* key);

// Insert a key-value pair
void hashtableInsert(HashTable* table, const char* key, int value);

// Search for a value by key
int hashtableSearch(HashTable* table, const char* key);

// Delete a key-value pair
void hashtableDelete(HashTable* table, const char* key);

// Free the hash table
void hashtableFree(HashTable* table);

// prints the hash table
void hashtablePrint(HashTable* table);

// print but sorted
void hashtablePrintSorted(HashTable* table);
