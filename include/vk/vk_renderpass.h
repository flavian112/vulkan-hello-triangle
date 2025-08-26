#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "vk_device.h"
#include "vk_swapchain.h"

typedef struct {
    VkRenderPass render_pass;

    VkFramebuffer *framebuffers;
    uint32_t framebuffer_count;

    VkFormat color_format;
} vk_renderpass_t;

bool vk_renderpass_create(vk_renderpass_t *renderpass, const vk_device_t *device, const vk_swapchain_t *swapchain);

void vk_renderpass_destroy(vk_renderpass_t *renderpass, const vk_device_t *device);

bool vk_renderpass_recreate_framebuffers(vk_renderpass_t *renderpass,
                                         const vk_device_t *device,
                                         const vk_swapchain_t *swapchain);

static inline bool vk_renderpass_format_mismatch(const vk_renderpass_t *renderpass, const vk_swapchain_t *swapchain) {
    return renderpass->render_pass != VK_NULL_HANDLE && renderpass->color_format != swapchain->image_format;
}
