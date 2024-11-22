#include "../include/Hashtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HashTable* hashtableCreate(unsigned size) {
    HashTable* table = malloc(sizeof(HashTable));
    if (!table) {
        fprintf(stderr, "Error allocating memory for hash table\n");
        return NULL;
    }
    table->buckets = calloc(size, sizeof(HashNode*));
    if (!table->buckets) {
        fprintf(stderr, "Error allocating memory for buckets\n");
        free(table);
        return NULL;
    }
    table->size = size;
    return table;
}

unsigned int hashtableHash(HashTable* table, const char* key) {
    unsigned long hash = 5381;
    int c;

    while ((c = *key++))
        hash = ((hash << 5) + hash) + c;

    return hash % table->size;
}

void hashtableInsert(HashTable* table, const char* key, int value) {
    unsigned int index = hashtableHash(table, key);
    HashNode* new_node = malloc(sizeof(HashNode));
    if (!new_node) {
        fprintf(stderr, "Error allocating memory for new node\n");
        return;
    }
    new_node->key = strdup(key);  // Duplicate the key string
    new_node->value = value;      // Store the integer value
    new_node->next = table->buckets[index];
    table->buckets[index] = new_node;
}

int hashtableSearch(HashTable* table, const char* key) {
    unsigned int index = hashtableHash(table, key);
    HashNode* node = table->buckets[index];

    while (node) {
        if (strcmp(node->key, key) == 0) {
            return node->value; // Return the integer value
        }
        node = node->next;
    }
    return -1; // Return -1 to indicate not found
}

void hashtableDelete(HashTable* table, const char* key) {
    unsigned int index = hashtableHash(table, key);
    HashNode* node = table->buckets[index];
    HashNode* prev = NULL;

    while (node) {
        if (strcmp(node->key, key) == 0) {
            if (prev)
                prev->next = node->next;
            else
                table->buckets[index] = node->next;

            free(node->key);
            free(node); // No need to free value as it's an integer
            printf("Key '%s' deleted successfully.\n", key);
            return;
        }
        prev = node;
        node = node->next;
    }
    printf("Key '%s' not found.\n", key);
}

void hashtableFree(HashTable* table) {
    for (int i = 0; i < table->size; i++) {
        HashNode* node = table->buckets[i];
        while (node) {
            HashNode* temp = node;
            node = node->next;
            free(temp->key);
            free(temp); // No need to free value as it's an integer
        }
    }
    free(table->buckets);
    free(table);
}

void hashtablePrint(HashTable* table) {
    if (!table) {
        fprintf(stderr, "HashTable is NULL\n");
        return;
    }

    printf("HashTable (size = %u):\n", table->size);
    for (unsigned int i = 0; i < table->size; i++) {
        printf("Bucket %u: ", i);
        HashNode* current = table->buckets[i];
        if (!current) {
            printf("(empty)");
        }
        while (current) {
            printf("-> [Key: %s, Value: %d] ", current->key, current->value);
            current = current->next;
        }
        printf("\n");
    }
}