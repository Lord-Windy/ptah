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

#ifndef SAMVULKAN_INSTANCE_H
#define SAMVULKAN_INSTANCE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct SamVulkanInstance SamVulkanInstance;

// Instance configuration
typedef struct {
    const char* application_name;
    uint32_t application_version;
    const char* engine_name;
    uint32_t engine_version;
    bool enable_validation;
    const char** required_extensions;
    size_t extension_count;
} SamVulkanInstanceConfig;

// Create a Vulkan instance
SamVulkanInstance* samvulkan_instance_create(const SamVulkanInstanceConfig* config);

// Destroy a Vulkan instance
void samvulkan_instance_destroy(SamVulkanInstance* instance);

// Get the underlying VkInstance handle
void* samvulkan_instance_get_handle(SamVulkanInstance* instance);

// Enumerate physical devices
size_t samvulkan_instance_get_physical_device_count(SamVulkanInstance* instance);

// Get physical device at index
void* samvulkan_instance_get_physical_device(SamVulkanInstance* instance, size_t index);

#ifdef __cplusplus
}
#endif

#endif // SAMVULKAN_INSTANCE_H