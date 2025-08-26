#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

#include "device.h"
#include "swapchain.h"

typedef struct {
    VkRenderPass render_pass;

    VkFramebuffer *framebuffers;
    uint32_t framebuffer_count;

    VkFormat color_format;
} renderpass_t;

bool renderpass_create(renderpass_t *renderpass, const device_t *device, const swapchain_t *swapchain);

void renderpass_destroy(renderpass_t *renderpass, const device_t *device);

bool renderpass_recreate_framebuffers(renderpass_t *renderpass, const device_t *device, const swapchain_t *swapchain);

bool renderpass_has_format_mismatch(const renderpass_t *renderpass, const swapchain_t *swapchain);
