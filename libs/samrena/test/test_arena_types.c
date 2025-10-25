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

#include "samrena.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Helper to get reserved size from arena (accessing internal state)
static uint64_t get_reserved_size(Samrena *arena) {
  SamrenaInfo info;
  samrena_get_info(arena, &info);
  // We need to check via capabilities since reserved size isn't in SamrenaInfo
  SamrenaCapabilities caps = samrena_get_capabilities(arena);
  // max_allocation_size = reserved - allocated
  // So at the start, max_allocation_size ≈ reserved size
  return caps.max_allocation_size + samrena_allocated(arena);
}

void test_default_arena() {
  printf("Testing default arena (256MB)...\n");

  Samrena *arena = samrena_create_default();
  assert(arena != NULL);

  // Check reserved size is 256MB (allowing for alignment)
  uint64_t reserved = get_reserved_size(arena);
  uint64_t expected = 256ULL * 1024 * 1024;
  printf("  Reserved: %lu bytes (expected ~%lu)\n", reserved, expected);
  assert(reserved >= expected);
  assert(reserved < expected + (1024 * 1024)); // Within 1MB of expected

  // Test basic allocation
  int *data = SAMRENA_PUSH_ARRAY(arena, int, 1000);
  assert(data != NULL);
  for (int i = 0; i < 1000; i++) {
    data[i] = i;
  }

  // Verify data
  for (int i = 0; i < 1000; i++) {
    assert(data[i] == i);
  }

  printf("  Basic allocation: OK\n");

  // Test larger allocation (10MB)
  void *large = samrena_push(arena, 10 * 1024 * 1024);
  assert(large != NULL);
  printf("  Large allocation (10MB): OK\n");

  samrena_destroy(arena);
  printf("  PASSED\n\n");
}

void test_session_arena() {
  printf("Testing session arena (256GB)...\n");

  Samrena *arena = samrena_create_session();
  assert(arena != NULL);

  // Check reserved size is 256GB (allowing for alignment)
  uint64_t reserved = get_reserved_size(arena);
  uint64_t expected = 256ULL * 1024 * 1024 * 1024;
  printf("  Reserved: %lu bytes (expected ~%lu)\n", reserved, expected);
  assert(reserved >= expected);
  assert(reserved < expected + (64 * 1024 * 1024)); // Within 64MB of expected

  // Test basic allocation
  int *data = SAMRENA_PUSH_ARRAY(arena, int, 1000);
  assert(data != NULL);
  for (int i = 0; i < 1000; i++) {
    data[i] = i * 2;
  }

  // Verify data
  for (int i = 0; i < 1000; i++) {
    assert(data[i] == i * 2);
  }

  printf("  Basic allocation: OK\n");

  // Test larger allocation (1GB)
  void *large = samrena_push(arena, 1ULL * 1024 * 1024 * 1024);
  assert(large != NULL);
  printf("  Large allocation (1GB): OK\n");

  // Test can allocate after large allocation
  int *more_data = SAMRENA_PUSH_ARRAY(arena, int, 100);
  assert(more_data != NULL);
  more_data[0] = 42;
  assert(more_data[0] == 42);
  printf("  Post-large allocation: OK\n");

  samrena_destroy(arena);
  printf("  PASSED\n\n");
}

void test_global_arena() {
  printf("Testing global arena (4TB)...\n");

  Samrena *arena = samrena_create_global();
  assert(arena != NULL);

  // Check reserved size is 4TB (allowing for alignment)
  uint64_t reserved = get_reserved_size(arena);
  uint64_t expected = 4ULL * 1024 * 1024 * 1024 * 1024;
  printf("  Reserved: %lu bytes (expected ~%lu)\n", reserved, expected);
  assert(reserved >= expected);
  assert(reserved < expected + (64 * 1024 * 1024)); // Within 64MB of expected

  // Test basic allocation
  int *data = SAMRENA_PUSH_ARRAY(arena, int, 1000);
  assert(data != NULL);
  for (int i = 0; i < 1000; i++) {
    data[i] = i * 3;
  }

  // Verify data
  for (int i = 0; i < 1000; i++) {
    assert(data[i] == i * 3);
  }

  printf("  Basic allocation: OK\n");

  // Test very large allocation (10GB)
  void *large = samrena_push(arena, 10ULL * 1024 * 1024 * 1024);
  assert(large != NULL);
  printf("  Very large allocation (10GB): OK\n");

  // Test can allocate after very large allocation
  int *more_data = SAMRENA_PUSH_ARRAY(arena, int, 100);
  assert(more_data != NULL);
  more_data[99] = 123;
  assert(more_data[99] == 123);
  printf("  Post-large allocation: OK\n");

  samrena_destroy(arena);
  printf("  PASSED\n\n");
}

