#pragma once
#include <stdint.h>

#include <vulkan/vulkan.h>

#include "vk_commands.h"
#include "vk_device.h"
#include "vk_pipeline.h"
#include "vk_renderpass.h"
#include "vk_swapchain.h"
#include "vk_sync.h"

typedef enum { VK_DRAW_OK = 0, VK_DRAW_NEED_RECREATE, VK_DRAW_ERROR } vk_draw_result_t;

vk_draw_result_t vk_draw_frame(const vk_device_t *device,
                               const vk_swapchain_t *swapchain,
                               const vk_renderpass_t *renderpass,
                               const vk_pipeline_t *pipeline,
                               const vk_commands_t *commands,
                               const vk_sync_t *sync,
                               uint32_t *current_frame);
