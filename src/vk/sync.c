#include "vk/sync.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "util/util.h"

bool sync_create(sync_t *sync, const device_t *device, uint32_t frame_count) {
    assert(sync != NULL);
    assert(device != NULL);

    memset(sync, 0, sizeof(*sync));

    sync->count = frame_count;
    sync->image_available = (VkSemaphore *)malloc(frame_count * sizeof(*sync->image_available));
    assert(sync->image_available != NULL);
    sync->render_finished = (VkSemaphore *)malloc(frame_count * sizeof(*sync->render_finished));
    assert(sync->render_finished != NULL);
    sync->in_flight = (VkFence *)malloc(frame_count * sizeof(*sync->in_flight));
    assert(sync->in_flight != NULL);
    sync->present_done = (VkFence *)malloc(frame_count * sizeof(*sync->present_done));
    assert(sync->present_done != NULL);

    VkSemaphoreCreateInfo sci = {0};
    sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fci = {0};
    fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < frame_count; ++i) {
        VK_CHECK(vkCreateSemaphore(device->logical, &sci, NULL, &sync->image_available[i]));
        VK_CHECK(vkCreateSemaphore(device->logical, &sci, NULL, &sync->render_finished[i]));
        VK_CHECK(vkCreateFence(device->logical, &fci, NULL, &sync->in_flight[i]));
        VK_CHECK(vkCreateFence(device->logical, &fci, NULL, &sync->present_done[i]));
    }

    return true;
}

void sync_destroy(sync_t *sync, const device_t *device) {
    assert(sync != NULL);
    assert(device != NULL);

    if (sync->image_available != NULL) {
        for (uint32_t i = 0; i < sync->count; ++i) {
            if (sync->image_available[i] != VK_NULL_HANDLE) {
                vkDestroySemaphore(device->logical, sync->image_available[i], NULL);
                sync->image_available[i] = VK_NULL_HANDLE;
            }
        }
        free(sync->image_available);
        sync->image_available = NULL;
    }

    if (sync->render_finished != NULL) {
        for (uint32_t i = 0; i < sync->count; ++i) {
            if (sync->render_finished[i] != VK_NULL_HANDLE) {
                vkDestroySemaphore(device->logical, sync->render_finished[i], NULL);
                sync->render_finished[i] = VK_NULL_HANDLE;
            }
        }
        free(sync->render_finished);
        sync->render_finished = NULL;
    }

    if (sync->in_flight != NULL) {
        for (uint32_t i = 0; i < sync->count; ++i) {
            if (sync->in_flight[i] != VK_NULL_HANDLE) {
                vkDestroyFence(device->logical, sync->in_flight[i], NULL);
                sync->in_flight[i] = VK_NULL_HANDLE;
            }
        }
        free(sync->in_flight);
        sync->in_flight = NULL;
    }

    if (sync->present_done != NULL) {
        for (uint32_t i = 0; i < sync->count; ++i) {
            if (sync->present_done[i] != VK_NULL_HANDLE) {
                vkDestroyFence(device->logical, sync->present_done[i], NULL);
                sync->present_done[i] = VK_NULL_HANDLE;
            }
        }
        free(sync->present_done);
        sync->present_done = NULL;
    }

    memset(sync, 0, sizeof(*sync));
}
