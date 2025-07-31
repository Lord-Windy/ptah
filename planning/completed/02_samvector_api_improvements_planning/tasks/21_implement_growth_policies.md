# Task 21: Implement Growth Policies

## Objective
Implement the growth strategy system designed in Task 20, providing configurable and adaptive capacity management that optimizes memory allocation patterns for different usage scenarios.

## Dependencies
- Task 20: Design Growth Strategies (must be completed)
- Task 04: Implement Capacity Functions (for capacity management)
- Task 08: Implement Error Handling (for error reporting)
- Understanding of arena allocation patterns

## Implementation Plan

### 1. Core Growth System Implementation
```c
// samrena_vector_growth.c - Growth policy implementation

#include "samrena_vector.h"
#include <time.h>
#include <math.h>

// Time utilities for adaptive strategies
static uint64_t get_current_timestamp(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// Initialize growth statistics
static void init_growth_stats(SamrenaGrowthStats* stats) {
    memset(stats, 0, sizeof(*stats));
    stats->average_utilization = 1.0;  // Start with full utilization
}

// Update vector structure to include growth management
typedef struct {
    // Standard vector fields
    uint64_t size;
    uint64_t element_size;
    uint64_t capacity;
    void *data;
    Samrena *arena;
    bool owns_arena;
    float growth_factor;    // Keep for compatibility
    size_t min_growth;      // Keep for compatibility
    bool can_shrink;
    
    // Enhanced growth management
    SamrenaGrowthPolicy* growth_policy;
    SamrenaGrowthStats growth_stats;
    SamrenaShrinkPolicy shrink_policy;
    uint64_t last_access_time;
    bool auto_shrink;
    bool track_statistics;
} SamrenaVectorWithGrowth;

// Helper to cast vector to extended version
static inline SamrenaVectorWithGrowth* get_extended_vector(SamrenaVector* vec) {
    // For now, assume we can extend the existing structure
    // In practice, this might require API changes or wrapper approach
    return (SamrenaVectorWithGrowth*)vec;
}
```

