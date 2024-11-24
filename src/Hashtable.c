#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Hashtable.h" // Ensure this header file contains the necessary structure definitions

HashTable* hashtableCreate(unsigned size) {
    HashTable* table = malloc(sizeof(HashTable));
    if (!table) {
        fprintf(stderr, "Error allocating memory for hash table\n");
        return NULL;
    }
    
    // Corrected memory allocation for buckets
    table->buckets = malloc(sizeof(HashNode*) * size);
    if (!table->buckets) {
        fprintf(stderr, "Error allocating memory for buckets\n");
        free(table);
        return NULL;
    }
    
    // Initialize buckets to NULL
    for (unsigned int i = 0; i < size; i++) {
        table->buckets[i] = NULL;
    }

    table->size = size;
    return table;
}

unsigned int hashtableHash(HashTable* table, const char* key) {
    unsigned long hash = 5381;
    int c;

    while ((c = (unsigned char)*key++))
        hash = ((hash << 5) + hash) + c;

    return hash % table->size;
}

void hashtableInsert(HashTable* table, const char* key, int value) {
    unsigned int index = hashtableHash(table, key);

    HashNode* current = table->buckets[index];
    while (current) {
        if (strcmp(current->key, key) == 0) {
            // Key exists, update the value
            current->value += value;
            return;
        }
        current = current->next;
    }

    // Key does not exist, insert a new node
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
            free(node);
            printf("Key '%s' deleted successfully.\n", key);
            return;
        }
        prev = node;
        node = node->next;
    }
    printf("Key '%s' not found.\n", key);
}

void hashtableFree(HashTable* table) {
    for (unsigned int i = 0; i < table->size; i++) {
        HashNode* node = table->buckets[i];
        while (node) {
            HashNode* temp = node;
            node = node->next;
            free(temp->key);
            free(temp);
        }
    }
    free(table->buckets);
    free(table);
}

void hashtablePrint(HashTable* table) {
    for (unsigned int i = 0; i < table->size; i++) {
        HashNode* current = table->buckets[i];
        if (current) 
        {
            while (current) {
                printf("[Key: %s, Value: %d]", current->key, current->value);
                current = current->next;
            }
            printf("\n");
        }
    }
}
