# Task 22: Performance Optimizations

## Objective
Implement comprehensive performance optimizations for Samvector operations, including SIMD vectorization, cache-aware algorithms, memory prefetching, and architecture-specific optimizations to maximize throughput and minimize latency.

## Dependencies
- All previous tasks (complete vector implementation)
- Understanding of target architecture capabilities
- SIMD instruction set availability (SSE, AVX, NEON)
- Cache hierarchy characteristics

## Implementation Plan

### 1. Performance Analysis and Profiling Infrastructure
```c
// samrena_vector_perf.c - Performance optimization implementations

#include "samrena_vector.h"
#include <immintrin.h>  // For SIMD intrinsics
#include <cpuid.h>      // For CPU feature detection

// Performance configuration
typedef struct {
    bool simd_enabled;           // Use SIMD instructions when available
    bool prefetch_enabled;       // Use memory prefetching
    bool cache_optimized;        // Use cache-aware algorithms
    size_t cache_line_size;      // L1 cache line size (typically 64 bytes)
    size_t l1_cache_size;        // L1 cache size
    size_t l2_cache_size;        // L2 cache size
    size_t simd_threshold;       // Minimum size for SIMD operations
    size_t prefetch_distance;    // How far ahead to prefetch
} SamrenaPerformanceConfig;

// CPU capability detection
typedef struct {
    bool has_sse2;
    bool has_sse4_1;
    bool has_avx;
    bool has_avx2;
    bool has_avx512;
    bool has_neon;  // ARM NEON
    int cache_line_size;
    int l1_cache_size;
    int l2_cache_size;
} SamrenaCpuInfo;

// Detect CPU capabilities
static SamrenaCpuInfo detect_cpu_capabilities(void) {
    SamrenaCpuInfo info = {0};
    
#ifdef __x86_64__
    unsigned int eax, ebx, ecx, edx;
    
    // Check for CPUID support
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        info.has_sse2 = (edx & bit_SSE2) != 0;
        info.has_sse4_1 = (ecx & bit_SSE4_1) != 0;
        info.has_avx = (ecx & bit_AVX) != 0;
    }
    
    if (__get_cpuid_max(0, NULL) >= 7) {
        __cpuid_count(7, 0, eax, ebx, ecx, edx);
        info.has_avx2 = (ebx & bit_AVX2) != 0;
        info.has_avx512 = (ebx & bit_AVX512F) != 0;
    }
    
    // Get cache information
    if (__get_cpuid(0x80000006, &eax, &ebx, &ecx, &edx)) {
        info.cache_line_size = ecx & 0xFF;
        info.l1_cache_size = (ecx >> 16) & 0xFFFF;
        info.l2_cache_size = (ecx >> 16) & 0xFFFF;
    }
    
#elif defined(__aarch64__)
    // ARM NEON detection
    info.has_neon = true;  // NEON is standard on AArch64
    info.cache_line_size = 64;  // Common on ARM
    
#endif
    
    // Defaults if detection fails
    if (info.cache_line_size == 0) info.cache_line_size = 64;
    if (info.l1_cache_size == 0) info.l1_cache_size = 32 * 1024;
    if (info.l2_cache_size == 0) info.l2_cache_size = 256 * 1024;
    
    return info;
}

// Global performance configuration
static SamrenaPerformanceConfig g_perf_config = {0};
static SamrenaCpuInfo g_cpu_info = {0};
static bool g_perf_initialized = false;

// Initialize performance subsystem
void samrena_vector_init_performance(void) {
    if (g_perf_initialized) return;
    
    g_cpu_info = detect_cpu_capabilities();
    
    g_perf_config = (SamrenaPerformanceConfig){
        .simd_enabled = g_cpu_info.has_sse2 || g_cpu_info.has_neon,
        .prefetch_enabled = true,
        .cache_optimized = true,
        .cache_line_size = g_cpu_info.cache_line_size,
        .l1_cache_size = g_cpu_info.l1_cache_size,
        .l2_cache_size = g_cpu_info.l2_cache_size,
        .simd_threshold = 16,  // Use SIMD for 16+ elements
        .prefetch_distance = 2  // Prefetch 2 cache lines ahead
    };
    
    g_perf_initialized = true;
}
```

