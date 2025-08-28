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

#include "samvulkan/compute.h"
#include "samvulkan/device.h"
#include <volk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct SamVulkanCompute {
    SamVulkanDevice* device;
    VkDevice vk_device;
    VkPhysicalDevice physical_device;
    VkQueue compute_queue;
    uint32_t queue_family_index;
    
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
    
    VkDescriptorSetLayout descriptor_set_layout;
    VkPipelineLayout pipeline_layout;
    VkPipeline compute_pipeline;
    
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet descriptor_set;
    
    VkFence fence;
    
    uint32_t push_constant_size;
};

struct SamVulkanBuffer {
    SamVulkanCompute* compute;
    VkBuffer buffer;
    VkDeviceMemory memory;
    size_t size;
    SamVulkanBufferUsage usage;
    SamVulkanMemoryProperty properties;
    void* mapped_data;
};

static uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);
    
    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    return UINT32_MAX;
}

SamVulkanCompute* samvulkan_compute_create(SamVulkanDevice* device) {
    if (!device || !samvulkan_device_supports_compute(device)) {
        return NULL;
    }
    
    SamVulkanCompute* compute = calloc(1, sizeof(SamVulkanCompute));
    if (!compute) {
        return NULL;
    }
    
    compute->device = device;
    compute->vk_device = (VkDevice)samvulkan_device_get_handle(device);
    compute->physical_device = (VkPhysicalDevice)samvulkan_device_get_physical_device(device);
    compute->compute_queue = (VkQueue)samvulkan_device_get_queue(device, SAMVULKAN_QUEUE_COMPUTE);
    compute->queue_family_index = samvulkan_device_get_queue_family_index(device, SAMVULKAN_QUEUE_COMPUTE);
    
    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = compute->queue_family_index
    };
    
    if (vkCreateCommandPool(compute->vk_device, &pool_info, NULL, &compute->command_pool) != VK_SUCCESS) {
        free(compute);
        return NULL;
    }
    
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = compute->command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    
    if (vkAllocateCommandBuffers(compute->vk_device, &alloc_info, &compute->command_buffer) != VK_SUCCESS) {
        vkDestroyCommandPool(compute->vk_device, compute->command_pool, NULL);
        free(compute);
        return NULL;
    }
    
    VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    
    if (vkCreateFence(compute->vk_device, &fence_info, NULL, &compute->fence) != VK_SUCCESS) {
        vkDestroyCommandPool(compute->vk_device, compute->command_pool, NULL);
        free(compute);
        return NULL;
    }
    
    return compute;
}

void samvulkan_compute_destroy(SamVulkanCompute* compute) {
    if (!compute) return;
    
    vkDeviceWaitIdle(compute->vk_device);
    
    if (compute->compute_pipeline) {
        vkDestroyPipeline(compute->vk_device, compute->compute_pipeline, NULL);
    }
    if (compute->pipeline_layout) {
        vkDestroyPipelineLayout(compute->vk_device, compute->pipeline_layout, NULL);
    }
    if (compute->descriptor_set_layout) {
        vkDestroyDescriptorSetLayout(compute->vk_device, compute->descriptor_set_layout, NULL);
    }
    if (compute->descriptor_pool) {
        vkDestroyDescriptorPool(compute->vk_device, compute->descriptor_pool, NULL);
    }
    if (compute->fence) {
        vkDestroyFence(compute->vk_device, compute->fence, NULL);
    }
    if (compute->command_pool) {
        vkDestroyCommandPool(compute->vk_device, compute->command_pool, NULL);
    }
    
    free(compute);
}

