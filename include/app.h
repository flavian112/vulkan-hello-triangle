#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

#include "platform_window.h"
#include "vk/commands.h"
#include "vk/device.h"
#include "vk/instance.h"
#include "vk/pipeline.h"
#include "vk/renderpass.h"
#include "vk/swapchain.h"
#include "vk/sync.h"

typedef struct {
    platform_window_t *window;

    instance_t instance;
    VkSurfaceKHR surface;
    device_t device;
    swapchain_t swapchain;
    renderpass_t renderpass;
    pipeline_t pipeline;
    commands_t commands;
    sync_t sync;

    uint32_t current_frame;
} app_t;

bool app_create(app_t *app);
void app_run(app_t *app);
void app_destroy(app_t *app);
