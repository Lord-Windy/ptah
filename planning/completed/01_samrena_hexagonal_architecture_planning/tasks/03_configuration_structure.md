# Task: Define Configuration Structure

## Overview
Create a unified configuration structure that can configure any adapter type, providing a consistent way to initialize arenas regardless of the underlying implementation.

## Requirements
- Support all adapter types with single structure
- Sensible defaults for all fields
- Extensible for future adapters
- Type-safe configuration

## Implementation Details

### 1. Define Configuration Structure
```c
typedef struct {
    // Strategy selection
    SamrenaStrategy strategy;
    
    // Common parameters
    uint64_t initial_pages;      // Initial allocation in pages
    uint64_t page_size;          // Page size (0 = default)
    
    // Chained adapter specific
    uint64_t growth_pages;       // Pages to add on expansion
    
    // Virtual adapter specific
    uint64_t max_reserve;        // Maximum virtual space (0 = default)
    uint64_t commit_size;        // Commit granularity (0 = page_size)
    
    // Optional features
    bool enable_stats;           // Track allocation statistics
    bool enable_debug;           // Enable debug features
} SamrenaConfig;
```

### 2. Define Default Configuration
```c
// Get default configuration
static inline SamrenaConfig samrena_default_config(void) {
    return (SamrenaConfig){
        .strategy = SAMRENA_STRATEGY_DEFAULT,
        .initial_pages = 1,
        .page_size = 0,  // Use system default
        .growth_pages = 1,
        .max_reserve = 0,  // Use adapter default
        .commit_size = 0,
        .enable_stats = false,
        .enable_debug = false
    };
}

// Configuration builder helpers
#define SAMRENA_CONFIG_CHAINED(pages) \
    ((SamrenaConfig){ \
        .strategy = SAMRENA_STRATEGY_CHAINED, \
        .initial_pages = pages, \
        .growth_pages = 1 \
    })

#define SAMRENA_CONFIG_VIRTUAL(reserve_mb) \
    ((SamrenaConfig){ \
        .strategy = SAMRENA_STRATEGY_VIRTUAL, \
        .initial_pages = 1, \
        .max_reserve = (reserve_mb) * 1024 * 1024 \
    })
```

## Location
- `libs/samrena/include/samrena.h` - Public configuration API

## Dependencies
- Task 01: Strategy enumeration defined

## Verification
- [ ] All adapter types supported
- [ ] Default values are sensible
- [ ] Builder macros work correctly
- [ ] Documentation clear on each field's purpose

## Notes
- Consider versioning for future compatibility
- Document which fields apply to which strategies
- Validate configuration in factory function