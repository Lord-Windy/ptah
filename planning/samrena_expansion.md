# Samrena Expansion: Hexagonal Architecture Design

## Overview
This document outlines the redesign of samrena to support multiple memory allocation strategies through a hexagonal architecture pattern. This will allow users to choose between different memory management implementations (adapters) while maintaining a consistent interface (port).

## Proposed Architecture

### Core Interface (Port)
The samrena interface will be abstracted to support different backing implementations:

```c
// Forward declaration of opaque handle
typedef struct SamrenaImpl SamrenaImpl;

// Arena handle visible to users
typedef struct {
    SamrenaImpl* impl;
    void* context;  // Implementation-specific data
} Samrena;

// Strategy selection
typedef enum {
    SAMRENA_STRATEGY_DEFAULT = 0,  // Let implementation choose
    SAMRENA_STRATEGY_CHAINED,      // Force chained pages
    SAMRENA_STRATEGY_VIRTUAL       // Force virtual memory (if available)
} SamrenaStrategy;

// Operations interface that all adapters must implement
typedef struct {
    const char* name;
    
    // Lifecycle operations
    SamrenaError (*create)(void** context, const void* config);
    void (*destroy)(void* context);
    
    // Memory operations
    void* (*push)(void* context, uint64_t size);
    void* (*push_zero)(void* context, uint64_t size);
    
    // Query operations
    uint64_t (*allocated)(void* context);
    uint64_t (*capacity)(void* context);
    
    // Optional: expansion hint
    SamrenaError (*reserve)(void* context, uint64_t min_capacity);
} SamrenaOps;

// Unified configuration structure
typedef struct {
    SamrenaStrategy strategy;
    uint64_t initial_pages;
    uint64_t growth_pages;      // For chained: pages to add each expansion
    uint64_t max_reserve;        // For virtual: maximum address space (0 = default)
} SamrenaConfig;

// Main factory function
Samrena* samrena_create(const SamrenaConfig* config);

// Convenience function with defaults
Samrena* samrena_allocate(uint64_t initial_pages);

// Destruction
void samrena_destroy(Samrena* arena);
```

### Build Configuration
The CMake build system will detect platform capabilities:

```cmake
# Check for virtual memory support
if(WIN32)
    set(SAMRENA_HAS_VIRTUAL_MEMORY ON)
elseif(UNIX AND NOT APPLE)
    set(SAMRENA_HAS_VIRTUAL_MEMORY ON)
elseif(APPLE)
    set(SAMRENA_HAS_VIRTUAL_MEMORY ON)
else()
    set(SAMRENA_HAS_VIRTUAL_MEMORY OFF)
endif()

# Allow override
option(SAMRENA_ENABLE_VIRTUAL "Enable virtual memory adapter" ${SAMRENA_HAS_VIRTUAL_MEMORY})
```

When virtual memory is disabled or unavailable, `SAMRENA_STRATEGY_VIRTUAL` will automatically fall back to chained pages.

## Adapter Summaries

### Chained Pages Adapter
- **Strategy**: Linked list of memory pages, allocates new pages on demand
- **Growth**: Adds new pages when current page insufficient
- **Use Case**: General purpose, portable arena with dynamic growth
- **Advantages**: 
  - Works on any platform with malloc/free
  - Grows as needed
  - Can handle allocations larger than page size
- **Limitations**:
  - Non-contiguous memory
  - Slight overhead for page management
  - Cannot use simple pointer arithmetic between allocations

### Virtual Memory Adapter
- **Strategy**: Reserve large virtual address space, commit physical pages as needed
- **Growth**: Commits more physical memory within reserved space
- **Use Case**: High-performance arena for modern 64-bit systems
- **Advantages**:
  - Contiguous memory space
  - Zero-copy growth
  - Efficient memory usage
  - Simple pointer arithmetic works
- **Limitations**:
  - Platform-specific (Windows/Linux/macOS)
  - Requires virtual memory support
  - Must choose maximum size upfront

## Factory Behavior

The `samrena_create` function will handle strategy selection as follows:

1. **SAMRENA_STRATEGY_DEFAULT**: 
   - Uses virtual memory adapter if available and enabled
   - Falls back to chained pages otherwise

2. **SAMRENA_STRATEGY_CHAINED**:
   - Always uses chained pages adapter

3. **SAMRENA_STRATEGY_VIRTUAL**:
   - Uses virtual memory if available
   - Falls back to chained pages if virtual memory disabled or unsupported
   - Logs a warning about the fallback

## Benefits of This Approach

1. **Flexibility**: Users choose the best strategy for their use case
2. **Portability**: Works on all platforms with automatic fallback
3. **Extensibility**: New adapters can be added without changing the interface
4. **Testability**: Each adapter can be tested in isolation
5. **Performance**: No virtual function overhead - adapter chosen at creation time

## Implementation Priority

1. Define and implement the core interface
2. Implement chained pages adapter (works everywhere)
3. Implement virtual memory adapter (platform-specific)
4. Add CMake detection and configuration
5. Create comprehensive test suite for all adapters