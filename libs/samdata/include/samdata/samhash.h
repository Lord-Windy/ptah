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

#ifndef SAMDATA_SAMHASH_H
#define SAMDATA_SAMHASH_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SAMHASH_DJB2,
    SAMHASH_FNV1A,
    SAMHASH_MURMUR3
} SamHashFunction;

uint32_t samhash_djb2(const void *data, size_t size);

uint32_t samhash_fnv1a(const void *data, size_t size);

uint32_t samhash_murmur3(const void *data, size_t size);

uint32_t samhash_string_djb2(const char *str);

uint32_t samhash_string_fnv1a(const char *str);

uint32_t samhash_string_murmur3(const char *str);

uint32_t samhash(const void *data, size_t size, SamHashFunction func);

uint32_t samhash_string(const char *str, SamHashFunction func);

#ifdef __cplusplus
}
#endif

#endif /* SAMDATA_SAMHASH_H */