#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

#include "device.h"

typedef struct {
    VkSemaphore *vk_semaphore_image_available;
    VkSemaphore *vk_semaphore_render_finished;
    VkFence *vk_fence_in_flight;
    VkFence *vk_fence_present_done;
    uint32_t frame_count;
} sync_t;

bool sync_create(sync_t *sync, const device_t *device, uint32_t frame_count);

void sync_destroy(sync_t *sync, const device_t *device);
