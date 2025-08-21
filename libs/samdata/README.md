# SamData

A high-performance, arena-backed data structures library for C, providing hash maps, sets, and hash functions with zero heap allocations after initialization.

## Features

- **Arena-Backed Memory Management**: All data structures use Samrena memory arenas, eliminating heap fragmentation
- **Multiple Hash Functions**: Choose between DJB2, FNV1A, and Murmur3 hash algorithms
- **Type-Safe Wrappers**: Macro-based type-safe wrappers for common types
- **Performance Metrics**: Built-in statistics tracking for collisions, resizing, and operations
- **Error Handling**: Comprehensive error reporting with callback support
- **Zero Copy Operations**: Efficient memory usage through arena allocation

## Data Structures

### SamHashMap
A high-performance hash map implementation with:
- Chaining collision resolution
- Dynamic resizing with configurable load factor
- String keys with arbitrary value types
- Iterator support for traversal
- Performance statistics tracking

### SamSet
A set data structure for storing unique elements with:
- Generic element support (any data type)
- Custom hash and equality functions
- Functional programming operations (filter, map)
- Array conversion utilities
- Set operations and iterators

### SamHash
A collection of hash functions:
- **DJB2**: Fast, simple hash by Dan Bernstein
- **FNV1A**: Fowler-Noll-Vo hash, excellent distribution
- **Murmur3**: High-performance, non-cryptographic hash

## Installation

SamData is part of the Ptah monorepo and requires Samrena for memory management.

```bash
# Clone the repository
git clone <repository-url>
cd ptah

# Build the project
mkdir build && cd build
cmake ..
make

# Run tests
ctest
```

## Quick Start

### Basic HashMap Usage

```c
#include <samdata.h>
#include <samrena.h>

int main() {
    // Create an arena for memory management
    Samrena *arena = samrena_create_default(1024 * 1024); // 1MB arena
    
    // Create a hash map with initial capacity of 16 buckets
    SamHashMap *map = samhashmap_create(16, arena);
    
    // Add key-value pairs
    samhashmap_put(map, "name", "Alice");
    samhashmap_put(map, "city", "Boston");
    samhashmap_put(map, "age", "30");
    
    // Retrieve values
    const char *name = samhashmap_get(map, "name");
    printf("Name: %s\n", name);
    
    // Check if key exists
    if (samhashmap_contains(map, "city")) {
        printf("City: %s\n", (char*)samhashmap_get(map, "city"));
    }
    
    // Get map size
    printf("Map contains %zu entries\n", samhashmap_size(map));
    
    // Iterate over all entries
    void print_entry(const char *key, void *value, void *user_data) {
        printf("%s: %s\n", key, (char*)value);
    }
    samhashmap_foreach(map, print_entry, NULL);
    
    // Clean up (arena handles all memory)
    samrena_destroy(arena);
    return 0;
}
```

### Type-Safe HashMap

```c
// Use predefined type-safe wrappers
string_string_samhashmap *config = string_string_create(32, arena);

string_string_put(config, "host", "localhost");
string_string_put(config, "port", "8080");

const char *host = string_string_get(config, "host");
```

### Set Operations

```c
// Create a set of integers
SamSet *numbers = samset_create(sizeof(int), 16, arena);

// Add elements
int values[] = {10, 20, 30, 20, 40}; // Note: 20 appears twice
for (int i = 0; i < 5; i++) {
    samset_add(numbers, &values[i]);
}

// Set will only contain unique elements
printf("Set size: %zu\n", samset_size(numbers)); // Output: 4

// Check membership
int check = 20;
if (samset_contains(numbers, &check)) {
    printf("%d is in the set\n", check);
}

// Remove element
samset_remove(numbers, &check);

// Convert to array
int array[10];
size_t count = samset_to_array(numbers, array, 10);
```

### Custom Hash Functions

```c
// Create hashmap with specific hash function
SamHashMap *map = samhashmap_create_with_hash(
    16, arena, SAMHASHMAP_HASH_MURMUR3
);

// For sets with custom hash/equality
uint32_t my_hash(const void *data, size_t size) {
    return samhash_murmur3(data, size);
}

bool my_equals(const void *a, const void *b, size_t size) {
    return memcmp(a, b, size) == 0;
}

SamSet *custom_set = samset_create_custom(
    sizeof(MyStruct), 16, arena, my_hash, my_equals
);
```

### Performance Monitoring

```c
// Enable statistics tracking
SamHashMap *map = samhashmap_create(16, arena);

// ... perform operations ...

// Get performance metrics
SamHashMapStats stats = samhashmap_get_stats(map);
printf("Total collisions: %zu\n", stats.total_collisions);
printf("Max chain length: %zu\n", stats.max_chain_length);
printf("Resize count: %zu\n", stats.resize_count);
printf("Average chain length: %.2f\n", stats.average_chain_length);

// Print formatted statistics
samhashmap_print_stats(map);
```

