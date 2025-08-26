#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

#include "device.h"

typedef struct {
    VkSemaphore *image_available;
    VkSemaphore *render_finished;
    VkFence *in_flight;
    VkFence *present_done;
    uint32_t count;
} sync_t;

bool sync_create(sync_t *sync, const device_t *device, uint32_t frame_count);

void sync_destroy(sync_t *sync, const device_t *device);