### 2. Growth Strategy Calculations
```c
// Exponential growth implementation
static size_t calculate_exponential_growth(const SamrenaGrowthPolicy* policy,
                                          size_t current_capacity,
                                          size_t required_capacity) {
    const auto* params = &policy->params.exponential;
    
    // Calculate factor-based growth
    size_t new_capacity = (size_t)(current_capacity * params->factor);
    
    // Handle edge cases for small capacities
    if (current_capacity == 0) {
        new_capacity = (required_capacity > params->min_increment) ? 
                      required_capacity : params->min_increment;
    }
    
    // Ensure minimum increment
    if (new_capacity < current_capacity + params->min_increment) {
        new_capacity = current_capacity + params->min_increment;
    }
    
    // Ensure we meet requirements
    if (new_capacity < required_capacity) {
        new_capacity = required_capacity;
    }
    
    // Apply maximum capacity limit
    if (new_capacity > params->max_capacity) {
        new_capacity = params->max_capacity;
    }
    
    return new_capacity;
}

// Linear growth implementation
static size_t calculate_linear_growth(const SamrenaGrowthPolicy* policy,
                                     size_t current_capacity,
                                     size_t required_capacity) {
    const auto* params = &policy->params.linear;
    
    if (current_capacity == 0) {
        return (required_capacity > params->increment) ? 
               required_capacity : params->increment;
    }
    
    size_t new_capacity = current_capacity;
    
    // Keep adding increments until we meet requirements
    while (new_capacity < required_capacity) {
        new_capacity += params->increment;
    }
    
    // Apply maximum capacity limit
    if (new_capacity > params->max_capacity) {
        new_capacity = params->max_capacity;
    }
    
    return new_capacity;
}

// Fibonacci growth implementation
static size_t calculate_fibonacci_growth(SamrenaGrowthPolicy* policy,
                                        size_t current_capacity,
                                        size_t required_capacity) {
    auto* params = &policy->params.fibonacci;
    
    // Initialize Fibonacci sequence if needed
    if (params->fib_a == 0 && params->fib_b == 0) {
        if (current_capacity == 0) {
            params->fib_a = 1;
            params->fib_b = 1;
        } else {
            // Find current position in Fibonacci sequence
            params->fib_a = 1;
            params->fib_b = 1;
            while (params->fib_b < current_capacity) {
                size_t next_fib = params->fib_a + params->fib_b;
                params->fib_a = params->fib_b;
                params->fib_b = next_fib;
                
                // Prevent overflow
                if (next_fib < params->fib_a) {
                    params->fib_b = params->max_capacity;
                    break;
                }
            }
        }
    }
    
    // Find next Fibonacci number >= required capacity
    while (params->fib_b < required_capacity) {
        size_t next_fib = params->fib_a + params->fib_b;
        
        // Check for overflow
        if (next_fib < params->fib_a) {
            return params->max_capacity;
        }
        
        params->fib_a = params->fib_b;
        params->fib_b = next_fib;
    }
    
    return (params->fib_b > params->max_capacity) ? 
           params->max_capacity : params->fib_b;
}

// Conservative growth implementation
static size_t calculate_conservative_growth(const SamrenaGrowthPolicy* policy,
                                           size_t current_capacity,
                                           size_t required_capacity,
                                           size_t current_size) {
    const auto* params = &policy->params.conservative;
    
    if (current_capacity == 0) {
        return required_capacity;
    }
    
    // Calculate current waste ratio
    float current_waste = (current_capacity > current_size) ? 
        (float)(current_capacity - current_size) / current_capacity : 0.0f;
    
    size_t new_capacity;
    
    if (current_waste > params->waste_threshold) {
        // Already wasting too much - grow minimally
        new_capacity = required_capacity;
    } else {
        // Calculate maximum capacity that keeps waste within threshold
        // waste_threshold = (capacity - size) / capacity
        // capacity * (1 - waste_threshold) = size
        // capacity = size / (1 - waste_threshold)
        size_t max_acceptable = (size_t)(current_size / (1.0f - params->waste_threshold));
        
        // Use the larger of required capacity and acceptable capacity
        new_capacity = (required_capacity > max_acceptable) ? 
                      required_capacity : max_acceptable;
    }
    
    // Ensure minimum increment to avoid thrashing
    if (new_capacity < current_capacity + params->min_increment) {
        new_capacity = current_capacity + params->min_increment;
    }
    
    return new_capacity;
}

// Aggressive growth implementation
static size_t calculate_aggressive_growth(SamrenaGrowthPolicy* policy,
                                         size_t current_capacity,
                                         size_t required_capacity) {
    auto* params = &policy->params.aggressive;
    
    if (current_capacity == 0) {
        return required_capacity * 2;  // Start aggressively
    }
    
    // Determine current growth factor based on capacity
    float current_factor = params->initial_factor;
    
    if (current_capacity > params->performance_threshold) {
        // Apply decay to become more conservative at large sizes
        float decay = powf(params->decay_rate, 
                          (float)current_capacity / params->performance_threshold);
        current_factor = params->initial_factor * (1.0f - decay);
        
        // Minimum factor to prevent stagnation
        if (current_factor < 1.1f) current_factor = 1.1f;
    }
    
    size_t new_capacity = (size_t)(current_capacity * current_factor);
    
    // Ensure we meet requirements
    if (new_capacity < required_capacity) {
        new_capacity = required_capacity;
    }
    
    return new_capacity;
}
```

