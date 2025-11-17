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

#ifndef SAMVULKAN_H
#define SAMVULKAN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct SamVulkanInstance SamVulkanInstance;
typedef struct SamVulkanDevice SamVulkanDevice;
typedef struct SamVulkanCompute SamVulkanCompute;

// Initialize Volk library
bool samvulkan_init(void);

// Cleanup Volk library
void samvulkan_cleanup(void);

// Check if Vulkan is available
bool samvulkan_is_available(void);

// Get Vulkan version
void samvulkan_get_version(uint32_t *major, uint32_t *minor, uint32_t *patch);

// Include sub-modules
#include "samvulkan/compute.h"
#include "samvulkan/device.h"
#include "samvulkan/instance.h"

#ifdef __cplusplus
}
#endif

#endif // SAMVULKAN_H