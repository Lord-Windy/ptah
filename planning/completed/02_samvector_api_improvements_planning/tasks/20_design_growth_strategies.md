# Task 20: Design Growth Strategies

## Objective
Design a flexible and configurable growth strategy system for Samvector that optimizes memory allocation patterns based on usage scenarios, providing multiple growth policies, adaptive strategies, and memory-conscious options.

## Requirements
- Multiple growth strategies for different use cases
- Configurable growth parameters per vector instance
- Adaptive growth based on usage patterns
- Memory-conscious growth for resource-constrained environments
- Integration with existing capacity management
- Performance optimization for common patterns

## Proposed Growth Strategy System

### Core Growth Strategy Framework
```c
// Growth strategy enumeration
typedef enum {
    SAMRENA_GROWTH_EXPONENTIAL,      // Exponential growth (1.5x, 2x, etc.)
    SAMRENA_GROWTH_LINEAR,           // Linear growth (fixed increment)
    SAMRENA_GROWTH_FIBONACCI,        // Fibonacci sequence growth
    SAMRENA_GROWTH_CONSERVATIVE,     // Minimal growth to reduce waste
    SAMRENA_GROWTH_AGGRESSIVE,       // Fast growth for performance
    SAMRENA_GROWTH_ADAPTIVE,         // Adaptive based on usage patterns
    SAMRENA_GROWTH_CUSTOM            // User-defined growth function
} SamrenaGrowthStrategy;

// Growth policy configuration
typedef struct {
    SamrenaGrowthStrategy strategy;
    
    // Strategy-specific parameters
    union {
        // Exponential growth parameters
        struct {
            float factor;            // Growth factor (e.g., 1.5, 2.0)
            size_t min_increment;    // Minimum elements to add
            size_t max_capacity;     // Maximum capacity limit
        } exponential;
        
        // Linear growth parameters
        struct {
            size_t increment;        // Fixed number of elements to add
            size_t max_capacity;     // Maximum capacity limit
        } linear;
        
        // Fibonacci growth parameters
        struct {
            size_t max_capacity;     // Maximum capacity limit
            size_t fib_a, fib_b;     // Current Fibonacci numbers
        } fibonacci;
        
        // Conservative growth parameters
        struct {
            float waste_threshold;   // Maximum acceptable waste ratio
            size_t min_increment;    // Minimum growth per allocation
        } conservative;
        
        // Aggressive growth parameters
        struct {
            float initial_factor;    // Initial growth factor
            float decay_rate;        // Factor decay per growth
            size_t performance_threshold; // Size at which to become conservative
        } aggressive;
        
        // Adaptive growth parameters
        struct {
            size_t sample_window;    // Number of recent operations to consider
            float resize_frequency_threshold; // Threshold for frequent resizing
            SamrenaGrowthStrategy fallback_strategy; // Strategy when adaptation fails
        } adaptive;
        
        // Custom growth parameters
        struct {
            size_t (*growth_function)(size_t current_capacity, size_t required_capacity, void* user_data);
            void* user_data;         // User data for custom function
        } custom;
    } params;
    
    // Global constraints
    size_t absolute_max_capacity;    // Hard limit on capacity
    size_t memory_limit_bytes;       // Memory usage limit
    bool allow_shrinking;            // Whether shrinking is allowed
} SamrenaGrowthPolicy;

// Growth statistics for monitoring and adaptation
typedef struct {
    size_t total_growths;            // Total number of growth operations
    size_t total_shrinks;            // Total number of shrink operations
    size_t bytes_allocated;          // Total bytes allocated
    size_t bytes_wasted;             // Total bytes wasted (capacity - size)
    double average_utilization;      // Average capacity utilization
    size_t recent_operations;        // Recent operation count
    uint64_t last_growth_time;       // Timestamp of last growth
    float growth_frequency;          // Growths per operation
} SamrenaGrowthStats;
```

