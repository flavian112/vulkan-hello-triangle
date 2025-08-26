#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

#include "platform_window.h"
#include "vk/vk_commands.h"
#include "vk/vk_device.h"
#include "vk/vk_instance.h"
#include "vk/vk_pipeline.h"
#include "vk/vk_renderpass.h"
#include "vk/vk_swapchain.h"
#include "vk/vk_sync.h"

enum { MAX_FRAMES_IN_FLIGHT = 2 };

typedef struct {
    platform_window_t *window;

    vk_instance_t instance;
    VkSurfaceKHR surface;
    vk_device_t device;

    vk_swapchain_t swapchain;
    vk_renderpass_t renderpass;
    vk_pipeline_t pipeline;
    vk_commands_t commands;
    vk_sync_t sync;

    uint32_t cur_frame;
} app_t;

bool app_init(app_t *app);
void app_main_loop(app_t *app);
void app_deinit(app_t *app);