### 3. Adaptive Growth Implementation
```c
// Adaptive growth that learns from usage patterns
static size_t calculate_adaptive_growth(SamrenaGrowthPolicy* policy,
                                       size_t current_capacity,
                                       size_t required_capacity,
                                       const SamrenaGrowthStats* stats) {
    auto* params = &policy->params.adaptive;
    
    // Analyze recent patterns
    bool frequent_resizing = (stats->growth_frequency > params->resize_frequency_threshold);
    bool high_utilization = (stats->average_utilization > 0.8);
    bool low_utilization = (stats->average_utilization < 0.5);
    bool large_vector = (current_capacity > 1000);
    
    // Choose adaptive strategy
    SamrenaGrowthStrategy chosen_strategy = params->fallback_strategy;
    
    if (frequent_resizing && high_utilization) {
        // Growing frequently with high utilization -> be more aggressive
        chosen_strategy = SAMRENA_GROWTH_AGGRESSIVE;
    } else if (low_utilization && !frequent_resizing) {
        // Low utilization and stable -> be conservative
        chosen_strategy = SAMRENA_GROWTH_CONSERVATIVE;
    } else if (large_vector && high_utilization) {
        // Large vector with high utilization -> use linear growth to control memory
        chosen_strategy = SAMRENA_GROWTH_LINEAR;
    }
    
    // Create temporary policy with chosen strategy
    SamrenaGrowthPolicy temp_policy = *policy;
    temp_policy.strategy = chosen_strategy;
    
    // Set strategy-specific parameters based on analysis
    switch (chosen_strategy) {
        case SAMRENA_GROWTH_AGGRESSIVE:
            temp_policy.params.aggressive = (typeof(temp_policy.params.aggressive)){
                .initial_factor = frequent_resizing ? 2.5f : 2.0f,
                .decay_rate = high_utilization ? 0.05f : 0.1f,
                .performance_threshold = current_capacity
            };
            return calculate_aggressive_growth(&temp_policy, current_capacity, required_capacity);
            
        case SAMRENA_GROWTH_CONSERVATIVE:
            temp_policy.params.conservative = (typeof(temp_policy.params.conservative)){
                .waste_threshold = low_utilization ? 0.15f : 0.25f,
                .min_increment = 1
            };
            return calculate_conservative_growth(&temp_policy, current_capacity, 
                                               required_capacity, 
                                               (size_t)(current_capacity * stats->average_utilization));
            
        case SAMRENA_GROWTH_LINEAR:
            temp_policy.params.linear = (typeof(temp_policy.params.linear)){
                .increment = large_vector ? current_capacity / 10 : 16,
                .max_capacity = SIZE_MAX / 2
            };
            return calculate_linear_growth(&temp_policy, current_capacity, required_capacity);
            
        default:
            // Default to exponential with adaptive factor
            temp_policy.params.exponential = (typeof(temp_policy.params.exponential)){
                .factor = high_utilization ? 1.6f : 1.4f,
                .min_increment = frequent_resizing ? 8 : 4,
                .max_capacity = SIZE_MAX / 2
            };
            return calculate_exponential_growth(&temp_policy, current_capacity, required_capacity);
    }
}
```