### Error Handling

```c
// Set up error callback
void handle_error(SamHashMapError error, const char *message, void *data) {
    fprintf(stderr, "HashMap Error %d: %s\n", error, message);
}

samhashmap_set_error_callback(map, handle_error, NULL);

// Check last error
if (samhashmap_get_last_error(map) != SAMHASHMAP_ERROR_NONE) {
    const char *error_str = samhashmap_error_string(
        samhashmap_get_last_error(map)
    );
    printf("Last error: %s\n", error_str);
}
```

## Advanced Usage

### Functional Programming with Sets

```c
// Filter set elements
bool is_even(const void *element, void *user_data) {
    return (*(int*)element) % 2 == 0;
}

SamSet *even_numbers = samset_filter(numbers, is_even, NULL, arena);

// Map transformation
void double_value(const void *in, void *out, void *user_data) {
    *(int*)out = (*(int*)in) * 2;
}

SamSet *doubled = samset_map(numbers, double_value, sizeof(int), NULL, arena);
```

### Working with Hash Functions Directly

```c
#include <samdata/samhash.h>

// Hash arbitrary data
uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
uint32_t hash1 = samhash_djb2(data, sizeof(data));
uint32_t hash2 = samhash_fnv1a(data, sizeof(data));
uint32_t hash3 = samhash_murmur3(data, sizeof(data));

// Hash strings
const char *text = "Hello, World!";
uint32_t str_hash = samhash_string_murmur3(text);

// Generic hash interface
uint32_t generic_hash = samhash(data, sizeof(data), SAMHASH_FNV1A);
```

## API Reference

### SamHashMap Core Functions

| Function | Description |
|----------|-------------|
| `samhashmap_create()` | Create new hash map |
| `samhashmap_put()` | Insert/update key-value pair |
| `samhashmap_get()` | Retrieve value by key |
| `samhashmap_remove()` | Remove key-value pair |
| `samhashmap_contains()` | Check if key exists |
| `samhashmap_clear()` | Remove all entries |
| `samhashmap_size()` | Get number of entries |
| `samhashmap_foreach()` | Iterate over entries |

### SamSet Core Functions

| Function | Description |
|----------|-------------|
| `samset_create()` | Create new set |
| `samset_add()` | Add element to set |
| `samset_remove()` | Remove element from set |
| `samset_contains()` | Check if element exists |
| `samset_clear()` | Remove all elements |
| `samset_size()` | Get number of elements |
| `samset_foreach()` | Iterate over elements |
| `samset_filter()` | Create filtered subset |
| `samset_map()` | Transform elements |

### Hash Functions

| Function | Algorithm | Use Case |
|----------|-----------|----------|
| `samhash_djb2()` | DJB2 | Fast, simple hashing |
| `samhash_fnv1a()` | FNV-1a | Good distribution |
| `samhash_murmur3()` | MurmurHash3 | High performance |

## Performance Characteristics

### Time Complexity

| Operation | HashMap | Set |
|-----------|---------|-----|
| Insert | O(1) average, O(n) worst | O(1) average, O(n) worst |
| Lookup | O(1) average, O(n) worst | O(1) average, O(n) worst |
| Delete | O(1) average, O(n) worst | O(1) average, O(n) worst |
| Iteration | O(n) | O(n) |

### Memory Usage

- **Zero heap allocations** after initialization
- All memory managed through Samrena arenas
- Automatic resizing when load factor exceeded (default 0.75)
- Memory reclaimed when arena is destroyed

## Thread Safety

SamData structures are **not thread-safe**. For concurrent access:
- Use external synchronization (mutexes, read-write locks)
- Create separate instances per thread with thread-local arenas
- Consider lock-free alternatives for high-contention scenarios

## Best Practices

1. **Arena Sizing**: Allocate sufficient arena space upfront to avoid exhaustion
2. **Load Factor**: Adjust load factor based on your collision tolerance (default 0.75)
3. **Hash Function**: Choose based on your data:
   - DJB2: Simple strings, speed critical
   - FNV1A: Better distribution, slightly slower
   - Murmur3: Best for large data, highest quality
4. **Error Handling**: Always check return values in production code
5. **Statistics**: Monitor performance metrics in development to tune parameters

## Examples

See the `test/` directory for comprehensive examples:
- `samhashmap_tests.c` - HashMap usage examples
- `samset_*_test.c` - Various set operation examples
- `samdata_test.c` - Integration tests

## License

Copyright 2025 Samuel "Lord-Windy" Brown

Licensed under the Apache License, Version 2.0. See LICENSE file for details.