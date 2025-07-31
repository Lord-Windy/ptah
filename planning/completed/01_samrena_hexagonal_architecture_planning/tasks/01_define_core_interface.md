# Task: Define Core Interface

## Overview
Design the hexagonal port interface that all samrena adapters will implement. This establishes the contract between users and the various memory allocation strategies.

## Requirements
- Opaque handle design for implementation hiding
- Clean separation between interface and implementation
- Support for multiple concurrent arenas
- Thread-safe design considerations

## Implementation Details

### 1. Define Opaque Types
```c
// Forward declaration of implementation
typedef struct SamrenaImpl SamrenaImpl;

// User-visible arena handle
typedef struct {
    SamrenaImpl* impl;
    void* context;  // Implementation-specific data
} Samrena;
```

### 2. Define Error Handling
```c
typedef enum {
    SAMRENA_OK = 0,
    SAMRENA_ERROR_OUT_OF_MEMORY,
    SAMRENA_ERROR_INVALID_PARAMETER,
    SAMRENA_ERROR_UNSUPPORTED_STRATEGY,
    SAMRENA_ERROR_PLATFORM_SPECIFIC
} SamrenaError;
```

### 3. Define Strategy Enumeration
```c
typedef enum {
    SAMRENA_STRATEGY_DEFAULT = 0,
    SAMRENA_STRATEGY_CHAINED,
    SAMRENA_STRATEGY_VIRTUAL
} SamrenaStrategy;
```

## Location
- `libs/samrena/include/samrena.h` - Public interface
- `libs/samrena/src/samrena_internal.h` - Internal definitions

## Dependencies
- None (first task)

## Verification
- [ ] Compiles without errors
- [ ] No implementation details exposed in public header
- [ ] Clear documentation for each type
- [ ] Consistent naming conventions

## Notes
- Keep interface minimal and focused
- Consider future extensibility
- Document thread safety requirements