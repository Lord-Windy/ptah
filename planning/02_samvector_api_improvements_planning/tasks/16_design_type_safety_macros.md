# Task 16: Design Type Safety Macros

## Objective
Design a comprehensive type safety system using C macros that provides compile-time type checking, eliminates void pointer casts, and creates type-specific vector APIs while maintaining compatibility with the generic vector implementation.

## Requirements
- Compile-time type safety without runtime overhead
- Automatic type-specific function generation
- Integration with existing generic vector API
- Support for common C types and user-defined structures
- Clear error messages for type mismatches
- Backward compatibility with existing code

## Proposed Type Safety System

### Core Type-Safe Vector Definition
```c
// Macro to define a type-safe vector wrapper
#define SAMRENA_VECTOR_DEFINE_TYPE(T) \
    typedef struct { \
        SamrenaVector* _internal; \
        /* Type marker for compile-time checking */ \
        T* _type_marker; \
    } SamrenaVector_##T; \
    \
    /* Static assertions for type compatibility */ \
    _Static_assert(sizeof(T) > 0, "Type " #T " must have non-zero size"); \
    \
    /* Type-safe initialization */ \
    static inline SamrenaVector_##T samrena_vector_##T##_init_owned(size_t capacity) { \
        SamrenaVector_##T typed_vec = {0}; \
        typed_vec._internal = samrena_vector_init_owned(sizeof(T), capacity); \
        return typed_vec; \
    } \
    \
    static inline SamrenaVector_##T samrena_vector_##T##_init_with_arena( \
        Samrena* arena, size_t capacity) { \
        SamrenaVector_##T typed_vec = {0}; \
        typed_vec._internal = samrena_vector_init_with_arena(arena, sizeof(T), capacity); \
        return typed_vec; \
    } \
    \
    /* Type-safe destruction */ \
    static inline void samrena_vector_##T##_destroy(SamrenaVector_##T* vec) { \
        if (vec && vec->_internal) { \
            samrena_vector_destroy(vec->_internal); \
            vec->_internal = NULL; \
        } \
    } \
    \
    /* Type-safe element access */ \
    static inline T* samrena_vector_##T##_at(SamrenaVector_##T* vec, size_t index) { \
        if (!vec || !vec->_internal) return NULL; \
        return (T*)samrena_vector_at(vec->_internal, index); \
    } \
    \
    static inline const T* samrena_vector_##T##_at_const( \
        const SamrenaVector_##T* vec, size_t index) { \
        if (!vec || !vec->_internal) return NULL; \
        return (const T*)samrena_vector_at_const(vec->_internal, index); \
    } \
    \
    /* Type-safe element modification */ \
    static inline int samrena_vector_##T##_push(SamrenaVector_##T* vec, T value) { \
        if (!vec || !vec->_internal) return SAMRENA_ERROR_NULL_POINTER; \
        return samrena_vector_push_auto(vec->_internal, &value); \
    } \
    \
    static inline int samrena_vector_##T##_get(const SamrenaVector_##T* vec, \
                                               size_t index, T* out_value) { \
        if (!vec || !vec->_internal || !out_value) return SAMRENA_ERROR_NULL_POINTER; \
        return samrena_vector_get(vec->_internal, index, out_value); \
    } \
    \
    static inline int samrena_vector_##T##_set(SamrenaVector_##T* vec, \
                                               size_t index, T value) { \
        if (!vec || !vec->_internal) return SAMRENA_ERROR_NULL_POINTER; \
        return samrena_vector_set(vec->_internal, index, &value); \
    } \
    \
    /* Type-safe query functions */ \
    static inline size_t samrena_vector_##T##_size(const SamrenaVector_##T* vec) { \
        return vec && vec->_internal ? samrena_vector_size(vec->_internal) : 0; \
    } \
    \
    static inline size_t samrena_vector_##T##_capacity(const SamrenaVector_##T* vec) { \
        return vec && vec->_internal ? samrena_vector_capacity(vec->_internal) : 0; \
    } \
    \
    static inline bool samrena_vector_##T##_is_empty(const SamrenaVector_##T* vec) { \
        return !vec || !vec->_internal || samrena_vector_is_empty(vec->_internal); \
    }
```