### 2. Memory Prefetching Optimizations
```c
// Memory prefetching utilities
static inline void prefetch_read(const void* addr) {
    if (g_perf_config.prefetch_enabled) {
#ifdef __builtin_prefetch
        __builtin_prefetch(addr, 0, 3);  // Read, high temporal locality
#elif defined(_MSC_VER)
        _mm_prefetch((const char*)addr, _MM_HINT_T0);
#endif
    }
}

static inline void prefetch_write(void* addr) {
    if (g_perf_config.prefetch_enabled) {
#ifdef __builtin_prefetch
        __builtin_prefetch(addr, 1, 3);  // Write, high temporal locality
#elif defined(_MSC_VER)
        _mm_prefetch((const char*)addr, _MM_HINT_T0);
#endif
    }
}

// Optimized bulk copy with prefetching
static void optimized_bulk_copy(void* dest, const void* src, size_t size) {
    if (!g_perf_config.cache_optimized || size < g_perf_config.cache_line_size * 4) {
        // Small copies - use standard memcpy
        memcpy(dest, src, size);
        return;
    }
    
    const size_t cache_line_size = g_perf_config.cache_line_size;
    const size_t prefetch_distance = g_perf_config.prefetch_distance * cache_line_size;
    
    const char* src_ptr = (const char*)src;
    char* dest_ptr = (char*)dest;
    size_t remaining = size;
    
    // Prefetch initial data
    for (size_t i = 0; i < prefetch_distance && i < size; i += cache_line_size) {
        prefetch_read(src_ptr + i);
        prefetch_write(dest_ptr + i);
    }
    
    // Copy with prefetching
    while (remaining >= cache_line_size) {
        // Prefetch ahead
        if (remaining > prefetch_distance) {
            prefetch_read(src_ptr + prefetch_distance);
            prefetch_write(dest_ptr + prefetch_distance);
        }
        
        // Copy cache line
        memcpy(dest_ptr, src_ptr, cache_line_size);
        
        src_ptr += cache_line_size;
        dest_ptr += cache_line_size;
        remaining -= cache_line_size;
    }
    
    // Copy remaining bytes
    if (remaining > 0) {
        memcpy(dest_ptr, src_ptr, remaining);
    }
}
```

