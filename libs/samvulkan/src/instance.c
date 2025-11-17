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

#include "samvulkan/instance.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <volk.h>

struct SamVulkanInstance {
  VkInstance instance;
  VkDebugUtilsMessengerEXT debug_messenger;
  VkPhysicalDevice *physical_devices;
  uint32_t physical_device_count;
};

static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
               VkDebugUtilsMessageTypeFlagsEXT messageType,
               const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {

  fprintf(stderr, "Vulkan validation: %s\n", pCallbackData->pMessage);
  return VK_FALSE;
}

SamVulkanInstance *samvulkan_instance_create(const SamVulkanInstanceConfig *config) {
  SamVulkanInstance *instance = calloc(1, sizeof(SamVulkanInstance));
  if (!instance) {
    return NULL;
  }

  VkApplicationInfo app_info = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = config->application_name ? config->application_name : "SamVulkan App",
      .applicationVersion = config->application_version,
      .pEngineName = config->engine_name ? config->engine_name : "SamVulkan",
      .engineVersion = config->engine_version,
      .apiVersion = VK_API_VERSION_1_2};

  VkInstanceCreateInfo create_info = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                      .pApplicationInfo = &app_info,
                                      .enabledExtensionCount = (uint32_t)config->extension_count,
                                      .ppEnabledExtensionNames = config->required_extensions};

  const char *validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
  if (config->enable_validation) {
    create_info.enabledLayerCount = 1;
    create_info.ppEnabledLayerNames = validation_layers;
  }

  VkResult result = vkCreateInstance(&create_info, NULL, &instance->instance);
  if (result != VK_SUCCESS) {
    free(instance);
    return NULL;
  }

  volkLoadInstance(instance->instance);

  if (config->enable_validation) {
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debug_callback};

    if (vkCreateDebugUtilsMessengerEXT) {
      vkCreateDebugUtilsMessengerEXT(instance->instance, &debug_create_info, NULL,
                                     &instance->debug_messenger);
    }
  }

  vkEnumeratePhysicalDevices(instance->instance, &instance->physical_device_count, NULL);
  if (instance->physical_device_count > 0) {
    instance->physical_devices = calloc(instance->physical_device_count, sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance->instance, &instance->physical_device_count,
                               instance->physical_devices);
  }

  return instance;
}

void samvulkan_instance_destroy(SamVulkanInstance *instance) {
  if (!instance)
    return;

  if (instance->debug_messenger && vkDestroyDebugUtilsMessengerEXT) {
    vkDestroyDebugUtilsMessengerEXT(instance->instance, instance->debug_messenger, NULL);
  }

  if (instance->physical_devices) {
    free(instance->physical_devices);
  }

  vkDestroyInstance(instance->instance, NULL);
  free(instance);
}

void *samvulkan_instance_get_handle(SamVulkanInstance *instance) {
  return instance ? instance->instance : NULL;
}

size_t samvulkan_instance_get_physical_device_count(SamVulkanInstance *instance) {
  return instance ? instance->physical_device_count : 0;
}

void *samvulkan_instance_get_physical_device(SamVulkanInstance *instance, size_t index) {
  if (!instance || index >= instance->physical_device_count) {
    return NULL;
  }
  return instance->physical_devices[index];
}