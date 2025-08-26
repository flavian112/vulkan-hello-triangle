#pragma once

#include <stdint.h>

#include <vulkan/vulkan.h>

#include "commands.h"
#include "device.h"
#include "pipeline.h"
#include "renderpass.h"
#include "swapchain.h"
#include "sync.h"

typedef enum { VK_DRAW_OK = 0, VK_DRAW_NEED_RECREATE, VK_DRAW_ERROR } draw_result_t;

draw_result_t draw_frame(const device_t *device,
                         const swapchain_t *swapchain,
                         const renderpass_t *renderpass,
                         const pipeline_t *pipeline,
                         const commands_t *commands,
                         const sync_t *sync,
                         uint32_t *current_frame);
