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

#include "samvulkan.h"
#include <volk.h>
#include <stdio.h>

static bool g_initialized = false;

bool samvulkan_init(void) {
    if (g_initialized) {
        return true;
    }
    
    VkResult result = volkInitialize();
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to initialize Volk: %d\n", result);
        return false;
    }
    
    g_initialized = true;
    return true;
}

void samvulkan_cleanup(void) {
    g_initialized = false;
}

bool samvulkan_is_available(void) {
    if (!g_initialized) {
        if (!samvulkan_init()) {
            return false;
        }
    }
    
    uint32_t version = volkGetInstanceVersion();
    return version != 0;
}

void samvulkan_get_version(uint32_t* major, uint32_t* minor, uint32_t* patch) {
    if (!g_initialized) {
        if (!samvulkan_init()) {
            if (major) *major = 0;
            if (minor) *minor = 0;
            if (patch) *patch = 0;
            return;
        }
    }
    
    uint32_t version = volkGetInstanceVersion();
    if (major) *major = VK_VERSION_MAJOR(version);
    if (minor) *minor = VK_VERSION_MINOR(version);
    if (patch) *patch = VK_VERSION_PATCH(version);
}