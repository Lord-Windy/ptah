# Task 08: Implement Error Handling

## Objective
Implement the error handling system designed in Task 07, providing robust error reporting and debugging capabilities while maintaining optimal performance for success paths.

## Dependencies
- Task 07: Design Error Handling System (must be completed)
- Understanding of thread-local storage requirements
- Integration with existing samrena error handling if any

## Implementation Plan

### 1. Core Error System Implementation
```c
// samrena_errors.c - Core error handling implementation

#include <stdio.h>
#include <string.h>
#include <assert.h>

// Thread-local error storage
_Thread_local SamrenaErrorInfo g_last_error = {
    .code = SAMRENA_SUCCESS,
    .message = NULL,
    .function = NULL,
    .file = NULL,
    .line = 0,
    .context = NULL
};

// Global error tracking setting
static bool g_error_tracking_enabled = true;
static SamrenaErrorCallback g_error_callback = NULL;

// Error message lookup table
static const char* error_messages[] = {
    [SAMRENA_SUCCESS] = "Success",
    [SAMRENA_ERROR_NULL_POINTER] = "Null pointer provided",
    [SAMRENA_ERROR_INVALID_SIZE] = "Invalid size parameter",
    [SAMRENA_ERROR_OUT_OF_BOUNDS] = "Index out of bounds",
    [SAMRENA_ERROR_INVALID_OPERATION] = "Invalid operation for current state",
    [SAMRENA_ERROR_ALLOCATION_FAILED] = "Memory allocation failed",
    [SAMRENA_ERROR_ARENA_EXHAUSTED] = "Arena has insufficient space",
    [SAMRENA_ERROR_INSUFFICIENT_CAPACITY] = "Vector capacity insufficient",
    [SAMRENA_ERROR_OWNERSHIP_CONFLICT] = "Operation conflicts with ownership model",
    [SAMRENA_ERROR_ARENA_MISMATCH] = "Arena mismatch between operations",
    [SAMRENA_ERROR_VECTOR_EMPTY] = "Operation invalid on empty vector",
    [SAMRENA_ERROR_VECTOR_FULL] = "Vector is at maximum capacity",
    [SAMRENA_ERROR_INVALID_STATE] = "Vector is in invalid state"
};
```

### 2. Error Setting and Retrieval
```c
void samrena_set_error_impl(SamrenaError code, const char* message, 
                           const char* function, const char* file, 
                           int line, void* context) {
    if (!g_error_tracking_enabled && code == SAMRENA_SUCCESS) return;
    
    g_last_error.code = code;
    g_last_error.message = message ? message : samrena_error_string(code);
    g_last_error.function = function;
    g_last_error.file = file;
    g_last_error.line = line;
    g_last_error.context = context;
    
    // Call callback if registered
    if (g_error_callback && code != SAMRENA_SUCCESS) {
        g_error_callback(&g_last_error);
    }
}

SamrenaErrorInfo samrena_get_last_error(void) {
    return g_last_error;
}

SamrenaError samrena_last_error_code(void) {
    return g_last_error.code;
}

const char* samrena_last_error_message(void) {
    return g_last_error.message;
}

void samrena_clear_error(void) {
    g_last_error.code = SAMRENA_SUCCESS;
    g_last_error.message = NULL;
    g_last_error.function = NULL;
    g_last_error.file = NULL;
    g_last_error.line = 0;
    g_last_error.context = NULL;
}
```

### 3. Error Message System
```c
const char* samrena_error_string(SamrenaError code) {
    int index = -code;  // Convert negative error codes to positive indices
    if (code == SAMRENA_SUCCESS) return error_messages[0];
    if (index < 0 || index >= (int)(sizeof(error_messages)/sizeof(error_messages[0]))) {
        return "Unknown error";
    }
    return error_messages[index] ? error_messages[index] : "Unknown error";
}

bool samrena_succeeded(SamrenaError result) {
    return result == SAMRENA_SUCCESS;
}

bool samrena_failed(SamrenaError result) {
    return result != SAMRENA_SUCCESS;
}

bool samrena_is_error(SamrenaError code) {
    return g_last_error.code == code;
}
```

### 4. Debug and Logging Functions
```c
void samrena_print_error(void) {
    if (g_last_error.code == SAMRENA_SUCCESS) return;
    
    fprintf(stderr, "Samrena Error: %s (%d)\n", 
            g_last_error.message, g_last_error.code);
    
    if (g_error_tracking_enabled && g_last_error.function) {
        fprintf(stderr, "  at %s() in %s:%d\n", 
                g_last_error.function, 
                g_last_error.file ? g_last_error.file : "unknown", 
                g_last_error.line);
    }
}

int samrena_format_error(char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) return -1;
    if (g_last_error.code == SAMRENA_SUCCESS) {
        buffer[0] = '\0';
        return 0;
    }
    
    if (g_error_tracking_enabled && g_last_error.function) {
        return snprintf(buffer, buffer_size, 
                       "%s (%d) at %s() in %s:%d",
                       g_last_error.message, g_last_error.code,
                       g_last_error.function,
                       g_last_error.file ? g_last_error.file : "unknown",
                       g_last_error.line);
    } else {
        return snprintf(buffer, buffer_size, "%s (%d)", 
                       g_last_error.message, g_last_error.code);
    }
}

void samrena_set_error_callback(SamrenaErrorCallback callback) {
    g_error_callback = callback;
}

void samrena_set_error_tracking(bool enabled) {
    g_error_tracking_enabled = enabled;
}
```

