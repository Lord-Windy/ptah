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

#include <stdio.h>
#include <libpq-fe.h>
#include "samrena.h"
#include "samdata.h"

int main(int argc, char *argv[]) {
    printf("samtrader prototype\n");

    // Initialize arena
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);

    // Cleanup
    samrena_destroy(arena);

    return 0;
}
