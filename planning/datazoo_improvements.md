# Datazoo (Honeycomb) Hashmap Improvements

This document outlines potential improvements for the Honeycomb hashmap implementation in the datazoo library.

## Current Strengths

Before diving into improvements, it's worth noting what the current implementation does well:

- **Arena-based memory management**: Using samrena for all allocations provides efficient memory usage and simplified cleanup
- **Clean API design**: Function names are intuitive and follow consistent naming conventions
- **Collision handling**: Separate chaining with linked lists is a proven approach
- **Dynamic resizing**: Automatically grows when load factor exceeds 0.75
- **Safety checks**: Proper null validation throughout the codebase

## Critical Bugs to Fix

### 1. Memory Waste in honeycomb_put()

**Issue**: The function allocates a new cell before checking if the key already exists, leading to memory waste when updating existing keys.

```c
// Current problematic code (lines 126-130):
Cell *new_cell = honeycomb_cell_create(comb->arena, key, value);
if (new_cell == NULL) {
    return false;
}
// ... later checks if key exists
```

**Solution**: Check for existing keys first, only allocate new cell if needed:
```c
// First, check if key exists
Cell *current = comb->cells[hash_bucket];
while (current != NULL) {
    if (strcmp(current->key, key) == 0) {
        current->value = value;
        return true;
    }
    current = current->next;
}

// Only now create new cell
Cell *new_cell = honeycomb_cell_create(comb->arena, key, value);
```

## Missing Core Functionality

### 2. Iterator Implementation

The header declares `hashmap_foreach` but it's never implemented. This is essential for iterating over all key-value pairs.

**Suggested implementation**:
```c
void honeycomb_foreach(const Honeycomb *map, HoneycombIterator iterator, void *user_data) {
    if (map == NULL || iterator == NULL) return;
    
    for (size_t i = 0; i < map->capacity; i++) {
        Cell *current = map->cells[i];
        while (current != NULL) {
            iterator(current->key, current->value, user_data);
            current = current->next;
        }
    }
}
```

### 3. Clear Operation

No way to remove all entries without destroying the hashmap.

**Suggested addition**:
```c
void honeycomb_clear(Honeycomb *comb) {
    if (comb == NULL) return;
    
    // Zero out all buckets
    memset(comb->cells, 0, sizeof(Cell *) * comb->capacity);
    comb->size = 0;
    // Note: Memory remains allocated in arena
}
```

### 4. Key/Value Collection

Add functions to get all keys or values as an array:
```c
// Get all keys (caller must ensure array is large enough)
size_t honeycomb_get_keys(const Honeycomb *comb, const char **keys, size_t max_keys);

// Get all values
size_t honeycomb_get_values(const Honeycomb *comb, void **values, size_t max_values);
```

## Type Safety and Flexibility Improvements

### 5. Generic Key Support

Currently limited to string keys. Consider supporting arbitrary key types:

```c
typedef struct {
    // ... existing fields ...
    size_t (*hash_func)(const void *key, size_t capacity);
    bool (*equals_func)(const void *key1, const void *key2);
    size_t key_size;  // For non-string keys
} Honeycomb;

// Alternative constructor for custom types
Honeycomb *honeycomb_create_custom(size_t initial_capacity, 
                                  Samrena *samrena,
                                  size_t (*hash_func)(const void *, size_t),
                                  bool (*equals_func)(const void *, const void *),
                                  size_t key_size);
```

### 6. Type-Safe Wrappers

Add macros for type safety:
```c
#define HONEYCOMB_DEFINE_TYPED(name, key_type, value_type) \
    typedef struct name##_honeycomb { \
        Honeycomb *base; \
    } name##_honeycomb; \
    \
    static inline bool name##_put(name##_honeycomb *h, key_type key, value_type value) { \
        return honeycomb_put(h->base, (const char*)key, (void*)value); \
    } \
    /* ... other typed functions ... */
```

## Performance Enhancements

### 7. Shrinking Support

The hashmap only grows but never shrinks. Add shrinking when size falls below a threshold:

```c
// In honeycomb_remove():
if (comb->size < comb->capacity * 0.25f && comb->capacity > INITIAL_CAPACITY) {
    honeycomb_shrink(comb);
}
```

### 8. Better Hash Distribution

While djb2 is decent, consider:
- Supporting multiple hash functions (FNV-1a, MurmurHash, etc.)
- Using a better default for modern systems
- Adding hash quality metrics

### 9. Performance Metrics

Add statistics tracking:
```c
typedef struct {
    size_t total_collisions;
    size_t max_chain_length;
    size_t resize_count;
    double average_chain_length;
} HoneycombStats;

HoneycombStats honeycomb_get_stats(const Honeycomb *comb);
```

## Error Handling Improvements

### 10. Better Resize Failure Handling

Currently, resize failures are silently ignored (line 119-121). Consider:
- Returning error codes from functions
- Adding an error callback mechanism
- Logging resize failures

### 11. Memory Allocation Failure Recovery

When arena allocation fails, consider strategies like:
- Pre-allocating emergency space
- Providing a fallback allocator
- Better error reporting to caller

## Advanced Features

### 12. Thread Safety Options

Add optional thread safety:
```c
typedef struct {
    Honeycomb base;
    pthread_rwlock_t lock;  // Or platform-specific lock
} ThreadSafeHoneycomb;

// Thread-safe variants of all operations
```

### 13. Configurable Load Factor

Allow runtime configuration:
```c
void honeycomb_set_load_factor(Honeycomb *comb, float factor);
```

### 14. Serialization Support

Add ability to save/load hashmaps:
```c
// Save to buffer
size_t honeycomb_serialize(const Honeycomb *comb, uint8_t *buffer, size_t buffer_size);

// Load from buffer
Honeycomb *honeycomb_deserialize(const uint8_t *buffer, size_t size, Samrena *arena);
```

### 15. Ordered Iteration

Maintain insertion order for predictable iteration:
```c
typedef struct {
    Cell **cells;
    Cell *first;  // First inserted
    Cell *last;   // Last inserted
    // ... other fields ...
} Honeycomb;
```

## Testing and Validation

### 16. Comprehensive Test Suite

Add tests for:
- Edge cases (empty hashmap, single element, etc.)
- Collision handling
- Resize behavior
- Memory exhaustion scenarios
- Performance benchmarks

### 17. Debugging Support

Add debug builds with:
- Invariant checking
- Memory usage tracking
- Collision visualization
- Chain length histograms

## Priority Recommendations

**High Priority** (fix immediately):
1. Fix the memory waste bug in `honeycomb_put`
2. Implement the missing `hashmap_foreach` function
3. Add `honeycomb_clear` function

**Medium Priority** (significant improvements):
4. Add generic key support
5. Implement shrinking
6. Add basic performance metrics

**Low Priority** (nice to have):
7. Thread safety options
8. Serialization support
9. Ordered iteration

These improvements would make the Honeycomb hashmap more robust, flexible, and suitable for production use while maintaining its clean design and efficient memory management through samrena.