### 3. SIMD-Optimized Operations
```c
// SIMD-optimized element comparison for sorted operations
#ifdef __x86_64__

// SSE2 optimized integer comparison
static inline int simd_compare_int32_sse2(const int32_t* a, const int32_t* b, size_t count) {
    if (!g_cpu_info.has_sse2 || count < 4) {
        return memcmp(a, b, count * sizeof(int32_t));
    }
    
    const __m128i* va = (const __m128i*)a;
    const __m128i* vb = (const __m128i*)b;
    size_t simd_count = count / 4;
    
    for (size_t i = 0; i < simd_count; i++) {
        __m128i cmp = _mm_cmpeq_epi32(va[i], vb[i]);
        int mask = _mm_movemask_epi8(cmp);
        
        if (mask != 0xFFFF) {
            // Found difference - fall back to scalar comparison
            size_t base_index = i * 4;
            for (size_t j = 0; j < 4; j++) {
                if (a[base_index + j] != b[base_index + j]) {
                    return (a[base_index + j] < b[base_index + j]) ? -1 : 1;
                }
            }
        }
    }
    
    // Compare remaining elements
    size_t remaining = count % 4;
    if (remaining > 0) {
        return memcmp(a + count - remaining, b + count - remaining, 
                     remaining * sizeof(int32_t));
    }
    
    return 0;  // All elements equal
}

// AVX2 optimized integer search
static inline size_t simd_find_int32_avx2(const int32_t* data, size_t count, int32_t target) {
    if (!g_cpu_info.has_avx2 || count < 8) {
        // Fall back to scalar search
        for (size_t i = 0; i < count; i++) {
            if (data[i] == target) return i;
        }
        return SAMRENA_NOT_FOUND;
    }
    
    const __m256i target_vec = _mm256_set1_epi32(target);
    const __m256i* data_vec = (const __m256i*)data;
    size_t simd_count = count / 8;
    
    for (size_t i = 0; i < simd_count; i++) {
        __m256i cmp = _mm256_cmpeq_epi32(data_vec[i], target_vec);
        int mask = _mm256_movemask_epi8(cmp);
        
        if (mask != 0) {
            // Found match - find exact position
            for (size_t j = 0; j < 8; j++) {
                if (data[i * 8 + j] == target) {
                    return i * 8 + j;
                }
            }
        }
    }
    
    // Search remaining elements
    for (size_t i = simd_count * 8; i < count; i++) {
        if (data[i] == target) return i;
    }
    
    return SAMRENA_NOT_FOUND;
}

#endif // __x86_64__

#ifdef __aarch64__

// NEON optimized operations for ARM
static inline size_t simd_find_int32_neon(const int32_t* data, size_t count, int32_t target) {
    if (!g_cpu_info.has_neon || count < 4) {
        for (size_t i = 0; i < count; i++) {
            if (data[i] == target) return i;
        }
        return SAMRENA_NOT_FOUND;
    }
    
    const int32x4_t target_vec = vdupq_n_s32(target);
    size_t simd_count = count / 4;
    
    for (size_t i = 0; i < simd_count; i++) {
        int32x4_t data_vec = vld1q_s32(&data[i * 4]);
        uint32x4_t cmp = vceqq_s32(data_vec, target_vec);
        
        // Check if any lane matched
        uint32_t mask = vgetq_lane_u32(cmp, 0) | vgetq_lane_u32(cmp, 1) |
                       vgetq_lane_u32(cmp, 2) | vgetq_lane_u32(cmp, 3);
        
        if (mask != 0) {
            // Found match - find exact position
            for (size_t j = 0; j < 4; j++) {
                if (data[i * 4 + j] == target) {
                    return i * 4 + j;
                }
            }
        }
    }
    
    // Search remaining elements
    for (size_t i = simd_count * 4; i < count; i++) {
        if (data[i] == target) return i;
    }
    
    return SAMRENA_NOT_FOUND;
}

#endif // __aarch64__
```