### 5. Validation Helper Functions
```c
// samrena_vector_validation.c
SamrenaError validate_vector(const SamrenaVector* vec) {
    if (!vec) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Vector is NULL", vec);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    if (!vec->arena) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_INVALID_STATE, "Vector arena is NULL", vec);
        return SAMRENA_ERROR_INVALID_STATE;
    }
    
    if (vec->size > vec->capacity) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_INVALID_STATE, "Vector size exceeds capacity", vec);
        return SAMRENA_ERROR_INVALID_STATE;
    }
    
    if (vec->capacity > 0 && !vec->data) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_INVALID_STATE, "Vector data is NULL but capacity > 0", vec);
        return SAMRENA_ERROR_INVALID_STATE;
    }
    
    return SAMRENA_SUCCESS;
}

SamrenaError validate_index(const SamrenaVector* vec, size_t index) {
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    if (index >= vec->size) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_OUT_OF_BOUNDS, "Index exceeds vector size", vec);
        return SAMRENA_ERROR_OUT_OF_BOUNDS;
    }
    
    return SAMRENA_SUCCESS;
}

SamrenaError validate_element_pointer(const void* element) {
    if (!element) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Element pointer is NULL", element);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    return SAMRENA_SUCCESS;
}
```

### 6. Updated Vector Functions with Error Handling
```c
// Example: Updated samrena_vector_get with proper error handling
int samrena_vector_get(const SamrenaVector* vec, size_t index, void* out_element) {
    // Clear previous error
    samrena_clear_error();
    
    // Validate parameters
    SamrenaError err = validate_index(vec, index);
    if (err != SAMRENA_SUCCESS) return err;
    
    err = validate_element_pointer(out_element);
    if (err != SAMRENA_SUCCESS) return err;
    
    // Perform operation
    const char* src = (const char*)vec->data + (index * vec->element_size);
    memcpy(out_element, src, vec->element_size);
    
    return SAMRENA_SUCCESS;
}

// Example: Updated samrena_vector_push with error handling
int samrena_vector_push_auto(SamrenaVector* vec, const void* element) {
    samrena_clear_error();
    
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    err = validate_element_pointer(element);
    if (err != SAMRENA_SUCCESS) return err;
    
    // Check if resize is needed
    if (vec->size >= vec->capacity) {
        size_t new_capacity = calculate_new_capacity(vec, vec->size + 1);
        err = samrena_vector_reserve_auto(vec, new_capacity);
        if (err != SAMRENA_SUCCESS) return err;
    }
    
    // Add element
    char* dst = (char*)vec->data + (vec->size * vec->element_size);
    memcpy(dst, element, vec->element_size);
    vec->size++;
    
    return SAMRENA_SUCCESS;
}
```

### 7. Performance-Optimized Versions
```c
// Fast versions for hot paths (minimal error checking)
static inline int samrena_vector_push_fast(SamrenaVector* vec, const void* element) {
    assert(vec != NULL);
    assert(element != NULL);
    assert(vec->size < vec->capacity);  // Caller ensures capacity
    
    char* dst = (char*)vec->data + (vec->size * vec->element_size);
    memcpy(dst, element, vec->element_size);
    vec->size++;
    
    return SAMRENA_SUCCESS;
}

static inline void* samrena_vector_at_fast(SamrenaVector* vec, size_t index) {
    assert(vec != NULL);
    assert(index < vec->size);
    
    return (char*)vec->data + (index * vec->element_size);
}
```

### 8. Error Context Helpers
```c
// Helper to create error context for debugging
typedef struct {
    const SamrenaVector* vector;
    size_t requested_index;
    size_t requested_capacity;
    const char* operation;
} SamrenaVectorErrorContext;

static void set_vector_error_context(SamrenaError code, const char* message,
                                    const SamrenaVector* vec, const char* operation,
                                    size_t index, size_t capacity) {
    static SamrenaVectorErrorContext context;
    context.vector = vec;
    context.requested_index = index;
    context.requested_capacity = capacity;
    context.operation = operation;
    
    SAMRENA_SET_ERROR(code, message, &context);
}
```

## Testing Implementation

### Unit Tests for Error System
```c
// tests/test_error_handling.c
void test_error_basic_functionality(void) {
    samrena_clear_error();
    assert(samrena_last_error_code() == SAMRENA_SUCCESS);
    
    SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Test error", NULL);
    assert(samrena_last_error_code() == SAMRENA_ERROR_NULL_POINTER);
    assert(strcmp(samrena_last_error_message(), "Test error") == 0);
    
    samrena_clear_error();
    assert(samrena_last_error_code() == SAMRENA_SUCCESS);
}

void test_error_thread_safety(void) {
    // Test that thread-local storage works correctly
    // (requires threading test framework)
}

void test_validation_functions(void) {
    SamrenaVector invalid_vec = {0};
    assert(validate_vector(&invalid_vec) != SAMRENA_SUCCESS);
    assert(validate_index(NULL, 0) == SAMRENA_ERROR_NULL_POINTER);
    assert(validate_element_pointer(NULL) == SAMRENA_ERROR_NULL_POINTER);
}
```

### Integration Tests
```c
void test_vector_operations_with_errors(void) {
    SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 2);
    assert(vec != NULL);
    
    // Test bounds checking
    int value;
    assert(samrena_vector_get(vec, 10, &value) == SAMRENA_ERROR_OUT_OF_BOUNDS);
    
    // Test null parameter
    assert(samrena_vector_get(vec, 0, NULL) == SAMRENA_ERROR_NULL_POINTER);
    
    samrena_vector_destroy(vec);
}
```

## Performance Benchmarks
- Error handling overhead measurement
- Success path performance impact
- Thread-local storage access cost
- Validation function efficiency

## Integration Notes
- Update all existing vector functions
- Maintain backward compatibility
- Document migration path
- Consider conditional compilation for release builds