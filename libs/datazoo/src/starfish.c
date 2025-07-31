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

#include <datazoo/starfish.h>
#include <string.h>

uint32_t starfish_hash_djb2(const void *data, size_t size) {
    const unsigned char *bytes = (const unsigned char *)data;
    uint32_t hash = 5381;
    
    for (size_t i = 0; i < size; i++) {
        hash = ((hash << 5) + hash) + bytes[i];
    }
    
    return hash;
}

uint32_t starfish_hash_fnv1a(const void *data, size_t size) {
    const unsigned char *bytes = (const unsigned char *)data;
    uint32_t hash = 2166136261u;
    
    for (size_t i = 0; i < size; i++) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    
    return hash;
}

uint32_t starfish_hash_murmur3(const void *data, size_t size) {
    const unsigned char *bytes = (const unsigned char *)data;
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    const uint32_t r1 = 15;
    const uint32_t r2 = 13;
    const uint32_t m = 5;
    const uint32_t n = 0xe6546b64;
    
    uint32_t hash = 0;
    
    const int nblocks = size / 4;
    const uint32_t *blocks = (const uint32_t *)bytes;
    int i;
    
    for (i = 0; i < nblocks; i++) {
        uint32_t k = blocks[i];
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;
        
        hash ^= k;
        hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;
    }
    
    const unsigned char *tail = (const unsigned char *)(bytes + nblocks * 4);
    uint32_t k1 = 0;
    
    switch (size & 3) {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
                k1 *= c1;
                k1 = (k1 << r1) | (k1 >> (32 - r1));
                k1 *= c2;
                hash ^= k1;
    }
    
    hash ^= size;
    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);
    
    return hash;
}

uint32_t starfish_hash_string_djb2(const char *str) {
    uint32_t hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    
    return hash;
}

uint32_t starfish_hash_string_fnv1a(const char *str) {
    uint32_t hash = 2166136261u;
    
    while (*str) {
        hash ^= (unsigned char)*str++;
        hash *= 16777619u;
    }
    
    return hash;
}

uint32_t starfish_hash_string_murmur3(const char *str) {
    return starfish_hash_murmur3(str, strlen(str));
}

uint32_t starfish_hash(const void *data, size_t size, StarfishHashFunction func) {
    switch (func) {
        case STARFISH_HASH_DJB2:
            return starfish_hash_djb2(data, size);
        case STARFISH_HASH_FNV1A:
            return starfish_hash_fnv1a(data, size);
        case STARFISH_HASH_MURMUR3:
            return starfish_hash_murmur3(data, size);
        default:
            return starfish_hash_djb2(data, size);
    }
}

uint32_t starfish_hash_string(const char *str, StarfishHashFunction func) {
    switch (func) {
        case STARFISH_HASH_DJB2:
            return starfish_hash_string_djb2(str);
        case STARFISH_HASH_FNV1A:
            return starfish_hash_string_fnv1a(str);
        case STARFISH_HASH_MURMUR3:
            return starfish_hash_string_murmur3(str);
        default:
            return starfish_hash_string_djb2(str);
    }
}