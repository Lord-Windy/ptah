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

#ifndef SAMVULKAN_DEVICE_H
#define SAMVULKAN_DEVICE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct SamVulkanInstance SamVulkanInstance;
typedef struct SamVulkanDevice SamVulkanDevice;

// Queue family types
typedef enum {
  SAMVULKAN_QUEUE_COMPUTE = 1 << 0,
  SAMVULKAN_QUEUE_GRAPHICS = 1 << 1,
  SAMVULKAN_QUEUE_TRANSFER = 1 << 2
} SamVulkanQueueType;

// Device configuration
typedef struct {
  void *physical_device; // VkPhysicalDevice
  SamVulkanQueueType required_queues;
  const char **required_extensions;
  size_t extension_count;
  const char **required_features;
  size_t feature_count;
} SamVulkanDeviceConfig;

// Create a logical device
SamVulkanDevice *samvulkan_device_create(SamVulkanInstance *instance,
                                         const SamVulkanDeviceConfig *config);

// Destroy a logical device
void samvulkan_device_destroy(SamVulkanDevice *device);

// Get the underlying VkDevice handle
void *samvulkan_device_get_handle(SamVulkanDevice *device);

// Get queue handle
void *samvulkan_device_get_queue(SamVulkanDevice *device, SamVulkanQueueType type);

// Get queue family index
uint32_t samvulkan_device_get_queue_family_index(SamVulkanDevice *device, SamVulkanQueueType type);

// Check if device supports compute shaders
bool samvulkan_device_supports_compute(SamVulkanDevice *device);

// Get device properties
void samvulkan_device_get_properties(SamVulkanDevice *device, char *name, size_t name_size,
                                     uint32_t *vendor_id);

// Get physical device handle
void *samvulkan_device_get_physical_device(SamVulkanDevice *device);

#ifdef __cplusplus
}
#endif

#endif // SAMVULKAN_DEVICE_H