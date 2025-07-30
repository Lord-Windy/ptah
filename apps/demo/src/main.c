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
#include <stdio.h>
#include <string.h>

#include "datazoo.h"
#include "samrena.h"

int main() {
  printf("=== Ptah Demo Application ===\n");
  printf("Demonstrating samrena (memory arena) and datazoo (hashmap) libraries\n\n");

  // Create a memory arena
  printf("1. Creating memory arena with 2 pages...\n");
  Samrena *arena = samrena_create_default();
  printf("   Arena capacity: %lu bytes\n", samrena_capacity(arena));
  printf("   Arena allocated: %lu bytes\n\n", samrena_allocated(arena));

  // Create a hashmap using the arena
  printf("2. Creating hashmap using the arena...\n");
  Honeycomb *map = honeycomb_create(16, arena);
  printf("   Hashmap created with initial capacity: 16\n");
  printf("   Arena allocated after hashmap creation: %lu bytes\n\n", samrena_allocated(arena));

  // Add some key-value pairs
  printf("3. Adding programming languages and their year of creation...\n");

  // Store years as integers in the arena
  int *year_c = samrena_push(arena, sizeof(int));
  *year_c = 1972;
  honeycomb_put(map, "C", year_c);

  int *year_python = samrena_push(arena, sizeof(int));
  *year_python = 1991;
  honeycomb_put(map, "Python", year_python);

  int *year_rust = samrena_push(arena, sizeof(int));
  *year_rust = 2010;
  honeycomb_put(map, "Rust", year_rust);

  int *year_go = samrena_push(arena, sizeof(int));
  *year_go = 2009;
  honeycomb_put(map, "Go", year_go);

  printf("   Added 4 programming languages\n");
  printf("   Hashmap size: %zu\n", honeycomb_size(map));
  printf("   Arena allocated after adding data: %lu bytes\n\n", samrena_allocated(arena));

  // Retrieve and display values
  printf("4. Retrieving values from hashmap...\n");

  const char *languages[] = {"C", "Python", "Rust", "Go", "JavaScript"};
  size_t num_languages = sizeof(languages) / sizeof(languages[0]);

  for (size_t i = 0; i < num_languages; i++) {
    int *year = (int *)honeycomb_get(map, languages[i]);
    if (year) {
      printf("   %s: created in %d\n", languages[i], *year);
    } else {
      printf("   %s: not found in database\n", languages[i]);
    }
  }
  printf("\n");

  // Test contains functionality
  printf("5. Testing contains functionality...\n");
  printf("   Contains 'C': %s\n", honeycomb_contains(map, "C") ? "yes" : "no");
  printf("   Contains 'JavaScript': %s\n", honeycomb_contains(map, "JavaScript") ? "yes" : "no");
  printf("\n");

  // Use samrena vector to store a list of language names
  printf("6. Creating a vector to store language names...\n");
  SamrenaVector *lang_vector = samrena_vector_init(arena, sizeof(char *), 4);

  // Add language names to vector
  for (size_t i = 0; i < num_languages - 1; i++) { // Skip JavaScript since it's not in our map
    if (honeycomb_contains(map, languages[i])) {
      // Store the string pointer in arena
      char **name_ptr = samrena_push(arena, sizeof(char *));
      *name_ptr = (char *)languages[i];
      samrena_vector_push(arena, lang_vector, name_ptr);
    }
  }

  printf("   Vector size: %lu\n", lang_vector->size);
  printf("   Languages in vector:\n");
  for (uint64_t i = 0; i < lang_vector->size; i++) {
    char **name_ptr = (char **)lang_vector->data + i;
    printf("     - %s\n", *name_ptr);
  }
  printf("\n");

  // Show final memory usage
  printf("7. Final memory statistics...\n");
  printf("   Arena capacity: %lu bytes\n", samrena_capacity(arena));
  printf("   Arena allocated: %lu bytes\n", samrena_allocated(arena));
  printf("   Memory efficiency: %.1f%%\n",
         (double)samrena_allocated(arena) / samrena_capacity(arena) * 100);
  printf("\n");

  // Cleanup
  honeycomb_destroy(map);
  samrena_destroy(arena);

  printf("Demo completed successfully!\n");
  printf("This demonstrates:\n");
  printf("- Memory arena allocation and management (samrena)\n");
  printf("- Hashmap operations with arena-backed memory (datazoo)\n");
  printf("- Dynamic vectors with arena allocation (samrena)\n");
  printf("- Integration between both libraries in a single application\n");

  return 0;
}