### 4. Main Growth Calculation Engine
```c
// Main function to calculate new capacity
static size_t calculate_new_capacity_internal(SamrenaVectorWithGrowth* vec,
                                             size_t required_capacity) {
    if (required_capacity <= vec->capacity) {
        return vec->capacity;  // No growth needed
    }
    
    // Get current growth policy (use default if none set)
    SamrenaGrowthPolicy* policy = vec->growth_policy;
    if (!policy) {
        policy = (SamrenaGrowthPolicy*)&SAMRENA_GROWTH_POLICY_DEFAULT;
    }
    
    // Check against absolute limits
    if (required_capacity > policy->absolute_max_capacity) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_INSUFFICIENT_CAPACITY, 
                         "Required capacity exceeds maximum", vec);
        return policy->absolute_max_capacity;
    }
    
    size_t new_capacity;
    
    // Calculate based on strategy
    switch (policy->strategy) {
        case SAMRENA_GROWTH_EXPONENTIAL:
            new_capacity = calculate_exponential_growth(policy, vec->capacity, required_capacity);
            break;
            
        case SAMRENA_GROWTH_LINEAR:
            new_capacity = calculate_linear_growth(policy, vec->capacity, required_capacity);
            break;
            
        case SAMRENA_GROWTH_FIBONACCI:
            new_capacity = calculate_fibonacci_growth(policy, vec->capacity, required_capacity);
            break;
            
        case SAMRENA_GROWTH_CONSERVATIVE:
            new_capacity = calculate_conservative_growth(policy, vec->capacity, 
                                                       required_capacity, vec->size);
            break;
            
        case SAMRENA_GROWTH_AGGRESSIVE:
            new_capacity = calculate_aggressive_growth(policy, vec->capacity, required_capacity);
            break;
            
        case SAMRENA_GROWTH_ADAPTIVE:
            new_capacity = calculate_adaptive_growth(policy, vec->capacity, 
                                                   required_capacity, &vec->growth_stats);
            break;
            
        case SAMRENA_GROWTH_CUSTOM:
            if (policy->params.custom.growth_function) {
                new_capacity = policy->params.custom.growth_function(
                    vec->capacity, required_capacity, policy->params.custom.user_data);
            } else {
                // Fallback to default exponential
                new_capacity = calculate_exponential_growth(&SAMRENA_GROWTH_POLICY_DEFAULT,
                                                          vec->capacity, required_capacity);
            }
            break;
            
        default:
            new_capacity = calculate_exponential_growth(&SAMRENA_GROWTH_POLICY_DEFAULT,
                                                      vec->capacity, required_capacity);
            break;
    }
    
    // Apply absolute limits
    if (new_capacity > policy->absolute_max_capacity) {
        new_capacity = policy->absolute_max_capacity;
    }
    
    // Check memory limits if specified
    if (policy->memory_limit_bytes > 0) {
        size_t projected_bytes = new_capacity * vec->element_size;
        if (projected_bytes > policy->memory_limit_bytes) {
            new_capacity = policy->memory_limit_bytes / vec->element_size;
            if (new_capacity < required_capacity) {
                SAMRENA_SET_ERROR(SAMRENA_ERROR_ALLOCATION_FAILED, 
                                 "Memory limit prevents required growth", vec);
                return vec->capacity;  // Can't grow
            }
        }
    }
    
    return new_capacity;
}

// Update growth statistics after a capacity change
static void update_growth_stats(SamrenaVectorWithGrowth* vec, 
                               size_t old_capacity, size_t new_capacity,
                               bool is_growth) {
    if (!vec->track_statistics) return;
    
    SamrenaGrowthStats* stats = &vec->growth_stats;
    
    if (is_growth) {
        stats->total_growths++;
    } else {
        stats->total_shrinks++;
    }
    
    // Update memory statistics
    stats->bytes_allocated = new_capacity * vec->element_size;
    stats->bytes_wasted = (new_capacity - vec->size) * vec->element_size;
    
    // Update utilization (exponential moving average)
    double current_utilization = (new_capacity > 0) ? 
        (double)vec->size / new_capacity : 1.0;
    
    if (stats->total_growths + stats->total_shrinks == 1) {
        // First operation
        stats->average_utilization = current_utilization;
    } else {
        // Exponential moving average with alpha = 0.1
        stats->average_utilization = stats->average_utilization * 0.9 + 
                                    current_utilization * 0.1;
    }
    
    // Update operation tracking
    stats->recent_operations++;
    stats->last_growth_time = get_current_timestamp();
    
    // Calculate growth frequency
    size_t total_operations = stats->total_growths + stats->total_shrinks;
    if (total_operations > 0) {
        stats->growth_frequency = (float)stats->total_growths / total_operations;
    }
    
    // Update last access time
    vec->last_access_time = stats->last_growth_time;
}
```