void test_arena_isolation() {
  printf("Testing arena isolation...\n");

  Samrena *default_arena = samrena_create_default();
  Samrena *session_arena = samrena_create_session();
  Samrena *global_arena = samrena_create_global();

  assert(default_arena != NULL);
  assert(session_arena != NULL);
  assert(global_arena != NULL);

  // Allocate from each arena
  int *default_data = SAMRENA_PUSH_TYPE(default_arena, int);
  int *session_data = SAMRENA_PUSH_TYPE(session_arena, int);
  int *global_data = SAMRENA_PUSH_TYPE(global_arena, int);

  assert(default_data != NULL);
  assert(session_data != NULL);
  assert(global_data != NULL);

  // Ensure pointers are different
  assert(default_data != session_data);
  assert(default_data != global_data);
  assert(session_data != global_data);

  // Write unique values
  *default_data = 1;
  *session_data = 2;
  *global_data = 3;

  // Verify isolation
  assert(*default_data == 1);
  assert(*session_data == 2);
  assert(*global_data == 3);

  printf("  Arena isolation: OK\n");

  samrena_destroy(default_arena);
  samrena_destroy(session_arena);
  samrena_destroy(global_arena);

  printf("  PASSED\n\n");
}

void test_allocation_limits() {
  printf("Testing allocation limits...\n");

  // Create a small default arena
  Samrena *arena = samrena_create_default();
  assert(arena != NULL);

  // Try to allocate more than 256MB - should fail
  void *too_large = samrena_push(arena, 300ULL * 1024 * 1024);
  assert(too_large == NULL);
  assert(samrena_get_last_error() == SAMRENA_ERROR_OUT_OF_MEMORY);
  printf("  Over-allocation correctly rejected: OK\n");

  // Can still allocate within limits
  void *valid = samrena_push(arena, 100 * 1024 * 1024);
  assert(valid != NULL);
  printf("  Valid allocation after failure: OK\n");

  samrena_destroy(arena);
  printf("  PASSED\n\n");
}

void test_capabilities() {
  printf("Testing arena capabilities...\n");

  Samrena *arena = samrena_create_default();
  assert(arena != NULL);

  SamrenaCapabilities caps = samrena_get_capabilities(arena);

  // All arena types should have the same capabilities
  assert(caps.flags & SAMRENA_CAP_CONTIGUOUS_MEMORY);
  assert(caps.flags & SAMRENA_CAP_ZERO_COPY_GROWTH);
  assert(caps.flags & SAMRENA_CAP_RESET);
  assert(caps.flags & SAMRENA_CAP_RESERVE);

  printf("  All capabilities present: OK\n");

  // Test has_capability function
  assert(samrena_has_capability(arena, SAMRENA_CAP_CONTIGUOUS_MEMORY));
  assert(samrena_has_capability(arena, SAMRENA_CAP_ZERO_COPY_GROWTH));
  assert(samrena_has_capability(arena, SAMRENA_CAP_RESET));
  assert(samrena_has_capability(arena, SAMRENA_CAP_RESERVE));

  printf("  Capability queries: OK\n");

  samrena_destroy(arena);
  printf("  PASSED\n\n");
}

void test_info_structure() {
  printf("Testing arena info...\n");

  Samrena *arena = samrena_create_default();
  assert(arena != NULL);

  SamrenaInfo info;
  samrena_get_info(arena, &info);

  assert(info.allocated == 0);  // Nothing allocated yet
  assert(info.capacity > 0);    // Some memory committed
  assert(info.page_size > 0);   // Page size set
  assert(info.is_contiguous);   // Should be contiguous

  printf("  Initial state: OK\n");

  // Allocate some memory
  void *data = samrena_push(arena, 1024);
  assert(data != NULL);

  samrena_get_info(arena, &info);
  assert(info.allocated >= 1024);  // At least 1024 bytes (may be aligned)
  assert(info.capacity >= info.allocated);  // Capacity >= allocated

  printf("  After allocation: OK\n");

  samrena_destroy(arena);
  printf("  PASSED\n\n");
}

int main(void) {
  printf("=== Samrena Arena Type Tests ===\n\n");

  test_default_arena();
  test_session_arena();
  test_global_arena();
  test_arena_isolation();
  test_allocation_limits();
  test_capabilities();
  test_info_structure();

  printf("=== All Tests Passed! ===\n");
  return 0;
}
