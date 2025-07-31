# Pearl Set Implementation Planning

## Overview
Pearl is a hash set data structure for unique elements with comprehensive functionality including set operations, type-safe wrappers, and performance monitoring.

## Phase 1: Core Infrastructure (Foundation)
- **File**: `libs/datazoo/src/pearl.c`
- **Dependencies**: Samrena arena allocator
- **Goals**: Basic structure creation/destruction and memory management

### Tasks:
1. Implement hash functions (DJB2, FNV1A, Murmur3)
2. Implement `pearl_create()`, `pearl_create_with_hash()`, `pearl_create_custom()`
3. Implement `pearl_destroy()`
4. Implement basic error handling infrastructure
5. Add default equality function for memory comparison

## Phase 2: Basic Set Operations (Core Functionality)  
- **Dependencies**: Phase 1 complete
- **Goals**: Essential set operations working

### Tasks:
1. Implement `pearl_add()` with collision handling via chaining
2. Implement `pearl_contains()` with hash lookup
3. Implement `pearl_remove()` with chain management
4. Implement `pearl_clear()`
5. Implement dynamic resizing when load factor exceeded
6. Add basic statistics tracking

## Phase 3: Utility Functions (Supporting Operations)
- **Dependencies**: Phase 2 complete  
- **Goals**: Size queries, copying, and array conversions

### Tasks:
1. Implement `pearl_size()` and `pearl_is_empty()`
2. Implement `pearl_copy()` with deep copy semantics
3. Implement `pearl_to_array()` for element extraction
4. Implement `pearl_from_array()` for bulk insertion
5. Implement `pearl_foreach()` iterator

## Phase 4: Set Mathematics (Advanced Operations)
- **Dependencies**: Phase 3 complete
- **Goals**: Mathematical set operations

### Tasks:
1. Implement `pearl_union()` - combine two sets
2. Implement `pearl_intersection()` - common elements  
3. Implement `pearl_difference()` - elements in first but not second
4. Implement `pearl_symmetric_difference()` - elements in either but not both
5. Implement subset operations: `pearl_is_subset()`, `pearl_is_superset()`
6. Implement `pearl_is_disjoint()` and `pearl_equals()`

## Phase 5: Performance & Debugging (Monitoring)
- **Dependencies**: Phase 4 complete
- **Goals**: Performance analysis and debugging tools

### Tasks:
1. Enhance statistics collection (collisions, chain lengths, resizes)
2. Implement `pearl_get_stats()`, `pearl_reset_stats()`, `pearl_print_stats()`
3. Add comprehensive error callback system
4. Implement `pearl_error_string()` for error descriptions
5. Add performance optimization for common cases

## Phase 6: Functional Programming (Advanced Features)
- **Dependencies**: Phase 5 complete
- **Goals**: Higher-order functions for set manipulation

### Tasks:
1. Implement `pearl_filter()` with predicate functions
2. Implement `pearl_map()` for element transformation
3. Add memory-efficient implementations
4. Test with complex user-defined predicates

## Phase 7: String Specialization (Type-Safe API)
- **Dependencies**: Phase 6 complete
- **Goals**: Optimized string handling and type-safe wrappers

### Tasks:
1. Implement string-specific hash and equality functions
2. Complete string set specialization functions (declarations at end of header)
3. Test string set with various string types and lengths
4. Optimize for common string operations

## Phase 8: Testing & Integration
- **Dependencies**: All phases complete
- **Goals**: Comprehensive testing and CMake integration

### Tasks:
1. Create comprehensive test suite covering all APIs
2. Add performance benchmarks
3. Test edge cases (empty sets, large sets, collision-heavy scenarios)
4. Memory leak testing with Valgrind
5. Integration testing with existing datazoo components

## Implementation Notes

### Memory Management
- All allocations go through Samrena arena
- No individual free() calls - arena handles cleanup
- Nodes allocated from arena, not malloc

### Hash Function Strategy
- DJB2 as default (simple, fast)
- FNV1A for better distribution
- Murmur3 for cryptographic strength
- Custom functions supported

### Collision Resolution
- Separate chaining with linked lists
- Load factor threshold of 0.75 triggers resize
- Resize doubles capacity and rehashes all elements

### Error Handling
- Comprehensive error codes for all failure modes
- Optional error callbacks for debugging
- Graceful degradation on memory exhaustion

### Type Safety
- Macro system generates type-safe wrappers
- Common types pre-instantiated (int, long, ptr)
- String specialization with optimized operations