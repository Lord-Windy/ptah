#ifndef ZOOKEEPER_H
#define ZOOKEEPER_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include <samrena.h>


//HONEYCOMB
// Define the structure for each key-value pair
typedef struct Cell {
    char* key;
    void* value;
    struct Cell* next;
} Cell;

// Define the main hashmap structure
typedef struct {
    Cell** cells;
    size_t size;        // Current number of elements
    size_t capacity;    // Number of buckets
    bool memory_managed_internally;
    Samrena* arena;    // Arena or null for malloc
} Honeycomb;

// Function declarations

/**
 *
 * @param samrena - NULLABLE - if null it will create its own
 */ 
Honeycomb* honeycomb_create(size_t initial_capacity, Samrena* samrena);

void honeycomb_destroy(Honeycomb* comb);

void honeycomb_put(Honeycomb* comb, const char* key, void* value);
void* honeycomb_get(Honeycomb* comb, const char* key);

void honeycomb_remove(Honeycomb* comb, const char* key);
bool honeycomb_contains(Honeycomb* comb, const char* key);

void honeycomb_print(Honeycomb* comb);

size_t honeycomb_size(const Honeycomb* comb);
bool honeycomb_is_empty(const Honeycomb* comb);

// Optional: Iterator functions
typedef void (*HoneycombIterator)(const char* key, void* value, void* user_data);
void hashmap_foreach(Honeycomb* map, HoneycombIterator iterator, void* user_data);


#endif // HONEYCOMB_H
