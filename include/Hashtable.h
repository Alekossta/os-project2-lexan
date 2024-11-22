#pragma once
#include <stdlib.h>
#include <string.h>

typedef struct HashNode {
    char* key;              // Key remains a string
    int value;              // Value is now an integer
    struct HashNode* next;  // Pointer to the next node for handling collisions
} HashNode;

typedef struct HashTable {
    HashNode** buckets;     // Array of bucket pointers
    unsigned size;
} HashTable;

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