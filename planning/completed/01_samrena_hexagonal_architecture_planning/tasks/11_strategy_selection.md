# Task: Runtime Strategy Selection

## Overview
Implement intelligent runtime strategy selection based on allocation patterns, system resources, and performance requirements.

## Requirements
- Automatic strategy selection based on hints
- Runtime switching between strategies
- Performance-based selection
- Memory pressure awareness

## Implementation Details

### 1. Allocation Hints
```c
// In include/samrena.h
typedef struct {
    // Expected usage pattern
    uint64_t expected_total_size;    // Total memory to be allocated
    uint64_t expected_max_alloc;     // Largest single allocation
    uint32_t expected_alloc_count;   // Number of allocations
    
    // Performance requirements
    bool require_contiguous;         // Need contiguous memory
    bool require_zero_copy_growth;   // Cannot tolerate reallocation
    bool frequent_reset;             // Will reset/clear often
    
    // System constraints
    uint64_t max_memory_limit;       // Hard memory limit
    bool low_memory_mode;            // Optimize for memory usage
} SamrenaAllocationHints;

// Create arena with hints
Samrena* samrena_create_with_hints(const SamrenaAllocationHints* hints);
```

### 2. Strategy Scoring System
```c
// In src/samrena.c
typedef struct {
    SamrenaStrategy strategy;
    int score;
    const char* reason;
} StrategyScore;

static int score_strategy(
    SamrenaStrategy strategy,
    const SamrenaAllocationHints* hints
) {
    int score = 100;  // Base score
    
    switch (strategy) {
    case SAMRENA_STRATEGY_VIRTUAL:
        if (!samrena_strategy_available(strategy)) {
            return -1;  // Not available
        }
        
        // Excellent for contiguous memory
        if (hints->require_contiguous) {
            score += 50;
        }
        
        // Good for large allocations
        if (hints->expected_total_size > 100 * 1024 * 1024) {
            score += 30;
        }
        
        // Penalty for low memory mode
        if (hints->low_memory_mode) {
            score -= 20;  // Reserves address space
        }
        break;
        
    case SAMRENA_STRATEGY_CHAINED:
        if (!samrena_strategy_available(strategy)) {
            return -1;
        }
        
        // Good for incremental growth
        if (hints->expected_alloc_count > 1000) {
            score += 20;
        }
        
        // Excellent for low memory
        if (hints->low_memory_mode) {
            score += 40;
        }
        
        // Penalty for contiguous requirement
        if (hints->require_contiguous) {
            score -= 100;  // Cannot provide
        }
        
        // Good for frequent resets
        if (hints->frequent_reset) {
            score += 20;
        }
        break;
    }
    
    return score;
}

static SamrenaStrategy select_best_strategy(
    const SamrenaAllocationHints* hints
) {
    StrategyScore scores[] = {
        { SAMRENA_STRATEGY_VIRTUAL, 0, NULL },
        { SAMRENA_STRATEGY_CHAINED, 0, NULL },
    };
    
    int best_score = -1;
    SamrenaStrategy best_strategy = SAMRENA_STRATEGY_CHAINED;
    
    for (size_t i = 0; i < sizeof(scores)/sizeof(scores[0]); i++) {
        scores[i].score = score_strategy(scores[i].strategy, hints);
        
        if (scores[i].score > best_score) {
            best_score = scores[i].score;
            best_strategy = scores[i].strategy;
        }
    }
    
    return best_strategy;
}
```

### 3. System Resource Detection
```c
static void detect_system_resources(SamrenaAllocationHints* hints) {
    // Platform-specific memory detection
#ifdef _WIN32
    MEMORYSTATUSEX status = { sizeof(status) };
    if (GlobalMemoryStatusEx(&status)) {
        uint64_t available = status.ullAvailPhys;
        
        // Set low memory mode if less than 1GB available
        if (available < 1024 * 1024 * 1024) {
            hints->low_memory_mode = true;
        }
        
        // Set memory limit to 75% of available
        if (hints->max_memory_limit == 0) {
            hints->max_memory_limit = available * 3 / 4;
        }
    }
#elif defined(__linux__)
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        uint64_t available = info.freeram * info.mem_unit;
        
        if (available < 1024 * 1024 * 1024) {
            hints->low_memory_mode = true;
        }
        
        if (hints->max_memory_limit == 0) {
            hints->max_memory_limit = available * 3 / 4;
        }
    }
#endif
}
```

### 4. Implementation with Hints
```c
Samrena* samrena_create_with_hints(const SamrenaAllocationHints* hints) {
    SamrenaAllocationHints local_hints = hints ? *hints : (SamrenaAllocationHints){0};
    
    // Auto-detect system resources
    detect_system_resources(&local_hints);
    
    // Select best strategy
    SamrenaStrategy strategy = select_best_strategy(&local_hints);
    
    // Build configuration from hints
    SamrenaConfig config = samrena_default_config();
    config.strategy = strategy;
    
    // Configure based on hints
    if (local_hints.expected_total_size > 0) {
        config.initial_pages = (local_hints.expected_total_size + 
                               config.page_size - 1) / config.page_size;
    }
    
    if (strategy == SAMRENA_STRATEGY_VIRTUAL && 
        local_hints.expected_total_size > 0) {
        // Reserve 2x expected size for growth
        config.max_reserve = local_hints.expected_total_size * 2;
    }
    
    // Create arena with selected configuration
    return samrena_create(&config);
}
```

## Location
- `libs/samrena/include/samrena.h` - Hints API
- `libs/samrena/src/samrena.c` - Selection logic

## Dependencies
- Task 10: Factory function implemented

## Verification
- [ ] Selects virtual for large contiguous needs
- [ ] Selects chained for low memory situations
- [ ] Respects all hint parameters
- [ ] System detection works on all platforms
- [ ] Falls back gracefully when preferred unavailable

## Notes
- Consider caching system resource detection
- May want to add profiling-based selection
- Document hint recommendations