bool samvulkan_compute_create_pipeline(SamVulkanCompute* compute, const SamVulkanComputeConfig* config) {
    if (!compute || !config || !config->shader_code) {
        return false;
    }
    
    VkShaderModuleCreateInfo shader_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = config->shader_size,
        .pCode = config->shader_code
    };
    
    VkShaderModule shader_module;
    if (vkCreateShaderModule(compute->vk_device, &shader_info, NULL, &shader_module) != VK_SUCCESS) {
        return false;
    }
    
    VkDescriptorSetLayoutBinding bindings[8] = {0};
    for (int i = 0; i < 8; i++) {
        bindings[i].binding = i;
        bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        bindings[i].descriptorCount = 1;
        bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    }
    
    VkDescriptorSetLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 8,
        .pBindings = bindings
    };
    
    if (vkCreateDescriptorSetLayout(compute->vk_device, &layout_info, NULL, &compute->descriptor_set_layout) != VK_SUCCESS) {
        vkDestroyShaderModule(compute->vk_device, shader_module, NULL);
        return false;
    }
    
    VkPushConstantRange push_constant = {
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0,
        .size = config->push_constant_size
    };
    
    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &compute->descriptor_set_layout
    };
    
    if (config->push_constant_size > 0) {
        pipeline_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_info.pPushConstantRanges = &push_constant;
        compute->push_constant_size = config->push_constant_size;
    }
    
    if (vkCreatePipelineLayout(compute->vk_device, &pipeline_layout_info, NULL, &compute->pipeline_layout) != VK_SUCCESS) {
        vkDestroyDescriptorSetLayout(compute->vk_device, compute->descriptor_set_layout, NULL);
        vkDestroyShaderModule(compute->vk_device, shader_module, NULL);
        return false;
    }
    
    VkPipelineShaderStageCreateInfo shader_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = shader_module,
        .pName = config->entry_point ? config->entry_point : "main"
    };
    
    VkComputePipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage = shader_stage_info,
        .layout = compute->pipeline_layout
    };
    
    VkResult result = vkCreateComputePipelines(compute->vk_device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &compute->compute_pipeline);
    
    vkDestroyShaderModule(compute->vk_device, shader_module, NULL);
    
    if (result != VK_SUCCESS) {
        vkDestroyPipelineLayout(compute->vk_device, compute->pipeline_layout, NULL);
        vkDestroyDescriptorSetLayout(compute->vk_device, compute->descriptor_set_layout, NULL);
        return false;
    }
    
    VkDescriptorPoolSize pool_size = {
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 8
    };
    
    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size,
        .maxSets = 1
    };
    
    if (vkCreateDescriptorPool(compute->vk_device, &pool_info, NULL, &compute->descriptor_pool) != VK_SUCCESS) {
        return false;
    }
    
    VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = compute->descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &compute->descriptor_set_layout
    };
    
    if (vkAllocateDescriptorSets(compute->vk_device, &alloc_info, &compute->descriptor_set) != VK_SUCCESS) {
        return false;
    }
    
    return true;
}

SamVulkanBuffer* samvulkan_compute_create_buffer(
    SamVulkanCompute* compute,
    size_t size,
    SamVulkanBufferUsage usage,
    SamVulkanMemoryProperty properties) {
    
    if (!compute || size == 0) {
        return NULL;
    }
    
    SamVulkanBuffer* buffer = calloc(1, sizeof(SamVulkanBuffer));
    if (!buffer) {
        return NULL;
    }
    
    buffer->compute = compute;
    buffer->size = size;
    buffer->usage = usage;
    buffer->properties = properties;
    
    VkBufferUsageFlags vk_usage = 0;
    if (usage & SAMVULKAN_BUFFER_STORAGE) vk_usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (usage & SAMVULKAN_BUFFER_UNIFORM) vk_usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (usage & SAMVULKAN_BUFFER_TRANSFER_SRC) vk_usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (usage & SAMVULKAN_BUFFER_TRANSFER_DST) vk_usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    
    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = vk_usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    
    if (vkCreateBuffer(compute->vk_device, &buffer_info, NULL, &buffer->buffer) != VK_SUCCESS) {
        free(buffer);
        return NULL;
    }
    
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(compute->vk_device, buffer->buffer, &mem_requirements);
    
    VkMemoryPropertyFlags vk_properties = 0;
    if (properties & SAMVULKAN_MEMORY_DEVICE_LOCAL) vk_properties |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (properties & SAMVULKAN_MEMORY_HOST_VISIBLE) vk_properties |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    if (properties & SAMVULKAN_MEMORY_HOST_COHERENT) vk_properties |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    
    uint32_t memory_type = find_memory_type(compute->physical_device, mem_requirements.memoryTypeBits, vk_properties);
    if (memory_type == UINT32_MAX) {
        vkDestroyBuffer(compute->vk_device, buffer->buffer, NULL);
        free(buffer);
        return NULL;
    }
    
    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = memory_type
    };
    
    if (vkAllocateMemory(compute->vk_device, &alloc_info, NULL, &buffer->memory) != VK_SUCCESS) {
        vkDestroyBuffer(compute->vk_device, buffer->buffer, NULL);
        free(buffer);
        return NULL;
    }
    
    vkBindBufferMemory(compute->vk_device, buffer->buffer, buffer->memory, 0);
    
    return buffer;
}