### Default Growth Policies
```c
// Predefined growth policies for common use cases

// Default exponential growth (1.5x factor)
static const SamrenaGrowthPolicy SAMRENA_GROWTH_POLICY_DEFAULT = {
    .strategy = SAMRENA_GROWTH_EXPONENTIAL,
    .params.exponential = {
        .factor = 1.5f,
        .min_increment = 4,
        .max_capacity = SIZE_MAX / 2
    },
    .absolute_max_capacity = SIZE_MAX / 2,
    .memory_limit_bytes = 0,  // No limit
    .allow_shrinking = true
};

// Fast growth for performance-critical scenarios
static const SamrenaGrowthPolicy SAMRENA_GROWTH_POLICY_FAST = {
    .strategy = SAMRENA_GROWTH_EXPONENTIAL,
    .params.exponential = {
        .factor = 2.0f,
        .min_increment = 8,
        .max_capacity = SIZE_MAX / 2
    },
    .absolute_max_capacity = SIZE_MAX / 2,
    .memory_limit_bytes = 0,
    .allow_shrinking = false
};

// Conservative growth for memory-constrained environments
static const SamrenaGrowthPolicy SAMRENA_GROWTH_POLICY_CONSERVATIVE = {
    .strategy = SAMRENA_GROWTH_CONSERVATIVE,
    .params.conservative = {
        .waste_threshold = 0.25f,  // Max 25% waste
        .min_increment = 1
    },
    .absolute_max_capacity = SIZE_MAX / 4,
    .memory_limit_bytes = 0,
    .allow_shrinking = true
};

// Linear growth for predictable memory usage
static const SamrenaGrowthPolicy SAMRENA_GROWTH_POLICY_LINEAR = {
    .strategy = SAMRENA_GROWTH_LINEAR,
    .params.linear = {
        .increment = 16,
        .max_capacity = SIZE_MAX / 2
    },
    .absolute_max_capacity = SIZE_MAX / 2,
    .memory_limit_bytes = 0,
    .allow_shrinking = true
};

// Adaptive growth that learns from usage patterns
static const SamrenaGrowthPolicy SAMRENA_GROWTH_POLICY_ADAPTIVE = {
    .strategy = SAMRENA_GROWTH_ADAPTIVE,
    .params.adaptive = {
        .sample_window = 100,
        .resize_frequency_threshold = 0.1f,  // 10% of operations cause resize
        .fallback_strategy = SAMRENA_GROWTH_EXPONENTIAL
    },
    .absolute_max_capacity = SIZE_MAX / 2,
    .memory_limit_bytes = 0,
    .allow_shrinking = true
};
```

### Growth Strategy Implementation
```c
// Growth strategy calculation functions

// Calculate new capacity using exponential growth
static size_t calculate_exponential_growth(const SamrenaGrowthPolicy* policy,
                                          size_t current_capacity,
                                          size_t required_capacity) {
    const auto* params = &policy->params.exponential;
    
    // Start with factor-based growth
    size_t new_capacity = (size_t)(current_capacity * params->factor);
    
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

// Calculate new capacity using linear growth
static size_t calculate_linear_growth(const SamrenaGrowthPolicy* policy,
                                     size_t current_capacity,
                                     size_t required_capacity) {
    const auto* params = &policy->params.linear;
    
    size_t new_capacity = current_capacity + params->increment;
    
    // Ensure we meet requirements
    while (new_capacity < required_capacity) {
        new_capacity += params->increment;
    }
    
    // Apply maximum capacity limit
    if (new_capacity > params->max_capacity) {
        new_capacity = params->max_capacity;
    }
    
    return new_capacity;
}

// Calculate new capacity using Fibonacci growth
static size_t calculate_fibonacci_growth(SamrenaGrowthPolicy* policy,
                                        size_t current_capacity,
                                        size_t required_capacity) {
    auto* params = &policy->params.fibonacci;
    
    // Initialize Fibonacci sequence if needed
    if (params->fib_a == 0 && params->fib_b == 0) {
        params->fib_a = 1;
        params->fib_b = 1;
    }
    
    // Find next Fibonacci number >= required capacity
    while (params->fib_b < required_capacity) {
        size_t next_fib = params->fib_a + params->fib_b;
        params->fib_a = params->fib_b;
        params->fib_b = next_fib;
        
        // Prevent overflow
        if (next_fib < params->fib_a) {
            params->fib_b = params->max_capacity;
            break;
        }
    }
    
    size_t new_capacity = params->fib_b;
    
    // Apply maximum capacity limit
    if (new_capacity > params->max_capacity) {
        new_capacity = params->max_capacity;
    }
    
    return new_capacity;
}

// Calculate new capacity using conservative growth
static size_t calculate_conservative_growth(const SamrenaGrowthPolicy* policy,
                                           size_t current_capacity,
                                           size_t required_capacity,
                                           size_t current_size) {
    const auto* params = &policy->params.conservative;
    
    // Calculate current waste
    float current_waste = (current_capacity > current_size) ? 
        (float)(current_capacity - current_size) / current_capacity : 0.0f;
    
    // If waste is already high, minimize growth
    size_t new_capacity;
    if (current_waste > params->waste_threshold) {
        new_capacity = required_capacity;
    } else {
        // Allow some growth but stay within waste threshold
        size_t max_acceptable_capacity = (size_t)(current_size / (1.0f - params->waste_threshold));
        new_capacity = (required_capacity < max_acceptable_capacity) ? 
                      max_acceptable_capacity : required_capacity;
    }
    
    // Ensure minimum increment
    if (new_capacity < current_capacity + params->min_increment) {
        new_capacity = current_capacity + params->min_increment;
    }
    
    return new_capacity;
}

// Calculate new capacity using aggressive growth
static size_t calculate_aggressive_growth(SamrenaGrowthPolicy* policy,
                                         size_t current_capacity,
                                         size_t required_capacity) {
    auto* params = &policy->params.aggressive;
    
    // Use high growth factor initially, then decay
    float current_factor = params->initial_factor;
    
    // Apply decay based on current capacity
    if (current_capacity > params->performance_threshold) {
        current_factor *= (1.0f - params->decay_rate);
        if (current_factor < 1.1f) current_factor = 1.1f;  // Minimum factor
    }
    
    size_t new_capacity = (size_t)(current_capacity * current_factor);
    
    // Ensure we meet requirements
    if (new_capacity < required_capacity) {
        new_capacity = required_capacity;
    }
    
    return new_capacity;
}
```

