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

#ifndef HONEYCOMB_TESTS_H
#define HONEYCOMB_TESTS_H

#include "datazoo.h"

void honeycomb_tests();
void typed_honeycomb_tests();

// Comprehensive test suites (Steps 16-17 from planning)
void comprehensive_edge_case_tests();
void collision_handling_tests();
void resize_behavior_tests();
void memory_exhaustion_tests();
void performance_benchmark_tests();
void debugging_support_tests();

#endif // HONEYCOMB_TESTS_H