### 4. Cache-Aware Algorithms
```c
// Cache-aware bulk operations
static void cache_aware_bulk_insert(SamrenaVector* vec, size_t index, 
                                   const void* elements, size_t count) {
    if (!g_perf_config.cache_optimized || count < g_perf_config.simd_threshold) {
        // Use standard insertion for small operations
        samrena_vector_insert_array(vec, index, elements, count);
        return;
    }
    
    const size_t element_size = vec->element_size;
    const size_t cache_line_elements = g_perf_config.cache_line_size / element_size;
    
    // Ensure sufficient capacity
    size_t required_capacity = vec->size + count;
    if (required_capacity > vec->capacity) {
        int result = samrena_vector_reserve_auto(vec, required_capacity);
        if (result != SAMRENA_SUCCESS) return;
    }
    
    // Calculate memory regions
    char* data = (char*)vec->data;
    char* insert_point = data + (index * element_size);
    char* end_of_data = data + (vec->size * element_size);
    size_t move_size = (vec->size - index) * element_size;
    
    // Use optimized bulk copy for large moves
    if (move_size > 0) {
        // Move existing elements with cache optimization
        optimized_bulk_copy(insert_point + (count * element_size), 
                           insert_point, move_size);
    }
    
    // Insert new elements with prefetching
    if (count > cache_line_elements) {
        const char* src = (const char*)elements;
        char* dest = insert_point;
        size_t remaining = count * element_size;
        
        // Process in cache-line-sized chunks
        while (remaining >= g_perf_config.cache_line_size) {
            prefetch_read(src + g_perf_config.cache_line_size);
            prefetch_write(dest + g_perf_config.cache_line_size);
            
            memcpy(dest, src, g_perf_config.cache_line_size);
            
            src += g_perf_config.cache_line_size;
            dest += g_perf_config.cache_line_size;
            remaining -= g_perf_config.cache_line_size;
        }
        
        // Copy remaining data
        if (remaining > 0) {
            memcpy(dest, src, remaining);
        }
    } else {
        memcpy(insert_point, elements, count * element_size);
    }
    
    vec->size += count;
}

// Cache-friendly iteration for large operations
static void cache_friendly_foreach(SamrenaVector* vec, 
                                  SamrenaVectorCallback callback,
                                  void* user_data) {
    if (!g_perf_config.cache_optimized || vec->size < g_perf_config.simd_threshold) {
        // Use standard iteration
        samrena_vector_foreach(vec, callback, user_data);
        return;
    }
    
    const size_t element_size = vec->element_size;
    const size_t elements_per_cache_line = g_perf_config.cache_line_size / element_size;
    const size_t prefetch_distance = g_perf_config.prefetch_distance * elements_per_cache_line;
    
    char* data = (char*)vec->data;
    
    // Prefetch initial cache lines
    for (size_t i = 0; i < prefetch_distance && i < vec->size; i += elements_per_cache_line) {
        prefetch_read(data + (i * element_size));
    }
    
    // Process elements with prefetching
    for (size_t i = 0; i < vec->size; i++) {
        // Prefetch ahead
        if (i + prefetch_distance < vec->size) {
            prefetch_read(data + ((i + prefetch_distance) * element_size));
        }
        
        void* element = data + (i * element_size);
        callback(element, i, user_data);
    }
}
```

### 5. Optimized Search Operations
```c
// High-performance search with multiple optimizations
size_t samrena_vector_find_optimized(const SamrenaVector* vec, const void* target,
                                    SamrenaCompareFn compare_fn, void* user_data) {
    if (validate_vector(vec) != SAMRENA_SUCCESS || !target) {
        return SAMRENA_NOT_FOUND;
    }
    
    if (vec->size == 0) return SAMRENA_NOT_FOUND;
    
    // Try SIMD optimizations for common types
    if (!compare_fn && g_perf_config.simd_enabled) {
        if (vec->element_size == sizeof(int32_t) && vec->size >= g_perf_config.simd_threshold) {
            int32_t target_val = *(const int32_t*)target;
            
#ifdef __x86_64__
            if (g_cpu_info.has_avx2) {
                return simd_find_int32_avx2((const int32_t*)vec->data, vec->size, target_val);
            }
#endif
#ifdef __aarch64__
            if (g_cpu_info.has_neon) {
                return simd_find_int32_neon((const int32_t*)vec->data, vec->size, target_val);
            }
#endif
        }
        
        // For other types, use optimized memcmp
        if (vec->size >= g_perf_config.simd_threshold) {
            const char* data = (const char*)vec->data;
            const size_t element_size = vec->element_size;
            
            for (size_t i = 0; i < vec->size; i++) {
                // Prefetch ahead
                if (i + g_perf_config.prefetch_distance < vec->size) {
                    prefetch_read(data + ((i + g_perf_config.prefetch_distance) * element_size));
                }
                
                if (memcmp(data + (i * element_size), target, element_size) == 0) {
                    return i;
                }
            }
            
            return SAMRENA_NOT_FOUND;
        }
    }
    
    // Fall back to standard search
    return samrena_vector_find(vec, target, compare_fn, user_data);
}

// Optimized binary search with prefetching
size_t samrena_vector_binary_search_optimized(const SamrenaVector* vec, const void* target,
                                             SamrenaCompareFn compare_fn, void* user_data) {
    if (validate_vector(vec) != SAMRENA_SUCCESS || !target || !compare_fn) {
        return SAMRENA_NOT_FOUND;
    }
    
    if (vec->size == 0) return SAMRENA_NOT_FOUND;
    
    const char* data = (const char*)vec->data;
    const size_t element_size = vec->element_size;
    size_t left = 0;
    size_t right = vec->size;
    
    // Prefetch likely access points
    if (g_perf_config.prefetch_enabled && vec->size >= g_perf_config.simd_threshold) {
        // Prefetch middle elements that are likely to be accessed
        size_t mid = vec->size / 2;
        prefetch_read(data + (mid * element_size));
        
        if (vec->size > 16) {
            prefetch_read(data + ((mid / 2) * element_size));
            prefetch_read(data + ((mid + mid / 2) * element_size));
        }
    }
    
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        const void* mid_element = data + (mid * element_size);
        
        // Prefetch likely next access points
        if (g_perf_config.prefetch_enabled && (right - left) > 8) {
            if (mid > left) {
                size_t left_mid = left + (mid - left) / 2;
                prefetch_read(data + (left_mid * element_size));
            }
            if (mid + 1 < right) {
                size_t right_mid = mid + 1 + (right - mid - 1) / 2;
                prefetch_read(data + (right_mid * element_size));
            }
        }
        
        int cmp = compare_fn(target, mid_element, user_data);
        
        if (cmp == 0) {
            return mid;
        } else if (cmp < 0) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }
    
    return SAMRENA_NOT_FOUND;
}
```

