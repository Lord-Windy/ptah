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
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("Testing SamVulkan initialization...\n");
    
    if (!samvulkan_init()) {
        printf("Failed to initialize SamVulkan (Volk). This is expected if Vulkan is not installed.\n");
        return 0;
    }
    
    printf("SamVulkan initialized successfully!\n");
    
    if (samvulkan_is_available()) {
        uint32_t major, minor, patch;
        samvulkan_get_version(&major, &minor, &patch);
        printf("Vulkan version: %u.%u.%u\n", major, minor, patch);
        
        SamVulkanInstanceConfig config = {
            .application_name = "SamVulkan Test",
            .application_version = 1,
            .engine_name = "SamVulkan",
            .engine_version = 1,
            .enable_validation = false,
            .required_extensions = NULL,
            .extension_count = 0
        };
        
        SamVulkanInstance* instance = samvulkan_instance_create(&config);
        if (instance) {
            printf("Created Vulkan instance successfully!\n");
            
            size_t device_count = samvulkan_instance_get_physical_device_count(instance);
            printf("Found %zu physical device(s)\n", device_count);
            
            if (device_count > 0) {
                void* physical_device = samvulkan_instance_get_physical_device(instance, 0);
                
                SamVulkanDeviceConfig device_config = {
                    .physical_device = physical_device,
                    .required_queues = SAMVULKAN_QUEUE_COMPUTE,
                    .required_extensions = NULL,
                    .extension_count = 0,
                    .required_features = NULL,
                    .feature_count = 0
                };
                
                SamVulkanDevice* device = samvulkan_device_create(instance, &device_config);
                if (device) {
                    printf("Created logical device successfully!\n");
                    
                    if (samvulkan_device_supports_compute(device)) {
                        printf("Device supports compute shaders!\n");
                        
                        SamVulkanCompute* compute = samvulkan_compute_create(device);
                        if (compute) {
                            printf("Created compute context successfully!\n");
                            samvulkan_compute_destroy(compute);
                        }
                    }
                    
                    char device_name[256];
                    uint32_t vendor_id;
                    samvulkan_device_get_properties(device, device_name, sizeof(device_name), &vendor_id);
                    printf("Device: %s (Vendor ID: 0x%04X)\n", device_name, vendor_id);
                    
                    samvulkan_device_destroy(device);
                }
            }
            
            samvulkan_instance_destroy(instance);
        }
    } else {
        printf("Vulkan is not available on this system.\n");
    }
    
    samvulkan_cleanup();
    printf("SamVulkan test completed.\n");
    
    return 0;
}