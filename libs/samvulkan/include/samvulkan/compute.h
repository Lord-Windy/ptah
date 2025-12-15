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

#ifndef SAMVULKAN_COMPUTE_H
#define SAMVULKAN_COMPUTE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct SamVulkanDevice SamVulkanDevice;
typedef struct SamVulkanCompute SamVulkanCompute;

// Buffer usage flags
typedef enum {
  SAMVULKAN_BUFFER_STORAGE = 1 << 0,
  SAMVULKAN_BUFFER_UNIFORM = 1 << 1,
  SAMVULKAN_BUFFER_TRANSFER_SRC = 1 << 2,
  SAMVULKAN_BUFFER_TRANSFER_DST = 1 << 3
} SamVulkanBufferUsage;

// Memory property flags
typedef enum {
  SAMVULKAN_MEMORY_DEVICE_LOCAL = 1 << 0,
  SAMVULKAN_MEMORY_HOST_VISIBLE = 1 << 1,
  SAMVULKAN_MEMORY_HOST_COHERENT = 1 << 2
} SamVulkanMemoryProperty;

// Compute pipeline configuration
typedef struct {
  const uint32_t *shader_code;
  size_t shader_size;
  const char *entry_point;
  uint32_t push_constant_size;
} SamVulkanComputeConfig;

// Buffer handle
typedef struct SamVulkanBuffer SamVulkanBuffer;

// Create compute context
SamVulkanCompute *samvulkan_compute_create(SamVulkanDevice *device);

// Destroy compute context
void samvulkan_compute_destroy(SamVulkanCompute *compute);

// Create compute pipeline from SPIR-V shader
bool samvulkan_compute_create_pipeline(SamVulkanCompute *compute,
                                       const SamVulkanComputeConfig *config);

// Create buffer
SamVulkanBuffer *samvulkan_compute_create_buffer(SamVulkanCompute *compute, size_t size,
                                                 SamVulkanBufferUsage usage,
                                                 SamVulkanMemoryProperty properties);

// Destroy buffer
void samvulkan_compute_destroy_buffer(SamVulkanBuffer *buffer);

// Map buffer memory for host access
void *samvulkan_compute_map_buffer(SamVulkanBuffer *buffer);

// Unmap buffer memory
void samvulkan_compute_unmap_buffer(SamVulkanBuffer *buffer);

// Copy data to buffer
bool samvulkan_compute_upload_buffer(SamVulkanBuffer *buffer, const void *data, size_t size);

// Copy data from buffer
bool samvulkan_compute_download_buffer(SamVulkanBuffer *buffer, void *data, size_t size);

// Bind buffer to descriptor set
void samvulkan_compute_bind_buffer(SamVulkanCompute *compute, uint32_t binding,
                                   SamVulkanBuffer *buffer);

// Set push constants
void samvulkan_compute_set_push_constants(SamVulkanCompute *compute, const void *data, size_t size);

// Dispatch compute work
bool samvulkan_compute_dispatch(SamVulkanCompute *compute, uint32_t x, uint32_t y, uint32_t z);

// Wait for compute to complete
bool samvulkan_compute_wait(SamVulkanCompute *compute);

// Submit and wait for compute in one call
bool samvulkan_compute_submit_and_wait(SamVulkanCompute *compute, uint32_t x, uint32_t y,
                                       uint32_t z);

#ifdef __cplusplus
}
#endif

#endif // SAMVULKAN_COMPUTE_H