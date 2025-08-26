#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

#include "vk_device.h"
#include "vk_pipeline.h"
#include "vk_renderpass.h"
#include "vk_swapchain.h"

typedef struct {
    VkCommandPool pool;
    VkCommandBuffer *buffers;
    uint32_t count;
} vk_commands_t;

bool vk_commands_create(vk_commands_t *commands, const vk_device_t *device, uint32_t frame_count);

void vk_commands_destroy(vk_commands_t *commands, const vk_device_t *device);

bool vk_commands_record_frame(const vk_commands_t *commands,
                              const vk_device_t *device,
                              const vk_renderpass_t *renderpass,
                              const vk_pipeline_t *pipeline,
                              const vk_swapchain_t *swapchain,
                              uint32_t image_index,
                              uint32_t frame_index);
