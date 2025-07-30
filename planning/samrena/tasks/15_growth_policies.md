# Task: Implement Growth Policies

## Overview
Create configurable growth policies that control how arenas expand when they need more memory, optimizing for different allocation patterns.

## Requirements
- Multiple growth strategies
- Runtime policy selection
- Adaptive growth based on patterns
- Memory pressure awareness

## Implementation Details

### 1. Growth Policy Types
```c
// In include/samrena.h
typedef enum {
    SAMRENA_GROWTH_LINEAR,      // Add fixed amount each time
    SAMRENA_GROWTH_EXPONENTIAL, // Double size each time
    SAMRENA_GROWTH_FIBONACCI,   // Fibonacci sequence growth
    SAMRENA_GROWTH_ADAPTIVE,    // Learn from allocation pattern
    SAMRENA_GROWTH_CUSTOM       // User-defined function
} SamrenaGrowthPolicy;

typedef struct {
    SamrenaGrowthPolicy policy;
    union {
        struct {
            uint64_t increment;  // For linear growth
        } linear;
        struct {
            double factor;       // For exponential (default 2.0)
            uint64_t max_step;   // Maximum single growth
        } exponential;
        struct {
            uint64_t fib_a;      // Fibonacci state
            uint64_t fib_b;
        } fibonacci;
        struct {
            uint64_t window_size;     // Allocation history window
            double aggressiveness;    // 0.0 = conservative, 1.0 = aggressive
        } adaptive;
    } params;
    
    // Custom growth function
    uint64_t (*custom_growth)(void* context, uint64_t current_size, 
                             uint64_t requested_size, void* user_data);
    void* custom_user_data;
} SamrenaGrowthConfig;
```

### 2. Growth Calculator
```c
// In src/samrena.c
typedef struct {
    // Allocation history for adaptive growth
    uint64_t* allocation_sizes;
    size_t history_size;
    size_t history_capacity;
    size_t history_index;
    
    // Statistics
    uint64_t total_growths;
    uint64_t total_wasted;
    uint64_t allocation_count;
} GrowthState;

static uint64_t calculate_growth(
    const SamrenaGrowthConfig* config,
    GrowthState* state,
    uint64_t current_size,
    uint64_t requested_size
) {
    uint64_t min_growth = requested_size - current_size;
    uint64_t new_size = current_size;
    
    switch (config->policy) {
    case SAMRENA_GROWTH_LINEAR:
        new_size += config->params.linear.increment;
        break;
        
    case SAMRENA_GROWTH_EXPONENTIAL:
        new_size = (uint64_t)(current_size * config->params.exponential.factor);
        if (new_size - current_size > config->params.exponential.max_step) {
            new_size = current_size + config->params.exponential.max_step;
        }
        break;
        
    case SAMRENA_GROWTH_FIBONACCI:
        {
            uint64_t next = config->params.fibonacci.fib_a + 
                           config->params.fibonacci.fib_b;
            config->params.fibonacci.fib_a = config->params.fibonacci.fib_b;
            config->params.fibonacci.fib_b = next;
            new_size += next;
        }
        break;
        
    case SAMRENA_GROWTH_ADAPTIVE:
        new_size += calculate_adaptive_growth(config, state, 
                                            current_size, requested_size);
        break;
        
    case SAMRENA_GROWTH_CUSTOM:
        if (config->custom_growth) {
            new_size = config->custom_growth(state, current_size, 
                                           requested_size, 
                                           config->custom_user_data);
        }
        break;
    }
    
    // Ensure minimum growth
    if (new_size < current_size + min_growth) {
        new_size = current_size + min_growth;
    }
    
    return new_size - current_size;
}
```

