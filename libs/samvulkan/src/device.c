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

#include "samvulkan/device.h"
#include "samvulkan/instance.h"
#include <volk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct SamVulkanDevice {
    VkDevice device;
    VkPhysicalDevice physical_device;
    VkQueue compute_queue;
    VkQueue graphics_queue;
    VkQueue transfer_queue;
    uint32_t compute_queue_family;
    uint32_t graphics_queue_family;
    uint32_t transfer_queue_family;
    bool has_compute;
    bool has_graphics;
    bool has_transfer;
};

static uint32_t find_queue_family(VkPhysicalDevice physical_device, VkQueueFlags required_flags) {
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);
    
    VkQueueFamilyProperties* queue_families = calloc(queue_family_count, sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);
    
    for (uint32_t i = 0; i < queue_family_count; i++) {
        if ((queue_families[i].queueFlags & required_flags) == required_flags) {
            free(queue_families);
            return i;
        }
    }
    
    free(queue_families);
    return UINT32_MAX;
}

SamVulkanDevice* samvulkan_device_create(SamVulkanInstance* instance, const SamVulkanDeviceConfig* config) {
    if (!instance || !config || !config->physical_device) {
        return NULL;
    }
    
    SamVulkanDevice* device = calloc(1, sizeof(SamVulkanDevice));
    if (!device) {
        return NULL;
    }
    
    device->physical_device = (VkPhysicalDevice)config->physical_device;
    
    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_infos[3];
    uint32_t queue_create_count = 0;
    uint32_t unique_families[3];
    uint32_t unique_family_count = 0;
    
    if (config->required_queues & SAMVULKAN_QUEUE_COMPUTE) {
        device->compute_queue_family = find_queue_family(device->physical_device, VK_QUEUE_COMPUTE_BIT);
        if (device->compute_queue_family != UINT32_MAX) {
            device->has_compute = true;
            bool found = false;
            for (uint32_t i = 0; i < unique_family_count; i++) {
                if (unique_families[i] == device->compute_queue_family) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                unique_families[unique_family_count++] = device->compute_queue_family;
            }
        }
    }
    
    if (config->required_queues & SAMVULKAN_QUEUE_GRAPHICS) {
        device->graphics_queue_family = find_queue_family(device->physical_device, VK_QUEUE_GRAPHICS_BIT);
        if (device->graphics_queue_family != UINT32_MAX) {
            device->has_graphics = true;
            bool found = false;
            for (uint32_t i = 0; i < unique_family_count; i++) {
                if (unique_families[i] == device->graphics_queue_family) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                unique_families[unique_family_count++] = device->graphics_queue_family;
            }
        }
    }
    
    if (config->required_queues & SAMVULKAN_QUEUE_TRANSFER) {
        device->transfer_queue_family = find_queue_family(device->physical_device, VK_QUEUE_TRANSFER_BIT);
        if (device->transfer_queue_family != UINT32_MAX) {
            device->has_transfer = true;
            bool found = false;
            for (uint32_t i = 0; i < unique_family_count; i++) {
                if (unique_families[i] == device->transfer_queue_family) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                unique_families[unique_family_count++] = device->transfer_queue_family;
            }
        }
    }
    
    for (uint32_t i = 0; i < unique_family_count; i++) {
        VkDeviceQueueCreateInfo queue_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = unique_families[i],
            .queueCount = 1,
            .pQueuePriorities = &queue_priority
        };
        queue_create_infos[queue_create_count++] = queue_create_info;
    }
    
    VkPhysicalDeviceFeatures device_features = {0};
    
    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = queue_create_count,
        .pQueueCreateInfos = queue_create_infos,
        .pEnabledFeatures = &device_features,
        .enabledExtensionCount = (uint32_t)config->extension_count,
        .ppEnabledExtensionNames = config->required_extensions
    };
    
    VkResult result = vkCreateDevice(device->physical_device, &create_info, NULL, &device->device);
    if (result != VK_SUCCESS) {
        free(device);
        return NULL;
    }
    
    if (device->has_compute) {
        vkGetDeviceQueue(device->device, device->compute_queue_family, 0, &device->compute_queue);
    }
    if (device->has_graphics) {
        vkGetDeviceQueue(device->device, device->graphics_queue_family, 0, &device->graphics_queue);
    }
    if (device->has_transfer) {
        vkGetDeviceQueue(device->device, device->transfer_queue_family, 0, &device->transfer_queue);
    }
    
    return device;
}

void samvulkan_device_destroy(SamVulkanDevice* device) {
    if (!device) return;
    
    vkDestroyDevice(device->device, NULL);
    free(device);
}

void* samvulkan_device_get_handle(SamVulkanDevice* device) {
    return device ? device->device : NULL;
}

void* samvulkan_device_get_queue(SamVulkanDevice* device, SamVulkanQueueType type) {
    if (!device) return NULL;
    
    switch (type) {
        case SAMVULKAN_QUEUE_COMPUTE:
            return device->compute_queue;
        case SAMVULKAN_QUEUE_GRAPHICS:
            return device->graphics_queue;
        case SAMVULKAN_QUEUE_TRANSFER:
            return device->transfer_queue;
        default:
            return NULL;
    }
}

uint32_t samvulkan_device_get_queue_family_index(SamVulkanDevice* device, SamVulkanQueueType type) {
    if (!device) return UINT32_MAX;
    
    switch (type) {
        case SAMVULKAN_QUEUE_COMPUTE:
            return device->compute_queue_family;
        case SAMVULKAN_QUEUE_GRAPHICS:
            return device->graphics_queue_family;
        case SAMVULKAN_QUEUE_TRANSFER:
            return device->transfer_queue_family;
        default:
            return UINT32_MAX;
    }
}

bool samvulkan_device_supports_compute(SamVulkanDevice* device) {
    return device ? device->has_compute : false;
}

void samvulkan_device_get_properties(SamVulkanDevice* device, char* name, size_t name_size, uint32_t* vendor_id) {
    if (!device) return;
    
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device->physical_device, &properties);
    
    if (name && name_size > 0) {
        strncpy(name, properties.deviceName, name_size - 1);
        name[name_size - 1] = '\0';
    }
    
    if (vendor_id) {
        *vendor_id = properties.vendorID;
    }
}

void* samvulkan_device_get_physical_device(SamVulkanDevice* device) {
    return device ? device->physical_device : NULL;
}