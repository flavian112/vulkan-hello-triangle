#pragma once

#include "vk_device.h"

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

typedef struct {
    VkSemaphore *image_available;
    VkSemaphore *render_finished;
    VkFence *in_flight;
    VkFence *present_done;
    uint32_t count;
} vk_sync_t;

bool vk_sync_create(vk_sync_t *sync, const vk_device_t *device, uint32_t frame_count);

void vk_sync_destroy(vk_sync_t *sync, const vk_device_t *device);
