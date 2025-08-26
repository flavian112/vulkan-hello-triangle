#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "platform_window.h"
#include "vk_device.h"

typedef struct {
    VkSwapchainKHR handle;
    VkFormat image_format;
    VkExtent2D extent;

    uint32_t image_count;
    VkImage *images;
    VkImageView *image_views;
} vk_swapchain_t;

bool vk_swapchain_create(vk_swapchain_t *swapchain,
                         const vk_device_t *device,
                         VkSurfaceKHR surface,
                         const platform_window_t *window);

bool vk_swapchain_recreate(vk_swapchain_t *swapchain,
                           const vk_device_t *device,
                           VkSurfaceKHR surface,
                           const platform_window_t *window);

void vk_swapchain_destroy(vk_swapchain_t *swapchain, const vk_device_t *device);
