#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

typedef struct {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
} vk_instance_t;

bool vk_instance_create(vk_instance_t *instance);
void vk_instance_destroy(vk_instance_t *instance);