### Extended Type-Safe Operations
```c
// Macro for additional type-safe operations
#define SAMRENA_VECTOR_DEFINE_TYPE_EXTENDED(T) \
    SAMRENA_VECTOR_DEFINE_TYPE(T) \
    \
    /* Type-safe bulk operations */ \
    static inline int samrena_vector_##T##_push_array(SamrenaVector_##T* vec, \
                                                      const T* elements, size_t count) { \
        if (!vec || !vec->_internal || (!elements && count > 0)) { \
            return SAMRENA_ERROR_NULL_POINTER; \
        } \
        return samrena_vector_push_array(vec->_internal, elements, count); \
    } \
    \
    static inline int samrena_vector_##T##_insert(SamrenaVector_##T* vec, \
                                                  size_t index, T value) { \
        if (!vec || !vec->_internal) return SAMRENA_ERROR_NULL_POINTER; \
        return samrena_vector_insert(vec->_internal, index, &value); \
    } \
    \
    static inline int samrena_vector_##T##_remove(SamrenaVector_##T* vec, size_t index) { \
        if (!vec || !vec->_internal) return SAMRENA_ERROR_NULL_POINTER; \
        return samrena_vector_remove(vec->_internal, index); \
    } \
    \
    /* Type-safe search operations */ \
    static inline size_t samrena_vector_##T##_find(const SamrenaVector_##T* vec, \
                                                   T target, \
                                                   int (*compare)(const T*, const T*)) { \
        if (!vec || !vec->_internal) return SAMRENA_NOT_FOUND; \
        \
        /* Wrapper to convert generic compare to typed compare */ \
        struct compare_wrapper { int (*typed_compare)(const T*, const T*); }; \
        struct compare_wrapper wrapper = {compare}; \
        \
        int generic_compare(const void* a, const void* b, void* user_data) { \
            struct compare_wrapper* w = (struct compare_wrapper*)user_data; \
            return w->typed_compare((const T*)a, (const T*)b); \
        } \
        \
        return compare ? \
            samrena_vector_find(vec->_internal, &target, generic_compare, &wrapper) : \
            samrena_vector_find_bytes(vec->_internal, &target); \
    } \
    \
    /* Type-safe iteration */ \
    static inline void samrena_vector_##T##_foreach(SamrenaVector_##T* vec, \
                                                    void (*callback)(T* element, size_t index, void* user_data), \
                                                    void* user_data) { \
        if (!vec || !vec->_internal || !callback) return; \
        \
        /* Wrapper to convert generic callback to typed callback */ \
        struct callback_wrapper { \
            void (*typed_callback)(T* element, size_t index, void* user_data); \
            void* user_data; \
        }; \
        struct callback_wrapper wrapper = {callback, user_data}; \
        \
        void generic_callback(void* element, size_t index, void* wrapper_data) { \
            struct callback_wrapper* w = (struct callback_wrapper*)wrapper_data; \
            w->typed_callback((T*)element, index, w->user_data); \
        } \
        \
        samrena_vector_foreach(vec->_internal, generic_callback, &wrapper); \
    }
```

### Predefined Type-Safe Vectors
```c
// Common type definitions - users can include this header
// samrena_vector_types.h

// Predefined vectors for common types
SAMRENA_VECTOR_DEFINE_TYPE_EXTENDED(int)
SAMRENA_VECTOR_DEFINE_TYPE_EXTENDED(float)  
SAMRENA_VECTOR_DEFINE_TYPE_EXTENDED(double)
SAMRENA_VECTOR_DEFINE_TYPE_EXTENDED(char)
SAMRENA_VECTOR_DEFINE_TYPE_EXTENDED(size_t)

// String vector (char pointer)
typedef char* string_t;
SAMRENA_VECTOR_DEFINE_TYPE_EXTENDED(string_t)

// Pointer vector (void pointer)
typedef void* ptr_t;
SAMRENA_VECTOR_DEFINE_TYPE_EXTENDED(ptr_t)

// Convenient typedefs
typedef SamrenaVector_int SamrenaVectorInt;
typedef SamrenaVector_float SamrenaVectorFloat;
typedef SamrenaVector_double SamrenaVectorDouble;
typedef SamrenaVector_char SamrenaVectorChar;
typedef SamrenaVector_string_t SamrenaVectorString;
typedef SamrenaVector_ptr_t SamrenaVectorPtr;
```

