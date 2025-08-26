#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

#include "device.h"
#include "pipeline.h"
#include "renderpass.h"
#include "swapchain.h"

typedef struct {
    VkCommandPool pool;
    VkCommandBuffer *buffers;
    uint32_t count;
} commands_t;

bool commands_create(commands_t *commands, const device_t *device, uint32_t frame_count);

void commands_destroy(commands_t *commands, const device_t *device);

bool commands_record_frame(const commands_t *commands,
                           const device_t *device,
                           const renderpass_t *renderpass,
                           const pipeline_t *pipeline,
                           const swapchain_t *swapchain,
                           uint32_t image_index,
                           uint32_t frame_index);
