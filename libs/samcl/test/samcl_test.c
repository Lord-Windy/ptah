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

#include "samcl.h"
#include <assert.h>
#include <stdio.h>

void test_create_destroy() {
    SamCL* cl = samcl_create();
    assert(cl != NULL);
    samcl_destroy(cl);
    printf("test_create_destroy: PASSED\n");
}

void test_init() {
    SamCL* cl = samcl_create();
    assert(cl != NULL);
    
    int result = samcl_init(cl);
    assert(result == 0);
    
    samcl_destroy(cl);
    printf("test_init: PASSED\n");
}

void test_execute() {
    SamCL* cl = samcl_create();
    assert(cl != NULL);
    
    int result = samcl_init(cl);
    assert(result == 0);
    
    result = samcl_execute(cl, "test command");
    assert(result == 0);
    
    samcl_destroy(cl);
    printf("test_execute: PASSED\n");
}

int main() {
    printf("Running SamCL tests...\n");
    
    test_create_destroy();
    test_init();
    test_execute();
    
    printf("All tests passed!\n");
    return 0;
}