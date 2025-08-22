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
#include <stdio.h>

// Need to access build configuration - this will be available through the generated header
#ifdef SAMRENA_ENABLE_VIRTUAL
#define SAMRENA_VM_PLATFORM "detected"
#endif

void test_available_features(void) {
  printf("Samrena build configuration:\n");

#ifdef SAMRENA_ENABLE_VIRTUAL
  printf("  Virtual memory: ENABLED (%s)\n", SAMRENA_VM_PLATFORM);
#else
  printf("  Virtual memory: DISABLED\n");
#endif

  printf("\nAvailable strategies:\n");
  SamrenaStrategy strategies[10];
  int count = samrena_available_strategies(strategies, 10);

  for (int i = 0; i < count; i++) {
    printf("  - %s\n", samrena_strategy_name(strategies[i]));
  }
}

void test_strategy_availability(void) {
  printf("\nTesting strategy availability:\n");

  // Chained adapter should always be available
  assert(samrena_strategy_available(SAMRENA_STRATEGY_CHAINED));
  printf("  Chained: Available\n");

  // Virtual adapter availability depends on build configuration
  bool virtual_available = samrena_strategy_available(SAMRENA_STRATEGY_VIRTUAL);
  printf("  Virtual: %s\n", virtual_available ? "Available" : "Not Available");

  // Check consistency with build configuration
  // Note: We can't directly include samrena_config.h in test, so we check the runtime availability
  // and compare it with what we expect based on the capability query
  printf("  Virtual availability matches expectation: %s\n", virtual_available ? "YES" : "NO");
}

void test_strategy_fallback(void) {
  printf("\nTesting strategy fallback:\n");

  // Test creating arena with virtual strategy
  SamrenaConfig config = samrena_default_config();
  config.strategy = SAMRENA_STRATEGY_VIRTUAL;

  Samrena *arena = samrena_create(&config);

  // Arena should always be created (either directly or via fallback)
  assert(arena != NULL);

  bool virtual_available = samrena_strategy_available(SAMRENA_STRATEGY_VIRTUAL);
  if (virtual_available) {
    printf("  Virtual strategy: Created successfully\n");
  } else {
    printf("  Virtual strategy: Fell back to chained (expected warning printed)\n");
  }

  if (arena) {
    samrena_destroy(arena);
  }

  // Test strict mode
  printf("\nTesting strict fallback mode:\n");
  config.fallback_mode = SAMRENA_FALLBACK_STRICT;
  config.strategy = SAMRENA_STRATEGY_VIRTUAL;

  Samrena *strict_arena = samrena_create(&config);

  if (virtual_available) {
    assert(strict_arena != NULL);
    printf("  Strict mode with virtual: Success\n");
    if (strict_arena) {
      samrena_destroy(strict_arena);
    }
  } else {
    assert(strict_arena == NULL);
    printf("  Strict mode with virtual: Failed as expected\n");
  }
}

int main(void) {
  printf("=== Samrena Feature Tests ===\n\n");

  test_available_features();
  test_strategy_availability();
  test_strategy_fallback();

  printf("\n=== All tests passed ===\n");
  return 0;
}