#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

typedef struct {
    VkPhysicalDevice vk_physical_device;
    VkDevice vk_device;

    uint32_t graphics_queue_familiy_index;
    uint32_t present_queue_family_index;
    bool has_graphics_queue;
    bool has_present_queue;
    VkQueue graphics_queue;
    VkQueue present_queue;
} device_t;

bool device_create(device_t *device, VkInstance vk_instance, VkSurfaceKHR vk_surface);

void device_destroy(device_t *device);
