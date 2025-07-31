# Task: Conditional Compilation

## Overview
Implement conditional compilation throughout the codebase to include/exclude virtual memory adapter based on platform capabilities and configuration.

## Requirements
- Clean separation of platform-specific code
- No virtual memory code in builds without support
- Minimal ifdef usage in main code
- Clear error messages when unavailable

## Implementation Details

### 1. Adapter Registry
```c
// In src/samrena.c
#include "samrena_config.h"
#include "samrena_internal.h"

// Adapter registry
typedef struct {
    SamrenaStrategy strategy;
    const SamrenaOps* ops;
    const char* name;
} AdapterEntry;

static const AdapterEntry adapters[] = {
    { SAMRENA_STRATEGY_CHAINED, &chained_ops, "chained" },
#ifdef SAMRENA_ENABLE_VIRTUAL
    { SAMRENA_STRATEGY_VIRTUAL, &virtual_ops, "virtual" },
#endif
    { 0, NULL, NULL }  // Sentinel
};

static const SamrenaOps* find_adapter(SamrenaStrategy strategy) {
    for (const AdapterEntry* entry = adapters; entry->ops; entry++) {
        if (entry->strategy == strategy) {
            return entry->ops;
        }
    }
    return NULL;
}
```

### 2. Strategy Selection Logic
```c
static SamrenaStrategy select_strategy(SamrenaStrategy requested) {
    switch (requested) {
    case SAMRENA_STRATEGY_DEFAULT:
#ifdef SAMRENA_ENABLE_VIRTUAL
        // Prefer virtual memory when available
        return SAMRENA_STRATEGY_VIRTUAL;
#else
        return SAMRENA_STRATEGY_CHAINED;
#endif
    
    case SAMRENA_STRATEGY_VIRTUAL:
#ifdef SAMRENA_ENABLE_VIRTUAL
        return SAMRENA_STRATEGY_VIRTUAL;
#else
        // Fall back with warning
        fprintf(stderr, "samrena: Virtual memory adapter not available, "
                        "using chained adapter\n");
        return SAMRENA_STRATEGY_CHAINED;
#endif
    
    default:
        return requested;
    }
}
```

### 3. Capability Query API
```c
// In include/samrena.h
// Check if a strategy is available at runtime
bool samrena_strategy_available(SamrenaStrategy strategy);

// Get list of available strategies
int samrena_available_strategies(SamrenaStrategy* strategies, int max_count);

// Get human-readable name for strategy
const char* samrena_strategy_name(SamrenaStrategy strategy);

// Implementation
bool samrena_strategy_available(SamrenaStrategy strategy) {
    return find_adapter(strategy) != NULL;
}

int samrena_available_strategies(SamrenaStrategy* strategies, int max_count) {
    int count = 0;
    for (const AdapterEntry* entry = adapters; 
         entry->ops && count < max_count; 
         entry++) {
        if (strategies) {
            strategies[count] = entry->strategy;
        }
        count++;
    }
    return count;
}
```

### 4. Build-time Feature Test
```c
// In test/test_features.c
#include "samrena.h"
#include <stdio.h>

void test_available_features(void) {
    printf("Samrena build configuration:\n");
    
#ifdef SAMRENA_ENABLE_VIRTUAL
    printf("  Virtual memory: ENABLED (%s)\n", SAMRENA_VM_PLATFORM);
#else
    printf("  Virtual memory: DISABLED\n");
#endif
    
    printf("\nAvailable strategies:\n");
    SamrenaStrategy strategies[10];
    int count = samrena_available_strategies(strategies, 10);
    
    for (int i = 0; i < count; i++) {
        printf("  - %s\n", samrena_strategy_name(strategies[i]));
    }
}
```

## Location
- `libs/samrena/src/samrena.c` - Main implementation
- `libs/samrena/include/samrena.h` - Capability query API
- `libs/samrena/test/test_features.c` - Feature test

## Dependencies
- Task 07: CMake configuration generates SAMRENA_ENABLE_VIRTUAL

## Verification
- [ ] Builds without virtual memory code when disabled
- [ ] No undefined symbols when virtual disabled
- [ ] Capability queries work correctly
- [ ] Clear messages on fallback
- [ ] Test passes on all platforms

## Notes
- Keep platform-specific code isolated
- Consider runtime feature detection in future
- Document compile-time options clearly