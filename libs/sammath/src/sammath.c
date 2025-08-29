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

#include "sammath.h"
#include <stdio.h>
#include <string.h>

SamMath *sammath_create(Samrena *arena) {
    if (!arena) return NULL;
    
    SamMath *math = (SamMath *)samrena_push(arena, sizeof(SamMath));
    if (!math) return NULL;
    
    math->arena = arena;
    math->constants = samhashmap_create(16, arena);
    
    if (math->constants) {
        double *pi = (double *)samrena_push(arena, sizeof(double));
        *pi = 3.14159265358979323846;
        samhashmap_put(math->constants, "pi", pi);
        
        double *e = (double *)samrena_push(arena, sizeof(double));
        *e = 2.71828182845904523536;
        samhashmap_put(math->constants, "e", e);
    }
    
    return math;
}

void sammath_destroy(SamMath *math) {
    if (!math) return;
}

void sammath_hello(void) {
    printf("Hello from SamMath library!\n");
    printf("SamMath now includes samrena (memory arena) and samdata (hash maps) support!\n");
}