void samvulkan_compute_destroy_buffer(SamVulkanBuffer* buffer) {
    if (!buffer) return;
    
    if (buffer->mapped_data) {
        vkUnmapMemory(buffer->compute->vk_device, buffer->memory);
    }
    
    vkDestroyBuffer(buffer->compute->vk_device, buffer->buffer, NULL);
    vkFreeMemory(buffer->compute->vk_device, buffer->memory, NULL);
    free(buffer);
}

void* samvulkan_compute_map_buffer(SamVulkanBuffer* buffer) {
    if (!buffer || buffer->mapped_data) {
        return buffer ? buffer->mapped_data : NULL;
    }
    
    if (!(buffer->properties & SAMVULKAN_MEMORY_HOST_VISIBLE)) {
        return NULL;
    }
    
    if (vkMapMemory(buffer->compute->vk_device, buffer->memory, 0, buffer->size, 0, &buffer->mapped_data) != VK_SUCCESS) {
        return NULL;
    }
    
    return buffer->mapped_data;
}

void samvulkan_compute_unmap_buffer(SamVulkanBuffer* buffer) {
    if (!buffer || !buffer->mapped_data) return;
    
    vkUnmapMemory(buffer->compute->vk_device, buffer->memory);
    buffer->mapped_data = NULL;
}

bool samvulkan_compute_upload_buffer(SamVulkanBuffer* buffer, const void* data, size_t size) {
    if (!buffer || !data || size > buffer->size) {
        return false;
    }
    
    void* mapped = samvulkan_compute_map_buffer(buffer);
    if (!mapped) {
        return false;
    }
    
    memcpy(mapped, data, size);
    samvulkan_compute_unmap_buffer(buffer);
    
    return true;
}

bool samvulkan_compute_download_buffer(SamVulkanBuffer* buffer, void* data, size_t size) {
    if (!buffer || !data || size > buffer->size) {
        return false;
    }
    
    void* mapped = samvulkan_compute_map_buffer(buffer);
    if (!mapped) {
        return false;
    }
    
    memcpy(data, mapped, size);
    samvulkan_compute_unmap_buffer(buffer);
    
    return true;
}

void samvulkan_compute_bind_buffer(SamVulkanCompute* compute, uint32_t binding, SamVulkanBuffer* buffer) {
    if (!compute || !buffer || !compute->descriptor_set) return;
    
    VkDescriptorBufferInfo buffer_info = {
        .buffer = buffer->buffer,
        .offset = 0,
        .range = buffer->size
    };
    
    VkWriteDescriptorSet descriptor_write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = compute->descriptor_set,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .pBufferInfo = &buffer_info
    };
    
    vkUpdateDescriptorSets(compute->vk_device, 1, &descriptor_write, 0, NULL);
}

void samvulkan_compute_set_push_constants(SamVulkanCompute* compute, const void* data, size_t size) {
    if (!compute || !data || size > compute->push_constant_size) return;
    
    vkCmdPushConstants(compute->command_buffer, compute->pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, (uint32_t)size, data);
}

bool samvulkan_compute_dispatch(SamVulkanCompute* compute, uint32_t x, uint32_t y, uint32_t z) {
    if (!compute || !compute->compute_pipeline) {
        return false;
    }
    
    vkWaitForFences(compute->vk_device, 1, &compute->fence, VK_TRUE, UINT64_MAX);
    vkResetFences(compute->vk_device, 1, &compute->fence);
    
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    
    if (vkBeginCommandBuffer(compute->command_buffer, &begin_info) != VK_SUCCESS) {
        return false;
    }
    
    vkCmdBindPipeline(compute->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute->compute_pipeline);
    vkCmdBindDescriptorSets(compute->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute->pipeline_layout, 0, 1, &compute->descriptor_set, 0, NULL);
    vkCmdDispatch(compute->command_buffer, x, y, z);
    
    if (vkEndCommandBuffer(compute->command_buffer) != VK_SUCCESS) {
        return false;
    }
    
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &compute->command_buffer
    };
    
    return vkQueueSubmit(compute->compute_queue, 1, &submit_info, compute->fence) == VK_SUCCESS;
}

bool samvulkan_compute_wait(SamVulkanCompute* compute) {
    if (!compute) {
        return false;
    }
    
    return vkWaitForFences(compute->vk_device, 1, &compute->fence, VK_TRUE, UINT64_MAX) == VK_SUCCESS;
}

bool samvulkan_compute_submit_and_wait(SamVulkanCompute* compute, uint32_t x, uint32_t y, uint32_t z) {
    if (!samvulkan_compute_dispatch(compute, x, y, z)) {
        return false;
    }
    
    return samvulkan_compute_wait(compute);
}