### 6. Sorting Optimizations
```c
// Cache-aware quicksort with SIMD optimizations
static void optimized_quicksort_partition(void* data, size_t element_size,
                                         size_t start, size_t end,
                                         SamrenaCompareFn compare_fn, void* user_data) {
    // Use cache-aware partitioning for large arrays
    if (g_perf_config.cache_optimized && (end - start) > g_perf_config.simd_threshold) {
        const size_t cache_line_elements = g_perf_config.cache_line_size / element_size;
        
        // Process in cache-line-sized blocks when possible
        if ((end - start) > cache_line_elements * 4) {
            // Use blocked partitioning to improve cache locality
            // Implementation would be complex - using standard partitioning for now
        }
    }
    
    // Standard partitioning with prefetching
    char* array = (char*)data;
    size_t pivot_index = start + (end - start) / 2;
    
    // Move pivot to start
    if (pivot_index != start) {
        char temp[element_size];
        memcpy(temp, array + (start * element_size), element_size);
        memcpy(array + (start * element_size), array + (pivot_index * element_size), element_size);
        memcpy(array + (pivot_index * element_size), temp, element_size);
    }
    
    size_t left = start + 1;
    size_t right = end - 1;
    
    while (left <= right && left < end && right < end) {
        // Prefetch elements we'll compare soon
        if (g_perf_config.prefetch_enabled) {
            if (left + 2 < end) {
                prefetch_read(array + ((left + 2) * element_size));
            }
            if (right > 2 && right - 2 >= start) {
                prefetch_read(array + ((right - 2) * element_size));
            }
        }
        
        // Find elements to swap
        while (left < end && 
               compare_fn(array + (left * element_size), 
                         array + (start * element_size), user_data) <= 0) {
            left++;
        }
        
        while (right >= start && 
               compare_fn(array + (right * element_size), 
                         array + (start * element_size), user_data) > 0) {
            if (right == 0) break;
            right--;
        }
        
        if (left < right) {
            // Swap elements
            char temp[element_size];
            memcpy(temp, array + (left * element_size), element_size);
            memcpy(array + (left * element_size), array + (right * element_size), element_size);
            memcpy(array + (right * element_size), temp, element_size);
            
            left++;
            if (right > 0) right--;
        } else {
            break;
        }
    }
    
    // Place pivot in final position
    if (right < end && right != start) {
        char temp[element_size];
        memcpy(temp, array + (start * element_size), element_size);
        memcpy(array + (start * element_size), array + (right * element_size), element_size);
        memcpy(array + (right * element_size), temp, element_size);
    }
}
```

