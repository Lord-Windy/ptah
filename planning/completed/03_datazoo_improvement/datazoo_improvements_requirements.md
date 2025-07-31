# Requirements: Datazoo Library Improvements (Updated)

## Overview

Enhance the datazoo hash map library (Honeycomb) to improve performance,
safety, functionality, and maintainability using exclusively samrena
arena-based memory allocation with safe key management and optional type
safety.

## Functional Requirements

### Core Functionality Improvements

- **FR1**: Dynamic Resizing with Arena Allocation
  - Input: Hash map reaching load factor threshold (default 0.75)
  - Output: Automatically resized hash map with redistributed entries
  - Constraints: Must use samrena for all allocations, maintain O(1) amortized insertion

- **FR2**: Safe Key Management with Copying
  - Input: String keys provided by users
  - Output: Keys copied into arena-allocated memory owned by hash map
  - Constraints: All keys must be duplicated using samrena allocation

- **FR3**: Generic Type-Safe Macros
  - Input: Type-specific hash map declarations
  - Output: Type-safe operations via macro expansion
  - Constraints: Fallback to void* if macro approach proves impractical

- **FR4**: Arena-Based Iterator
  - Input: Hash map instance
  - Output: Iterator supporting traversal of all key-value pairs
  - Constraints: Iterator state allocated via samrena

### Enhanced API

- **API1**: Simplified Creation
  - Input: Initial capacity, load factor, and required samrena instance
  - Output: Initialized hash map
  - Constraints: Samrena parameter is mandatory (not nullable)

- **API2**: Clear Operation
  - Input: Hash map instance
  - Output: Empty hash map retaining capacity
  - Constraints: No individual deallocations (arena handles memory)

- **API3**: Bulk Operations
  - Input: Array of key-value pairs
  - Output: Efficient batch insertion
  - Constraints: Single resize operation if needed

### Error Handling

- **ERR1**: Null Parameter Validation
  - Input: NULL key, value, or samrena
  - Output: Return error code or false
  - Constraints: Must not crash, clear error reporting

- **ERR2**: Arena Exhaustion
  - Input: Samrena allocation failure
  - Output: Graceful operation failure
  - Constraints: Hash map remains in consistent state

## Non-Functional Requirements

### Performance

- **NFR1**: O(1) average case for get/put/remove operations
- **NFR2**: Resize operation in O(n) time using arena bulk allocation
- **NFR3**: Zero memory fragmentation (arena-based allocation)

### Safety

- **NFR4**: Single-threaded design (clearly documented)
- **NFR5**: No memory leaks possible (arena cleanup handles all)
- **NFR6**: Const-correctness throughout API

### Code Quality

- **NFR7**: Fix header guard naming (DATAZOO_H not ZOOKEEPER_H)
- **NFR8**: Complete test coverage using samrena
- **NFR9**: Modular hash function design for future customization

## Domain Model

### Key Entities
- **Honeycomb**: Main hash map requiring samrena instance
- **Cell**: Key-value pair with arena-allocated copied key
- **TypedHoneycomb**: Macro-generated type-safe wrapper (if feasible)
- **HoneycombIterator**: Stateful traversal object

### Relationships
- Honeycomb exclusively uses provided Samrena for all allocations
- All Cells allocated from same arena as Honeycomb
- Keys are always copied into arena memory
- Iterator lifetime tied to Honeycomb lifetime

## Acceptance Criteria

1. Samrena is the only memory allocation mechanism
2. All string keys are copied into arena memory
3. Hash map resizes automatically at configurable load factor
4. Type-safe macro layer provides compile-time type checking (or clear void* API)
5. No manual memory management required by users
6. Breaking API changes implemented cleanly
7. Test suite validates:
   - Key copying behavior
   - Resize operations
   - Arena memory usage patterns
   - Edge cases with NULL parameters
   - Large dataset handling

## Examples

### Arena-Only Creation
```c
Samrena *arena = samrena_allocate(100);
// Samrena is required, not optional
Honeycomb *comb = honeycomb_create(16, 0.75, arena);
```

### Safe Key Copying
```c
char temp_key[32];
sprintf(temp_key, "key_%d", i);
honeycomb_put(comb, temp_key, value);
// temp_key can be modified/freed - honeycomb has its own copy
```

### Type-Safe Macro Usage (if implemented)
```c
// Define type-safe hash map for int values
HONEYCOMB_DEFINE(IntMap, int)

Samrena *arena = samrena_allocate(100);
IntMap *map = intmap_create(16, 0.75, arena);
intmap_put(map, "count", 42);
int *value = intmap_get(map, "count");
```

### Iterator with Arena Allocation
```c
HoneycombIterator *iter = honeycomb_iterator_create(comb);
const char *key;
void *value;
while (honeycomb_iterator_next(iter, &key, &value)) {
    // Process pair - key is arena-allocated copy
}
// No need to destroy iterator - arena cleanup handles it
```