### 5. Shrinking Policy Implementation
```c
// Default shrinking policies
static const SamrenaShrinkPolicy SAMRENA_SHRINK_POLICY_DEFAULT = {
    .enabled = true,
    .utilization_threshold = 0.25f,  // Shrink when using < 25% of capacity
    .min_capacity = 4,               // Never shrink below 4 elements
    .shrink_factor = 0.5f,          // Shrink to half capacity
    .hysteresis_buffer = 2           // Add 2 elements buffer
};

static const SamrenaShrinkPolicy SAMRENA_SHRINK_POLICY_CONSERVATIVE = {
    .enabled = true,
    .utilization_threshold = 0.1f,   // Only shrink when using < 10%
    .min_capacity = 8,
    .shrink_factor = 0.75f,         // Shrink less aggressively
    .hysteresis_buffer = 4
};

// Calculate capacity after shrinking
static size_t calculate_shrink_capacity(const SamrenaShrinkPolicy* policy,
                                       size_t current_capacity,
                                       size_t current_size) {
    if (!policy->enabled || current_capacity <= policy->min_capacity) {
        return current_capacity;
    }
    
    // Check utilization threshold
    float utilization = (current_capacity > 0) ? 
        (float)current_size / current_capacity : 1.0f;
    
    if (utilization >= policy->utilization_threshold) {
        return current_capacity;  // Don't shrink
    }
    
    // Calculate new capacity
    size_t new_capacity = (size_t)(current_capacity * policy->shrink_factor);
    
    // Add hysteresis buffer to prevent thrashing
    new_capacity += policy->hysteresis_buffer;
    
    // Apply constraints
    if (new_capacity < policy->min_capacity) {
        new_capacity = policy->min_capacity;
    }
    
    if (new_capacity < current_size) {
        new_capacity = current_size;  // Must hold current elements
    }
    
    // Only shrink if it makes a meaningful difference
    if (new_capacity >= current_capacity * 0.9) {
        return current_capacity;  // Not worth shrinking
    }
    
    return new_capacity;
}

// Evaluate whether vector should be shrunk
static bool should_shrink_vector(SamrenaVectorWithGrowth* vec) {
    if (!vec->auto_shrink || !vec->can_shrink) {
        return false;
    }
    
    size_t new_capacity = calculate_shrink_capacity(&vec->shrink_policy, 
                                                   vec->capacity, vec->size);
    
    return new_capacity < vec->capacity;
}
```