### Adaptive Growth Strategy
```c
// Adaptive growth that learns from usage patterns
static size_t calculate_adaptive_growth(SamrenaGrowthPolicy* policy,
                                       size_t current_capacity,
                                       size_t required_capacity,
                                       const SamrenaGrowthStats* stats) {
    auto* params = &policy->params.adaptive;
    
    // Analyze recent growth patterns
    bool frequent_resizing = (stats->growth_frequency > params->resize_frequency_threshold);
    bool high_utilization = (stats->average_utilization > 0.8);
    bool low_utilization = (stats->average_utilization < 0.5);
    
    // Choose strategy based on patterns
    SamrenaGrowthStrategy adaptive_strategy;
    
    if (frequent_resizing && high_utilization) {
        // Frequent resizing with high utilization -> use aggressive growth
        adaptive_strategy = SAMRENA_GROWTH_AGGRESSIVE;
    } else if (low_utilization) {
        // Low utilization -> use conservative growth
        adaptive_strategy = SAMRENA_GROWTH_CONSERVATIVE;
    } else {
        // Default to fallback strategy
        adaptive_strategy = params->fallback_strategy;
    }
    
    // Temporarily modify policy to use chosen strategy
    SamrenaGrowthPolicy temp_policy = *policy;
    temp_policy.strategy = adaptive_strategy;
    
    // Initialize strategy-specific parameters based on analysis
    switch (adaptive_strategy) {
        case SAMRENA_GROWTH_AGGRESSIVE:
            temp_policy.params.aggressive = (typeof(temp_policy.params.aggressive)){
                .initial_factor = frequent_resizing ? 2.5f : 2.0f,
                .decay_rate = 0.1f,
                .performance_threshold = current_capacity * 2
            };
            return calculate_aggressive_growth(&temp_policy, current_capacity, required_capacity);
            
        case SAMRENA_GROWTH_CONSERVATIVE:
            temp_policy.params.conservative = (typeof(temp_policy.params.conservative)){
                .waste_threshold = 0.2f,
                .min_increment = 1
            };
            return calculate_conservative_growth(&temp_policy, current_capacity, 
                                               required_capacity, stats->recent_operations);
            
        default:
            temp_policy.params.exponential = (typeof(temp_policy.params.exponential)){
                .factor = high_utilization ? 1.6f : 1.4f,
                .min_increment = 4,
                .max_capacity = SIZE_MAX / 2
            };
            return calculate_exponential_growth(&temp_policy, current_capacity, required_capacity);
    }
}
```