## Advanced Type Safety Features

### Compile-Time Type Checking
```c
// Macro to ensure type compatibility between vectors
#define SAMRENA_VECTOR_ASSERT_SAME_TYPE(vec1, vec2) \
    _Static_assert(_Generic((vec1), \
        SamrenaVector_int*: _Generic((vec2), SamrenaVector_int*: 1, default: 0), \
        SamrenaVector_float*: _Generic((vec2), SamrenaVector_float*: 1, default: 0), \
        SamrenaVector_double*: _Generic((vec2), SamrenaVector_double*: 1, default: 0), \
        default: 0), \
        "Vector types must match")

// Type-safe assignment between compatible vectors
#define SAMRENA_VECTOR_ASSIGN_SAFE(dest, src) \
    do { \
        SAMRENA_VECTOR_ASSERT_SAME_TYPE(dest, src); \
        (dest)->_internal = (src)->_internal; \
    } while(0)
```

### Generic Selection for Type-Specific Operations
```c
// Generic macro that dispatches to appropriate type-specific function
#define samrena_vector_push(vec, value) _Generic((vec), \
    SamrenaVector_int*: samrena_vector_int_push, \
    SamrenaVector_float*: samrena_vector_float_push, \
    SamrenaVector_double*: samrena_vector_double_push, \
    SamrenaVector_char*: samrena_vector_char_push, \
    default: samrena_vector_push_generic \
    )((vec), (value))

#define samrena_vector_at(vec, index) _Generic((vec), \
    SamrenaVector_int*: samrena_vector_int_at, \
    SamrenaVector_float*: samrena_vector_float_at, \
    SamrenaVector_double*: samrena_vector_double_at, \
    SamrenaVector_char*: samrena_vector_char_at, \
    default: samrena_vector_at_generic \
    )((vec), (index))

#define samrena_vector_size(vec) _Generic((vec), \
    SamrenaVector_int*: samrena_vector_int_size, \
    SamrenaVector_float*: samrena_vector_float_size, \
    SamrenaVector_double*: samrena_vector_double_size, \
    SamrenaVector_char*: samrena_vector_char_size, \
    default: samrena_vector_size_generic \
    )((vec))
```

### Type-Safe Initialization Helpers
```c
// Convenient initialization macros
#define SAMRENA_VECTOR_INIT(type, capacity) \
    samrena_vector_##type##_init_owned(capacity)

#define SAMRENA_VECTOR_INIT_WITH_ARENA(type, arena, capacity) \
    samrena_vector_##type##_init_with_arena(arena, capacity)

// Initialization with values
#define SAMRENA_VECTOR_FROM_ARRAY(type, array, count) \
    ({ \
        SamrenaVector_##type vec = samrena_vector_##type##_init_owned(count); \
        if (vec._internal) { \
            samrena_vector_##type##_push_array(&vec, array, count); \
        } \
        vec; \
    })

// Literal initialization (C99+ compound literals)
#define SAMRENA_VECTOR_LITERAL(type, ...) \
    SAMRENA_VECTOR_FROM_ARRAY(type, (type[]){__VA_ARGS__}, \
                              sizeof((type[]){__VA_ARGS__})/sizeof(type))
```

## Error Handling Integration