### 3. Adaptive Growth Implementation
```c
static uint64_t calculate_adaptive_growth(
    const SamrenaGrowthConfig* config,
    GrowthState* state,
    uint64_t current_size,
    uint64_t requested_size
) {
    // Track allocation size
    if (state->history_size < state->history_capacity) {
        state->allocation_sizes[state->history_size++] = requested_size;
    } else {
        // Circular buffer
        state->allocation_sizes[state->history_index] = requested_size;
        state->history_index = (state->history_index + 1) % state->history_capacity;
    }
    
    // Calculate average allocation size
    uint64_t total = 0;
    uint64_t max_alloc = 0;
    size_t count = state->history_size;
    
    for (size_t i = 0; i < count; i++) {
        uint64_t size = state->allocation_sizes[i];
        total += size;
        if (size > max_alloc) {
            max_alloc = size;
        }
    }
    
    uint64_t avg_alloc = count > 0 ? total / count : requested_size;
    
    // Predict future needs
    double prediction_factor = 1.0 + config->params.adaptive.aggressiveness;
    uint64_t predicted_need = (uint64_t)(avg_alloc * count * prediction_factor);
    
    // Calculate growth to accommodate predicted needs
    uint64_t growth = predicted_need;
    
    // Consider maximum seen allocation
    if (max_alloc * 2 > growth) {
        growth = max_alloc * 2;
    }
    
    // Minimum growth to handle current request
    uint64_t min_growth = requested_size * 2;
    if (growth < min_growth) {
        growth = min_growth;
    }
    
    return growth;
}
```

### 4. Integration with Adapters
```c
// Extended configuration
typedef struct {
    // ... existing fields ...
    
    // Growth policy
    SamrenaGrowthConfig growth;
} SamrenaConfig;

// In chained adapter
static ChainedPage* chained_grow(ChainedContext* ctx, uint64_t min_size) {
    uint64_t current_capacity = calculate_total_capacity(ctx);
    uint64_t growth_size = calculate_growth(
        &ctx->growth_config,
        &ctx->growth_state,
        current_capacity,
        current_capacity + min_size
    );
    
    // Ensure minimum size
    if (growth_size < min_size) {
        growth_size = min_size;
    }
    
    return chained_add_page(ctx, growth_size);
}
```

### 5. Preset Growth Policies
```c
// Common growth configurations
static const SamrenaGrowthConfig GROWTH_CONSERVATIVE = {
    .policy = SAMRENA_GROWTH_LINEAR,
    .params.linear = { .increment = 65536 }  // 64KB at a time
};

static const SamrenaGrowthConfig GROWTH_BALANCED = {
    .policy = SAMRENA_GROWTH_EXPONENTIAL,
    .params.exponential = { .factor = 1.5, .max_step = 10485760 }  // 10MB max
};

static const SamrenaGrowthConfig GROWTH_AGGRESSIVE = {
    .policy = SAMRENA_GROWTH_EXPONENTIAL,
    .params.exponential = { .factor = 2.0, .max_step = UINT64_MAX }
};

static const SamrenaGrowthConfig GROWTH_SMART = {
    .policy = SAMRENA_GROWTH_ADAPTIVE,
    .params.adaptive = { .window_size = 100, .aggressiveness = 0.5 }
};

// Builder function
Samrena* samrena_create_with_growth(
    uint64_t initial_pages,
    const SamrenaGrowthConfig* growth
) {
    SamrenaConfig config = samrena_default_config();
    config.initial_pages = initial_pages;
    config.growth = growth ? *growth : GROWTH_BALANCED;
    return samrena_create(&config);
}
```

## Location
- `libs/samrena/include/samrena.h` - Growth policy API
- `libs/samrena/src/samrena.c` - Growth calculations
- `libs/samrena/src/adapter_chained.c` - Integration

## Dependencies
- Task 05: Chained adapter supports growth

## Verification
- [ ] All growth policies work correctly
- [ ] Adaptive growth learns from patterns
- [ ] Custom growth functions called properly
- [ ] Memory waste minimized
- [ ] Performance scales with growth policy

## Notes
- Consider memory fragmentation with aggressive growth
- Monitor growth patterns in production
- Document trade-offs of each policy