### 7. Performance Monitoring and Tuning
```c
// Performance statistics
typedef struct {
    uint64_t operations_count;
    uint64_t cache_misses;       // Estimated
    uint64_t simd_operations;
    uint64_t prefetch_operations;
    double average_operation_time;
    size_t memory_bandwidth_used;
} SamrenaPerformanceStats;

static SamrenaPerformanceStats g_perf_stats = {0};

// Performance monitoring
void samrena_vector_start_performance_monitoring(void) {
    memset(&g_perf_stats, 0, sizeof(g_perf_stats));
}

SamrenaPerformanceStats samrena_vector_get_performance_stats(void) {
    return g_perf_stats;
}

// Automatic performance tuning
void samrena_vector_auto_tune_performance(void) {
    // Analyze recent performance statistics
    if (g_perf_stats.operations_count < 1000) {
        return;  // Not enough data for tuning
    }
    
    // Adjust SIMD threshold based on effectiveness
    if (g_perf_stats.simd_operations > 0) {
        double simd_ratio = (double)g_perf_stats.simd_operations / g_perf_stats.operations_count;
        
        if (simd_ratio < 0.1) {
            // SIMD not being used much - increase threshold
            g_perf_config.simd_threshold *= 2;
        } else if (simd_ratio > 0.8) {
            // SIMD being used heavily - decrease threshold
            g_perf_config.simd_threshold = (g_perf_config.simd_threshold * 3) / 4;
            if (g_perf_config.simd_threshold < 4) {
                g_perf_config.simd_threshold = 4;
            }
        }
    }
    
    // Adjust prefetch distance based on estimated cache performance
    if (g_perf_stats.cache_misses > g_perf_stats.operations_count / 2) {
        // High cache miss rate - increase prefetch distance
        if (g_perf_config.prefetch_distance < 8) {
            g_perf_config.prefetch_distance++;
        }
    } else if (g_perf_stats.cache_misses < g_perf_stats.operations_count / 10) {
        // Low cache miss rate - might be over-prefetching
        if (g_perf_config.prefetch_distance > 1) {
            g_perf_config.prefetch_distance--;
        }
    }
}
```

### 8. Public Performance API
```c
// Public performance configuration API
void samrena_vector_set_performance_config(const SamrenaPerformanceConfig* config) {
    if (config) {
        g_perf_config = *config;
    }
}

SamrenaPerformanceConfig samrena_vector_get_performance_config(void) {
    return g_perf_config;
}

void samrena_vector_enable_simd(bool enabled) {
    g_perf_config.simd_enabled = enabled && (g_cpu_info.has_sse2 || g_cpu_info.has_neon);
}

void samrena_vector_enable_prefetch(bool enabled) {
    g_perf_config.prefetch_enabled = enabled;
}

void samrena_vector_set_simd_threshold(size_t threshold) {
    g_perf_config.simd_threshold = threshold;
}

// Optimized versions of common operations
#define SAMRENA_VECTOR_DEFINE_OPTIMIZED_OPS(T) \
    static inline size_t samrena_vector_##T##_find_fast(const SamrenaVector_##T* vec, T target) { \
        if (samrena_vector_##T##_check_valid(vec) != SAMRENA_SUCCESS) { \
            return SAMRENA_NOT_FOUND; \
        } \
        return samrena_vector_find_optimized(vec->_internal, &target, NULL, NULL); \
    } \
    \
    static inline void samrena_vector_##T##_foreach_fast(SamrenaVector_##T* vec, \
                                                        void (*callback)(T*, size_t, void*), \
                                                        void* user_data) { \
        if (samrena_vector_##T##_check_valid(vec) != SAMRENA_SUCCESS || !callback) return; \
        \
        struct callback_wrapper { \
            void (*typed_callback)(T*, size_t, void*); \
            void* user_data; \
        } wrapper = {callback, user_data}; \
        \
        void generic_callback(void* element, size_t index, void* wrapper_data) { \
            struct callback_wrapper* w = (struct callback_wrapper*)wrapper_data; \
            w->typed_callback((T*)element, index, w->user_data); \
        } \
        \
        cache_friendly_foreach(vec->_internal, generic_callback, &wrapper); \
    }

// Define optimized operations for common types
SAMRENA_VECTOR_DEFINE_OPTIMIZED_OPS(int)
SAMRENA_VECTOR_DEFINE_OPTIMIZED_OPS(float)
SAMRENA_VECTOR_DEFINE_OPTIMIZED_OPS(double)
```