### Type-Safe Error Handling
```c
// Type-aware error context
#define SAMRENA_SET_TYPED_ERROR(type, code, message, vec) \
    do { \
        static char type_message[256]; \
        snprintf(type_message, sizeof(type_message), \
                "Type %s: %s", #type, message); \
        SAMRENA_SET_ERROR(code, type_message, vec); \
    } while(0)

// Enhanced validation for typed vectors
#define SAMRENA_VALIDATE_TYPED_VECTOR(type, vec) \
    do { \
        if (!(vec)) { \
            SAMRENA_SET_TYPED_ERROR(type, SAMRENA_ERROR_NULL_POINTER, \
                                   "Typed vector is NULL", vec); \
            return SAMRENA_ERROR_NULL_POINTER; \
        } \
        if (!(vec)->_internal) { \
            SAMRENA_SET_TYPED_ERROR(type, SAMRENA_ERROR_INVALID_STATE, \
                                   "Internal vector is NULL", vec); \
            return SAMRENA_ERROR_INVALID_STATE; \
        } \
        if ((vec)->_internal->element_size != sizeof(type)) { \
            SAMRENA_SET_TYPED_ERROR(type, SAMRENA_ERROR_INVALID_OPERATION, \
                                   "Element size mismatch", vec); \
            return SAMRENA_ERROR_INVALID_OPERATION; \
        } \
    } while(0)
```

## Usage Examples

### Basic Type-Safe Usage
```c
// Create type-safe int vector
SamrenaVectorInt int_vec = SAMRENA_VECTOR_INIT(int, 10);

// Type-safe operations - no casting needed
samrena_vector_int_push(&int_vec, 42);
samrena_vector_int_push(&int_vec, 24);

// Type-safe access
int* value_ptr = samrena_vector_int_at(&int_vec, 0);
if (value_ptr) {
    printf("First value: %d\n", *value_ptr);
}

// Or with error checking
int value;
if (samrena_vector_int_get(&int_vec, 1, &value) == SAMRENA_SUCCESS) {
    printf("Second value: %d\n", value);
}

// Type-safe iteration
void print_int(int* element, size_t index, void* user_data) {
    printf("[%zu] = %d\n", index, *element);
}

samrena_vector_int_foreach(&int_vec, print_int, NULL);

samrena_vector_int_destroy(&int_vec);
```

### Advanced Type-Safe Usage
```c
// Custom struct
typedef struct {
    int id;
    float value;
    char name[32];
} DataPoint;

// Define type-safe vector for custom struct
SAMRENA_VECTOR_DEFINE_TYPE_EXTENDED(DataPoint)

// Use with literals (C99+)
SamrenaVector_DataPoint data_vec = SAMRENA_VECTOR_LITERAL(DataPoint,
    {1, 3.14f, "point1"},
    {2, 2.71f, "point2"},
    {3, 1.41f, "point3"}
);

// Type-safe search with custom comparator
int compare_by_id(const DataPoint* a, const DataPoint* b) {
    return a->id - b->id;
}

DataPoint target = {2, 0, ""};
size_t index = samrena_vector_DataPoint_find(&data_vec, target, compare_by_id);

if (index != SAMRENA_NOT_FOUND) {
    DataPoint* found = samrena_vector_DataPoint_at(&data_vec, index);
    printf("Found: %s\n", found->name);
}
```

### Generic Dispatch Usage
```c
// Using generic macros (when available)
SamrenaVectorInt int_vec = SAMRENA_VECTOR_INIT(int, 10);
SamrenaVectorFloat float_vec = SAMRENA_VECTOR_INIT(float, 10);

// Generic calls dispatch to appropriate typed functions
samrena_vector_push(&int_vec, 42);      // -> samrena_vector_int_push
samrena_vector_push(&float_vec, 3.14f); // -> samrena_vector_float_push

printf("Int vector size: %zu\n", samrena_vector_size(&int_vec));
printf("Float vector size: %zu\n", samrena_vector_size(&float_vec));
```

## Design Benefits

### Compile-Time Safety
- Type mismatches caught at compile time
- No void pointer casting required
- Clear error messages for type issues

### Performance
- Zero runtime overhead over generic implementation
- Inline functions optimize to direct calls
- No additional memory usage

### Usability
- Natural C syntax for typed operations
- IDE autocompletion and type checking
- Self-documenting code

### Compatibility
- Existing generic code continues to work
- Gradual migration possible
- Interoperability between typed and generic vectors

## Testing Strategy
- Compile-time error detection tests
- Runtime behavior equivalence with generic API
- Performance comparison with generic implementation
- Integration tests with existing codebase
- Type safety validation across different compilers

## Integration Requirements
- Compatible with C99 and later standards
- Works with existing build system
- Optional header for predefined types
- Documentation for macro usage patterns