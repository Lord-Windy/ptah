/*
 * Copyright 2025 Samuel "Lord-Windy" Brown
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <pthread.h>
#include <samrena.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct {
  const char *name;
  void (*test_func)(void);
} ArenaTest;

typedef struct {
  Samrena *arena;
  size_t thread_id;
  size_t allocation_count;
  volatile bool *success;
} ThreadTestData;

static void test_basic_allocation(void) {
  SamrenaConfig config = samrena_default_config();

  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  size_t sizes[] = {1, 8, 16, 32, 64, 128, 256, 512, 1024, 4096};
  void *ptrs[sizeof(sizes) / sizeof(sizes[0])];

  for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
    ptrs[i] = samrena_push(arena, sizes[i]);
    assert(ptrs[i] != NULL);

    memset(ptrs[i], 0xAA, sizes[i]);
  }

  for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
    for (size_t j = i + 1; j < sizeof(sizes) / sizeof(sizes[0]); j++) {
      assert(ptrs[i] != ptrs[j]);
      assert((char *)ptrs[i] + sizes[i] <= (char *)ptrs[j] ||
             (char *)ptrs[j] + sizes[j] <= (char *)ptrs[i]);
    }
  }

  samrena_destroy(arena);
}

static void test_large_allocation(void) {
  SamrenaConfig config = samrena_default_config();
  config.initial_pages = 1;

  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  size_t large_size = config.page_size == 0 ? 12288 : config.page_size * 3;
  void *large_ptr = samrena_push(arena, large_size);
  assert(large_ptr != NULL);

  memset(large_ptr, 0xBB, large_size);

  void *after = samrena_push(arena, 1024);
  assert(after != NULL);
  assert(after != large_ptr);

  samrena_destroy(arena);
}

static void test_many_small_allocations(void) {
  SamrenaConfig config = samrena_default_config();

  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  const size_t count = 10000;
  void **ptrs = malloc(count * sizeof(void *));

  for (size_t i = 0; i < count; i++) {
    size_t size = (i % 256) + 1;
    ptrs[i] = samrena_push(arena, size);
    assert(ptrs[i] != NULL);

    memset(ptrs[i], i & 0xFF, size);
  }

  for (size_t i = 0; i < count; i++) {
    size_t size = (i % 256) + 1;
    unsigned char *p = (unsigned char *)ptrs[i];
    for (size_t j = 0; j < size; j++) {
      assert(p[j] == (i & 0xFF));
    }
  }

  free(ptrs);
  samrena_destroy(arena);
}

static void test_growth_behavior(void) {
  SamrenaConfig config = samrena_default_config();
  config.initial_pages = 1;

  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  uint64_t initial_capacity = samrena_capacity(arena);

  size_t allocation_size = 1024;
  void *ptr = samrena_push(arena, allocation_size);
  assert(ptr != NULL);

  while (samrena_allocated(arena) < initial_capacity) {
    void *new_ptr = samrena_push(arena, allocation_size);
    assert(new_ptr != NULL);
  }

  void *growth_ptr = samrena_push(arena, allocation_size);
  assert(growth_ptr != NULL);

  uint64_t new_capacity = samrena_capacity(arena);
  assert(new_capacity > initial_capacity);

  samrena_destroy(arena);
}

static void test_reset_operation(void) {
  SamrenaConfig config = samrena_default_config();

  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  void *p1 = samrena_push(arena, 1024);
  assert(p1 != NULL);
  memset(p1, 0xAA, 1024);

  uint64_t allocated_before = samrena_allocated(arena);
  assert(allocated_before >= 1024);

  bool reset_ok = samrena_reset_if_supported(arena);
  assert(reset_ok);

  uint64_t allocated_after = samrena_allocated(arena);
  assert(allocated_after == 0);

  void *p2 = samrena_push(arena, 1024);
  assert(p2 != NULL);

  // Virtual memory should reuse the same address
  assert(p2 == p1);

  samrena_destroy(arena);
}

static void test_reserve_operation(void) {
  SamrenaConfig config = samrena_default_config();

  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  uint64_t reserve_size = 1024 * 1024;
  SamrenaError result = samrena_reserve(arena, reserve_size);
  assert(result == SAMRENA_SUCCESS);

  uint64_t capacity_after = samrena_capacity(arena);
  assert(capacity_after >= reserve_size);

  samrena_destroy(arena);
}

static void test_edge_cases(void) {
  SamrenaConfig config = samrena_default_config();

  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  void *zero_ptr = samrena_push(arena, 0);
  assert(zero_ptr == NULL);

  void *one_ptr = samrena_push(arena, 1);
  assert(one_ptr != NULL);

  void *small_ptr = samrena_push(arena, 8);
  assert(small_ptr != NULL);
  assert(small_ptr != one_ptr);

  samrena_destroy(arena);
}

static void *thread_allocator(void *arg) {
  ThreadTestData *data = (ThreadTestData *)arg;

  for (size_t i = 0; i < data->allocation_count; i++) {
    size_t size = (data->thread_id * 100 + i) % 1024 + 1;
    void *ptr = samrena_push(data->arena, size);
    if (ptr == NULL) {
      *(data->success) = false;
      return NULL;
    }

    memset(ptr, (data->thread_id + i) & 0xFF, size);
  }

  return NULL;
}

static void test_thread_safety(void) {
  const size_t thread_count = 4;
  const size_t allocs_per_thread = 1000;

  pthread_t threads[thread_count];
  ThreadTestData thread_data[thread_count];
  volatile bool success_flags[thread_count];

  for (size_t i = 0; i < thread_count; i++) {
    SamrenaConfig config = samrena_default_config();

    thread_data[i].arena = samrena_create(&config);
    thread_data[i].thread_id = i;
    thread_data[i].allocation_count = allocs_per_thread;
    success_flags[i] = true;
    thread_data[i].success = &success_flags[i];

    pthread_create(&threads[i], NULL, thread_allocator, &thread_data[i]);
  }

  for (size_t i = 0; i < thread_count; i++) {
    pthread_join(threads[i], NULL);
    assert(success_flags[i]);
  }

  for (size_t i = 0; i < thread_count; i++) {
    samrena_destroy(thread_data[i].arena);
  }
}

static void test_memory_leaks(void) {
  for (int i = 0; i < 100; i++) {
    SamrenaConfig config = samrena_default_config();

    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);

    for (int j = 0; j < 100; j++) {
      size_t size = (rand() % 4096) + 1;
      void *ptr = samrena_push(arena, size);
      assert(ptr != NULL);
    }

    samrena_destroy(arena);
  }
}

static void test_aligned_allocation(void) {
  SamrenaConfig config = samrena_default_config();

  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  size_t alignments[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
  for (size_t i = 0; i < sizeof(alignments) / sizeof(alignments[0]); i++) {
    void *ptr = samrena_push_aligned(arena, 100, alignments[i]);
    assert(ptr != NULL);
    assert(((uintptr_t)ptr % alignments[i]) == 0);
    memset(ptr, 0xCC, 100);
  }

  samrena_destroy(arena);
}

static void test_zero_allocation(void) {
  SamrenaConfig config = samrena_default_config();

  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  void *ptr = samrena_push_zero(arena, 1024);
  assert(ptr != NULL);

  unsigned char *bytes = (unsigned char *)ptr;
  for (size_t i = 0; i < 1024; i++) {
    assert(bytes[i] == 0);
  }

  samrena_destroy(arena);
}

static void test_capabilities(void) {
  SamrenaConfig config = samrena_default_config();

  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  SamrenaCapabilities caps = samrena_get_capabilities(arena);

  // Virtual memory should have these capabilities
  assert(caps.flags & SAMRENA_CAP_CONTIGUOUS_MEMORY);
  assert(caps.flags & SAMRENA_CAP_ZERO_COPY_GROWTH);
  assert(caps.flags & SAMRENA_CAP_RESET);
  assert(caps.flags & SAMRENA_CAP_RESERVE);

  assert(samrena_has_capability(arena, SAMRENA_CAP_CONTIGUOUS_MEMORY));
  assert(samrena_has_capability(arena, SAMRENA_CAP_RESET));

  samrena_destroy(arena);
}

static void run_tests(void) {
  ArenaTest tests[] = {{"basic_allocation", test_basic_allocation},
                       {"large_allocation", test_large_allocation},
                       {"many_small_allocations", test_many_small_allocations},
                       {"growth_behavior", test_growth_behavior},
                       {"reset_operation", test_reset_operation},
                       {"reserve_operation", test_reserve_operation},
                       {"edge_cases", test_edge_cases},
                       {"thread_safety", test_thread_safety},
                       {"memory_leaks", test_memory_leaks},
                       {"aligned_allocation", test_aligned_allocation},
                       {"zero_allocation", test_zero_allocation},
                       {"capabilities", test_capabilities},
                       {NULL, NULL}};

  printf("=== Samrena Arena Test Suite ===\n\n");

  for (ArenaTest *test = tests; test->name; test++) {
    printf("  %s: ", test->name);
    fflush(stdout);

    test->test_func();
    printf("PASS\n");
  }

  printf("\nAll tests completed successfully!\n");
}

int main(void) {
  srand((unsigned int)time(NULL));
  run_tests();
  return 0;
}
