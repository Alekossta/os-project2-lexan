#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Hashtable.h"

HashTable* hashtableCreate(unsigned size) {
    HashTable* table = malloc(sizeof(HashTable));
    if (!table) {
        fprintf(stderr, "Error allocating memory for hash table\n");
        return NULL;
    }

    table->buckets = malloc(sizeof(HashNode*) * size);
    if (!table->buckets) {
        fprintf(stderr, "Error allocating memory for buckets\n");
        free(table);
        return NULL;
    }
    
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
    new_node->key = strdup(key);
    new_node->value = value;
    new_node->next = table->buckets[index];
    table->buckets[index] = new_node;
}

int hashtableSearch(HashTable* table, const char* key) {
    unsigned int index = hashtableHash(table, key);
    HashNode* node = table->buckets[index];

    while (node) {
        if (strcmp(node->key, key) == 0) {
            return node->value;
        }
        node = node->next;
    }
    return -1;
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

// Comparison function for qsort
int compareWordFreq(const void* a, const void* b) {
    WordFreq* wf1 = (WordFreq*)a;
    WordFreq* wf2 = (WordFreq*)b;
    return wf2->frequency - wf1->frequency;
}

void hashtablePrintAndWriteTopK(HashTable* table, int k, char* outputFileName) {

    // count entries
    int totalEntries = 0;
    for (unsigned int i = 0; i < table->size; i++) {
        HashNode* node = table->buckets[i];
        while (node) {
            totalEntries++;
            node = node->next;
        }
    }

    // allocate wordfreq struct array for all entries
    WordFreq* entries = malloc(totalEntries * sizeof(WordFreq));
    if (!entries) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }

    // setup the wordfreq structure for each "node"
    int index = 0;
    for (unsigned int i = 0; i < table->size; i++) {
        HashNode* node = table->buckets[i];
        while (node) {
            entries[index].word = strdup(node->key);
            entries[index].frequency = node->value;
            index++;
            node = node->next;
        }
    }
    // sort the wordfreq struct array
    qsort(entries, totalEntries, sizeof(WordFreq), compareWordFreq);

    // count total of words
    int totalWords = 0;
    for(int i = 0; i < totalEntries; i++)
    {
        totalWords += entries[i].frequency;
    }

    for (int i = 0; i < k; i++) {
        printf("%s: %lf\n", entries[i].word, entries[i].frequency / (double)totalWords);
    }

    FILE* outputFile = fopen(outputFileName, "w");
    if(outputFile)
    {
        for(int i = 0; i < k; i++)
        {
            fprintf(outputFile, "%s: %d\n", entries[i].word, entries[i].frequency); 
        }
    }

    // free all words
    for(int i = 0; i < totalEntries; i++)
    {
        free(entries[i].word);
    }

    fclose(outputFile);
    free(entries);
}