## Testing and Benchmarking

### Performance Test Suite
```c
void benchmark_performance_optimizations() {
    samrena_vector_init_performance();
    
    const size_t test_sizes[] = {1000, 10000, 100000, 1000000};
    const size_t num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    
    printf("Performance Optimization Benchmarks:\n");
    printf("CPU Features: SSE2=%d, AVX2=%d, NEON=%d\n", 
           g_cpu_info.has_sse2, g_cpu_info.has_avx2, g_cpu_info.has_neon);
    
    for (size_t i = 0; i < num_sizes; i++) {
        printf("\nTesting with %zu elements:\n", test_sizes[i]);
        
        // Create test vector
        SamrenaVectorInt vec = SAMRENA_INT_VECTOR(test_sizes[i]);
        for (size_t j = 0; j < test_sizes[i]; j++) {
            samrena_vector_int_push(&vec, j);
        }
        
        // Benchmark search operations
        int target = test_sizes[i] / 2;
        
        // Standard search
        clock_t start = clock();
        for (int k = 0; k < 1000; k++) {
            samrena_vector_int_find(&vec, target, NULL);
        }
        clock_t standard_time = clock() - start;
        
        // Optimized search
        start = clock();
        for (int k = 0; k < 1000; k++) {
            samrena_vector_int_find_fast(&vec, target);
        }
        clock_t optimized_time = clock() - start;
        
        printf("  Search - Standard: %ldms, Optimized: %ldms, Speedup: %.2fx\n",
               standard_time, optimized_time, 
               (double)standard_time / optimized_time);
        
        // Benchmark iteration
        volatile int sum = 0;
        
        void count_callback(int* element, size_t index, void* user_data) {
            (void)index;
            *(int*)user_data += *element;
        }
        
        // Standard iteration
        start = clock();
        samrena_vector_int_foreach(&vec, count_callback, &sum);
        clock_t iter_standard = clock() - start;
        
        // Optimized iteration
        sum = 0;
        start = clock();
        samrena_vector_int_foreach_fast(&vec, count_callback, &sum);
        clock_t iter_optimized = clock() - start;
        
        printf("  Iteration - Standard: %ldms, Optimized: %ldms, Speedup: %.2fx\n",
               iter_standard, iter_optimized,
               (double)iter_standard / iter_optimized);
        
        samrena_vector_int_destroy(&vec);
    }
    
    // Print performance statistics
    SamrenaPerformanceStats stats = samrena_vector_get_performance_stats();
    printf("\nPerformance Statistics:\n");
    printf("  Operations: %lu\n", stats.operations_count);
    printf("  SIMD operations: %lu\n", stats.simd_operations);
    printf("  Prefetch operations: %lu\n", stats.prefetch_operations);
}
```

## Integration Notes
- Optional feature that can be disabled at compile time
- Automatic CPU capability detection
- Graceful fallback to standard implementations
- Configurable performance parameters
- Compatible with existing vector API
- Minimal overhead when optimizations are disabled
- Platform-specific optimizations with portable fallbacks