### Growth Policy Management
```c
// Main growth calculation function
static size_t calculate_new_capacity(SamrenaGrowthPolicy* policy,
                                    size_t current_capacity,
                                    size_t required_capacity,
                                    size_t current_size,
                                    const SamrenaGrowthStats* stats) {
    
    // Ensure required capacity is valid
    if (required_capacity <= current_capacity) {
        return current_capacity;
    }
    
    // Check against absolute limits
    if (required_capacity > policy->absolute_max_capacity) {
        return policy->absolute_max_capacity;
    }
    
    size_t new_capacity;
    
    // Calculate based on strategy
    switch (policy->strategy) {
        case SAMRENA_GROWTH_EXPONENTIAL:
            new_capacity = calculate_exponential_growth(policy, current_capacity, required_capacity);
            break;
            
        case SAMRENA_GROWTH_LINEAR:
            new_capacity = calculate_linear_growth(policy, current_capacity, required_capacity);
            break;
            
        case SAMRENA_GROWTH_FIBONACCI:
            new_capacity = calculate_fibonacci_growth(policy, current_capacity, required_capacity);
            break;
            
        case SAMRENA_GROWTH_CONSERVATIVE:
            new_capacity = calculate_conservative_growth(policy, current_capacity, 
                                                       required_capacity, current_size);
            break;
            
        case SAMRENA_GROWTH_AGGRESSIVE:
            new_capacity = calculate_aggressive_growth(policy, current_capacity, required_capacity);
            break;
            
        case SAMRENA_GROWTH_ADAPTIVE:
            new_capacity = calculate_adaptive_growth(policy, current_capacity, 
                                                   required_capacity, stats);
            break;
            
        case SAMRENA_GROWTH_CUSTOM:
            if (policy->params.custom.growth_function) {
                new_capacity = policy->params.custom.growth_function(current_capacity, 
                                                                    required_capacity,
                                                                    policy->params.custom.user_data);
            } else {
                // Fallback to exponential
                new_capacity = calculate_exponential_growth(&SAMRENA_GROWTH_POLICY_DEFAULT, 
                                                          current_capacity, required_capacity);
            }
            break;
            
        default:
            new_capacity = calculate_exponential_growth(&SAMRENA_GROWTH_POLICY_DEFAULT, 
                                                      current_capacity, required_capacity);
            break;
    }
    
    // Apply absolute limits
    if (new_capacity > policy->absolute_max_capacity) {
        new_capacity = policy->absolute_max_capacity;
    }
    
    // Check memory limits if specified
    if (policy->memory_limit_bytes > 0) {
        size_t element_size = (current_size > 0) ? 
            (current_capacity * sizeof(void*) / current_size) : sizeof(void*);
        size_t projected_bytes = new_capacity * element_size;
        
        if (projected_bytes > policy->memory_limit_bytes) {
            new_capacity = policy->memory_limit_bytes / element_size;
        }
    }
    
    return new_capacity;
}

// Update growth statistics
static void update_growth_stats(SamrenaGrowthStats* stats, 
                               size_t old_capacity, size_t new_capacity,
                               size_t current_size, bool is_growth) {
    if (is_growth) {
        stats->total_growths++;
    } else {
        stats->total_shrinks++;
    }
    
    // Update memory statistics
    stats->bytes_allocated = new_capacity * sizeof(void*);  // Approximation
    stats->bytes_wasted = (new_capacity - current_size) * sizeof(void*);
    
    // Update utilization (running average)
    double current_utilization = (new_capacity > 0) ? 
        (double)current_size / new_capacity : 1.0;
    
    stats->average_utilization = (stats->average_utilization * 0.9) + 
                                (current_utilization * 0.1);
    
    // Update operation tracking
    stats->recent_operations++;
    stats->last_growth_time = get_current_timestamp();
    
    // Calculate growth frequency
    if (stats->recent_operations > 0) {
        stats->growth_frequency = (float)stats->total_growths / stats->recent_operations;
    }
}
```

### Shrinking Strategies
```c
// Shrinking policy configuration
typedef struct {
    bool enabled;                    // Whether shrinking is enabled
    float utilization_threshold;     // Shrink when utilization drops below this
    size_t min_capacity;            // Never shrink below this capacity
    float shrink_factor;            // Factor to shrink by (e.g., 0.5 = half)
    size_t hysteresis_buffer;       // Buffer to prevent resize thrashing
} SamrenaShrinkPolicy;

// Calculate capacity after shrinking
static size_t calculate_shrink_capacity(const SamrenaShrinkPolicy* policy,
                                       size_t current_capacity,
                                       size_t current_size) {
    if (!policy->enabled) {
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
    
    // Ensure minimum capacity
    if (new_capacity < policy->min_capacity) {
        new_capacity = policy->min_capacity;
    }
    
    // Ensure we can still hold current elements
    if (new_capacity < current_size) {
        new_capacity = current_size;
    }
    
    return new_capacity;
}
```

