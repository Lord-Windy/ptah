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

#ifndef DATAZOO_HASH_H
#define DATAZOO_HASH_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DATAZOO_HASH_DJB2,
    DATAZOO_HASH_FNV1A,
    DATAZOO_HASH_MURMUR3
} DatazooHashFunction;

uint32_t datazoo_hash_djb2(const void *data, size_t size);

uint32_t datazoo_hash_fnv1a(const void *data, size_t size);

uint32_t datazoo_hash_murmur3(const void *data, size_t size);

uint32_t datazoo_hash_string_djb2(const char *str);

uint32_t datazoo_hash_string_fnv1a(const char *str);

uint32_t datazoo_hash_string_murmur3(const char *str);

uint32_t datazoo_hash(const void *data, size_t size, DatazooHashFunction func);

uint32_t datazoo_hash_string(const char *str, DatazooHashFunction func);

#ifdef __cplusplus
}
#endif

#endif /* DATAZOO_HASH_H */