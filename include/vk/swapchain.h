#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

#include "device.h"
#include "platform_window.h"

typedef struct {
    VkSwapchainKHR vk_swapchain;
    VkFormat vk_image_format;
    VkExtent2D extent;

    uint32_t vk_image_count;
    VkImage *vk_images;
    VkImageView *vk_image_views;
} swapchain_t;

bool swapchain_create(swapchain_t *swapchain,
                      const device_t *device,
                      VkSurfaceKHR vk_surface,
                      const platform_window_t *window);

bool swapchain_recreate(swapchain_t *swapchain,
                        const device_t *device,
                        VkSurfaceKHR vk_surface,
                        const platform_window_t *window);

void swapchain_destroy(swapchain_t *swapchain, const device_t *device);