### 6. Public API Implementation
```c
// Set growth policy for a vector
int samrena_vector_set_growth_policy(SamrenaVector* vec, 
                                    const SamrenaGrowthPolicy* policy) {
    if (!vec || !policy) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Invalid parameters", vec);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    SamrenaVectorWithGrowth* ext_vec = get_extended_vector(vec);
    
    // Allocate policy copy if needed
    if (!ext_vec->growth_policy) {
        ext_vec->growth_policy = samrena_alloc(ext_vec->arena, sizeof(SamrenaGrowthPolicy));
        if (!ext_vec->growth_policy) {
            SAMRENA_SET_ERROR(SAMRENA_ERROR_ALLOCATION_FAILED, 
                             "Failed to allocate growth policy", vec);
            return SAMRENA_ERROR_ALLOCATION_FAILED;
        }
    }
    
    // Copy policy
    *ext_vec->growth_policy = *policy;
    
    return SAMRENA_SUCCESS;
}

// Get current growth policy
const SamrenaGrowthPolicy* samrena_vector_get_growth_policy(const SamrenaVector* vec) {
    if (!vec) return NULL;
    
    const SamrenaVectorWithGrowth* ext_vec = (const SamrenaVectorWithGrowth*)vec;
    return ext_vec->growth_policy ? ext_vec->growth_policy : &SAMRENA_GROWTH_POLICY_DEFAULT;
}

// Set shrinking policy
int samrena_vector_set_shrink_policy(SamrenaVector* vec, 
                                    const SamrenaShrinkPolicy* policy) {
    if (!vec || !policy) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Invalid parameters", vec);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    SamrenaVectorWithGrowth* ext_vec = get_extended_vector(vec);
    ext_vec->shrink_policy = *policy;
    
    return SAMRENA_SUCCESS;
}

// Get growth statistics
SamrenaGrowthStats samrena_vector_get_growth_stats(const SamrenaVector* vec) {
    SamrenaGrowthStats empty_stats = {0};
    
    if (!vec) return empty_stats;
    
    const SamrenaVectorWithGrowth* ext_vec = (const SamrenaVectorWithGrowth*)vec;
    return ext_vec->growth_stats;
}

// Reset growth statistics
void samrena_vector_reset_growth_stats(SamrenaVector* vec) {
    if (!vec) return;
    
    SamrenaVectorWithGrowth* ext_vec = get_extended_vector(vec);
    init_growth_stats(&ext_vec->growth_stats);
}

// Enable/disable statistics tracking
void samrena_vector_set_statistics_tracking(SamrenaVector* vec, bool enabled) {
    if (!vec) return;
    
    SamrenaVectorWithGrowth* ext_vec = get_extended_vector(vec);
    ext_vec->track_statistics = enabled;
    
    if (enabled && ext_vec->growth_stats.total_growths == 0) {
        init_growth_stats(&ext_vec->growth_stats);
    }
}

// Force evaluation of shrinking conditions
int samrena_vector_evaluate_shrink(SamrenaVector* vec) {
    if (!vec) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Vector is NULL", vec);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    SamrenaVectorWithGrowth* ext_vec = get_extended_vector(vec);
    
    if (!should_shrink_vector(ext_vec)) {
        return SAMRENA_SUCCESS;  // No shrinking needed
    }
    
    size_t new_capacity = calculate_shrink_capacity(&ext_vec->shrink_policy, 
                                                   ext_vec->capacity, ext_vec->size);
    
    // Perform the shrink
    int result = samrena_vector_reserve_auto(vec, new_capacity);
    
    if (result == SAMRENA_SUCCESS) {
        update_growth_stats(ext_vec, ext_vec->capacity, new_capacity, false);
        ext_vec->capacity = new_capacity;
    }
    
    return result;
}
```

### 7. Integration with Existing Reserve Functions
```c
// Enhanced reserve function that uses growth policies
int samrena_vector_reserve_with_growth_policy(SamrenaVector* vec, size_t min_capacity) {
    if (!vec) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Vector is NULL", vec);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    if (min_capacity <= vec->capacity) {
        return SAMRENA_SUCCESS;
    }
    
    SamrenaVectorWithGrowth* ext_vec = get_extended_vector(vec);
    
    // Calculate new capacity using growth policy
    size_t old_capacity = ext_vec->capacity;
    size_t new_capacity = calculate_new_capacity_internal(ext_vec, min_capacity);
    
    if (new_capacity <= old_capacity) {
        return SAMRENA_SUCCESS;  // No growth possible or needed
    }
    
    // Attempt reallocation
    size_t new_size = new_capacity * ext_vec->element_size;
    void* new_data;
    
    if (ext_vec->owns_arena) {
        new_data = samrena_realloc(ext_vec->arena, ext_vec->data, new_size);
    } else {
        new_data = samrena_alloc(ext_vec->arena, new_size);
        if (new_data && ext_vec->data) {
            memcpy(new_data, ext_vec->data, ext_vec->size * ext_vec->element_size);
        }
    }
    
    if (!new_data) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_ALLOCATION_FAILED, 
                         "Failed to allocate memory for growth", vec);
        return SAMRENA_ERROR_ALLOCATION_FAILED;
    }
    
    // Update vector
    ext_vec->data = new_data;
    ext_vec->capacity = new_capacity;
    
    // Update statistics
    update_growth_stats(ext_vec, old_capacity, new_capacity, true);
    
    return SAMRENA_SUCCESS;
}

// Override default reserve function to use growth policies
#ifdef SAMRENA_USE_GROWTH_POLICIES
#define samrena_vector_reserve_auto samrena_vector_reserve_with_growth_policy
#endif
```

