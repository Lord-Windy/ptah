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

#ifndef SAMRENA_INTERNAL_H
#define SAMRENA_INTERNAL_H

#include "samrena.h"
#include <stdio.h>

// Virtual memory context - internal implementation details
typedef struct {
  void *base_address;
  uint64_t reserved_size;
  uint64_t committed_size;
  uint64_t allocated_size;
  uint64_t commit_granularity;
  uint64_t page_size;
  bool enable_stats;
  bool enable_debug;
} VirtualContext;

// Implementation structure - private to samrena
struct SamrenaImpl {
  uint64_t page_size;
  SamrenaConfig config;

  // Statistics
  struct {
    uint64_t total_allocations;
    uint64_t failed_allocations;
    uint64_t peak_usage;
  } stats;
};

#endif // SAMRENA_INTERNAL_H