## Integration with Vector System

### Enhanced Vector Structure
```c
// Extended vector structure with growth management
typedef struct {
    // Existing fields
    uint64_t size;
    uint64_t element_size;
    uint64_t capacity;
    void *data;
    Samrena *arena;
    bool owns_arena;
    bool can_shrink;
    
    // Growth management fields
    SamrenaGrowthPolicy* growth_policy;   // Current growth policy
    SamrenaGrowthStats growth_stats;      // Growth statistics
    SamrenaShrinkPolicy shrink_policy;    // Shrinking policy
    uint64_t last_access_time;            // For adaptive strategies
    
    // Policy flags
    bool auto_shrink;                     // Automatically shrink when appropriate
    bool track_statistics;                // Whether to track detailed statistics
} SamrenaVectorExtended;
```

### Growth Policy API
```c
// Set growth policy for a vector
int samrena_vector_set_growth_policy(SamrenaVector* vec, 
                                    const SamrenaGrowthPolicy* policy);

// Get current growth policy
const SamrenaGrowthPolicy* samrena_vector_get_growth_policy(const SamrenaVector* vec);

// Set shrinking policy
int samrena_vector_set_shrink_policy(SamrenaVector* vec, 
                                    const SamrenaShrinkPolicy* policy);

// Get growth statistics
SamrenaGrowthStats samrena_vector_get_growth_stats(const SamrenaVector* vec);

// Reset growth statistics
void samrena_vector_reset_growth_stats(SamrenaVector* vec);

// Suggest optimal growth policy based on usage pattern
SamrenaGrowthPolicy samrena_vector_suggest_growth_policy(const SamrenaVector* vec);

// Force evaluation of shrinking conditions
int samrena_vector_evaluate_shrink(SamrenaVector* vec);
```

## Usage Examples

### Basic Growth Policy Usage
```c
// Create vector with fast growth policy
SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 10);
samrena_vector_set_growth_policy(vec, &SAMRENA_GROWTH_POLICY_FAST);

// Vector will now grow aggressively when needed
for (int i = 0; i < 1000; i++) {
    samrena_vector_push_auto(vec, &i);
}

samrena_vector_destroy(vec);
```

### Custom Growth Strategy
```c
// Custom growth function that doubles capacity but caps at 1000
size_t custom_growth(size_t current, size_t required, void* user_data) {
    size_t max_cap = *(size_t*)user_data;
    size_t new_cap = current * 2;
    
    if (new_cap > max_cap) new_cap = max_cap;
    if (new_cap < required) new_cap = required;
    
    return new_cap;
}

// Use custom growth policy
size_t max_capacity = 1000;
SamrenaGrowthPolicy custom_policy = {
    .strategy = SAMRENA_GROWTH_CUSTOM,
    .params.custom = {
        .growth_function = custom_growth,
        .user_data = &max_capacity
    },
    .absolute_max_capacity = 1000
};

samrena_vector_set_growth_policy(vec, &custom_policy);
```

### Adaptive Growth with Monitoring
```c
// Enable adaptive growth with statistics tracking
SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 10);
samrena_vector_set_growth_policy(vec, &SAMRENA_GROWTH_POLICY_ADAPTIVE);

// Perform operations
for (int phase = 0; phase < 3; phase++) {
    for (int i = 0; i < 100; i++) {
        samrena_vector_push_auto(vec, &i);
    }
    
    // Check statistics after each phase
    SamrenaGrowthStats stats = samrena_vector_get_growth_stats(vec);
    printf("Phase %d: Growths=%zu, Utilization=%.2f%%\n", 
           phase, stats.total_growths, stats.average_utilization * 100);
}

samrena_vector_destroy(vec);
```

## Testing Strategy
- Growth pattern verification for each strategy
- Memory usage efficiency analysis
- Performance impact measurement
- Adaptive strategy learning validation
- Shrinking policy correctness
- Memory limit enforcement testing

## Performance Considerations
- Minimal overhead for growth calculations
- Efficient statistics tracking
- Optimal memory allocation patterns
- Reduced fragmentation through smart growth
- Balance between memory usage and performance