### 8. Policy Suggestion System
```c
// Analyze vector usage and suggest optimal growth policy
SamrenaGrowthPolicy samrena_vector_suggest_growth_policy(const SamrenaVector* vec) {
    SamrenaGrowthPolicy suggested = SAMRENA_GROWTH_POLICY_DEFAULT;
    
    if (!vec) return suggested;
    
    const SamrenaVectorWithGrowth* ext_vec = (const SamrenaVectorWithGrowth*)vec;
    const SamrenaGrowthStats* stats = &ext_vec->growth_stats;
    
    // Analyze usage patterns
    bool high_growth_frequency = (stats->growth_frequency > 0.2f);
    bool low_utilization = (stats->average_utilization < 0.5);
    bool large_vector = (ext_vec->capacity > 1000);
    bool memory_sensitive = (ext_vec->growth_policy && 
                            ext_vec->growth_policy->memory_limit_bytes > 0);
    
    if (memory_sensitive || low_utilization) {
        // Memory-constrained or wasteful usage -> conservative
        suggested = SAMRENA_GROWTH_POLICY_CONSERVATIVE;
    } else if (high_growth_frequency && !large_vector) {
        // Frequent growth on small vectors -> aggressive
        suggested = SAMRENA_GROWTH_POLICY_FAST;
    } else if (large_vector) {
        // Large vectors -> linear growth to control memory
        suggested = SAMRENA_GROWTH_POLICY_LINEAR;
    } else if (stats->total_growths > 10) {
        // Established usage pattern -> adaptive
        suggested = SAMRENA_GROWTH_POLICY_ADAPTIVE;
    }
    
    return suggested;
}

// Auto-optimize growth policy based on current statistics
int samrena_vector_optimize_growth_policy(SamrenaVector* vec) {
    if (!vec) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Vector is NULL", vec);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    SamrenaGrowthPolicy suggested = samrena_vector_suggest_growth_policy(vec);
    return samrena_vector_set_growth_policy(vec, &suggested);
}
```

## Testing Implementation

