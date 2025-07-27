#ifndef SAMRENA_H
#define SAMRENA_H

#include <stdint.h>
#include <stdlib.h>

#define NEAT_INITIAL_PAGE_SIZE 1024

typedef struct {
  uint8_t* bytes;
  uint64_t allocated;
  uint64_t capacity;
} Samrena;

typedef struct {
  uint64_t size;
  uint64_t element_size;
  uint64_t capacity;
  void*    data;
} SamrenaVector;


Samrena* samrena_allocate(uint64_t page_count);
void samrena_deallocate(Samrena* samrena);

void* samrena_push(Samrena* samrena, uint64_t size);
void* samrena_push_zero(Samrena* samrena, uint64_t size);

void* samrena_resize_array(
    Samrena* samrena, 
    void*    original_array, 
    uint64_t original_size,
    uint64_t new_size
);

uint64_t samrena_allocated(Samrena* samrena);
uint64_t samrena_capacity(Samrena* samrena);

SamrenaVector* samrena_vector_init(
  Samrena* samrena,
  uint64_t element_size,
  uint64_t capacity // 0 uses a default
);

void* samrena_vector_push(
  SamrenaVector* samrena_vector,
  Samrena*       samrena,
  void*          element
);

void* samrena_vector_pop(
  SamrenaVector* samrena_vector
);

void* samrena_vector_resize(
  SamrenaVector* samrena_vector,
  Samrena*       samrena,
  uint64_t       resize_factor
);

#endif
