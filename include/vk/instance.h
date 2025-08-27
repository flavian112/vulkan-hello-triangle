#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

typedef struct {
    VkInstance vk_instance;
    VkDebugUtilsMessengerEXT vk_debug_utils_messenger;
} instance_t;

bool instance_create(instance_t *instance);
void instance_destroy(instance_t *instance);