### Growth Policy Tests
```c
// tests/test_growth_policies.c

void test_exponential_growth() {
    SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 4);
    samrena_vector_set_growth_policy(vec, &SAMRENA_GROWTH_POLICY_DEFAULT);
    samrena_vector_set_statistics_tracking(vec, true);
    
    // Fill initial capacity
    for (int i = 0; i < 4; i++) {
        samrena_vector_push_auto(vec, &i);
    }
    
    size_t initial_capacity = samrena_vector_capacity(vec);
    
    // Trigger growth
    int value = 4;
    samrena_vector_push_auto(vec, &value);
    
    size_t new_capacity = samrena_vector_capacity(vec);
    
    // Should have grown by at least 1.5x
    assert(new_capacity >= (size_t)(initial_capacity * 1.5f));
    
    // Check statistics
    SamrenaGrowthStats stats = samrena_vector_get_growth_stats(vec);
    assert(stats.total_growths == 1);
    
    samrena_vector_destroy(vec);
}

void test_conservative_growth() {
    SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 4);
    samrena_vector_set_growth_policy(vec, &SAMRENA_GROWTH_POLICY_CONSERVATIVE);
    
    // Fill to trigger multiple growths
    for (int i = 0; i < 20; i++) {
        samrena_vector_push_auto(vec, &i);
    }
    
    // Conservative growth should result in less waste
    SamrenaGrowthStats stats = samrena_vector_get_growth_stats(vec);
    size_t capacity = samrena_vector_capacity(vec);
    size_t size = samrena_vector_size(vec);
    
    float waste_ratio = (float)(capacity - size) / capacity;
    assert(waste_ratio <= 0.3f);  // Should have reasonable waste
    
    samrena_vector_destroy(vec);
}

void test_adaptive_growth() {
    SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 4);
    samrena_vector_set_growth_policy(vec, &SAMRENA_GROWTH_POLICY_ADAPTIVE);
    samrena_vector_set_statistics_tracking(vec, true);
    
    // Phase 1: Rapid growth
    for (int i = 0; i < 100; i++) {
        samrena_vector_push_auto(vec, &i);
    }
    
    SamrenaGrowthStats stats1 = samrena_vector_get_growth_stats(vec);
    
    // Phase 2: Stable usage
    for (int i = 0; i < 100; i++) {
        int temp;
        samrena_vector_get(vec, i % samrena_vector_size(vec), &temp);
    }
    
    // Phase 3: More growth
    for (int i = 100; i < 200; i++) {
        samrena_vector_push_auto(vec, &i);
    }
    
    SamrenaGrowthStats stats2 = samrena_vector_get_growth_stats(vec);
    
    // Should have adapted growth strategy
    assert(stats2.total_growths > stats1.total_growths);
    
    samrena_vector_destroy(vec);
}

void test_shrinking_policy() {
    SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 100);
    
    SamrenaShrinkPolicy shrink_policy = {
        .enabled = true,
        .utilization_threshold = 0.25f,
        .min_capacity = 4,
        .shrink_factor = 0.5f,
        .hysteresis_buffer = 2
    };
    
    samrena_vector_set_shrink_policy(vec, &shrink_policy);
    
    // Fill vector
    for (int i = 0; i < 80; i++) {
        samrena_vector_push_auto(vec, &i);
    }
    
    size_t full_capacity = samrena_vector_capacity(vec);
    
    // Remove most elements
    for (int i = 0; i < 70; i++) {
        samrena_vector_remove(vec, 0);
    }
    
    // Force shrink evaluation
    samrena_vector_evaluate_shrink(vec);
    
    size_t shrunk_capacity = samrena_vector_capacity(vec);
    
    // Should have shrunk
    assert(shrunk_capacity < full_capacity);
    
    samrena_vector_destroy(vec);
}
```

### Performance Benchmarks
```c
void benchmark_growth_strategies() {
    const size_t test_sizes[] = {1000, 10000, 100000};
    const SamrenaGrowthPolicy* policies[] = {
        &SAMRENA_GROWTH_POLICY_DEFAULT,
        &SAMRENA_GROWTH_POLICY_FAST,
        &SAMRENA_GROWTH_POLICY_CONSERVATIVE,
        &SAMRENA_GROWTH_POLICY_LINEAR
    };
    const char* policy_names[] = {
        "Default", "Fast", "Conservative", "Linear"
    };
    
    for (size_t i = 0; i < 3; i++) {
        printf("Testing with %zu elements:\n", test_sizes[i]);
        
        for (size_t j = 0; j < 4; j++) {
            SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 4);
            samrena_vector_set_growth_policy(vec, policies[j]);
            samrena_vector_set_statistics_tracking(vec, true);
            
            clock_t start = clock();
            
            for (size_t k = 0; k < test_sizes[i]; k++) {
                int value = k;
                samrena_vector_push_auto(vec, &value);
            }
            
            clock_t end = clock();
            
            SamrenaGrowthStats stats = samrena_vector_get_growth_stats(vec);
            
            printf("  %s: %ld ms, %zu growths, %.2f%% utilization\n",
                   policy_names[j],
                   (end - start) * 1000 / CLOCKS_PER_SEC,
                   stats.total_growths,
                   stats.average_utilization * 100);
            
            samrena_vector_destroy(vec);
        }
        printf("\n");
    }
}
```

## Integration Notes
- Works with existing vector API
- Backward compatible with fixed growth factor
- Optional feature - can be disabled
- Minimal performance overhead when disabled
- Statistics tracking is optional
- Integrates with arena memory management
- Supports both owned and shared arena vectors