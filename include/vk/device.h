#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

typedef struct {
    VkPhysicalDevice physical;
    VkDevice logical;

    uint32_t graphics_family;
    uint32_t present_family;
    bool has_graphics;
    bool has_present;
    VkQueue graphics_queue;
    VkQueue present_queue;
} device_t;

bool device_create(device_t *device, VkInstance instance, VkSurfaceKHR surface);

void device_destroy(device_t *device);
