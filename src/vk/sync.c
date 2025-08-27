#include "vk/sync.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "util/util.h"

bool sync_create(sync_t *sync, const device_t *device, uint32_t frame_count) {
    memset(sync, 0, sizeof(*sync));

    sync->frame_count = frame_count;
    sync->vk_semaphore_image_available =
        (VkSemaphore *)malloc(frame_count * sizeof(*sync->vk_semaphore_image_available));
    if (sync->vk_semaphore_image_available == NULL) {
        log_error("(SYNC) malloc failed.");
        sync_destroy(sync, device);
        return false;
    }
    sync->vk_semaphore_render_finished =
        (VkSemaphore *)malloc(frame_count * sizeof(*sync->vk_semaphore_render_finished));
    if (sync->vk_semaphore_render_finished == NULL) {
        log_error("(SYNC) malloc failed.");
        sync_destroy(sync, device);
        return false;
    }
    sync->vk_fence_in_flight = (VkFence *)malloc(frame_count * sizeof(*sync->vk_fence_in_flight));
    if (sync->vk_fence_in_flight == NULL) {
        log_error("(SYNC) malloc failed.");
        sync_destroy(sync, device);
        return false;
    }
    sync->vk_fence_present_done = (VkFence *)malloc(frame_count * sizeof(*sync->vk_fence_present_done));
    if (sync->vk_fence_present_done == NULL) {
        log_error("(SYNC) malloc failed.");
        sync_destroy(sync, device);
        return false;
    }

    VkSemaphoreCreateInfo semaphore_create_info = {0};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info = {0};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < frame_count; ++i) {
        VkResult res;
        res =
            vkCreateSemaphore(device->vk_device, &semaphore_create_info, NULL, &sync->vk_semaphore_image_available[i]);
        if (res != VK_SUCCESS) {
            log_error("(SYNC) vkCreateSemaphore failed (%s).", vk_res_str(res));
            sync->vk_semaphore_image_available = VK_NULL_HANDLE;
            sync_destroy(sync, device);
            return false;
        }
        res =
            vkCreateSemaphore(device->vk_device, &semaphore_create_info, NULL, &sync->vk_semaphore_render_finished[i]);
        if (res != VK_SUCCESS) {
            log_error("(SYNC) vkCreateSemaphore failed (%s).", vk_res_str(res));
            sync->vk_semaphore_render_finished = VK_NULL_HANDLE;
            sync_destroy(sync, device);
            return false;
        }
        res = vkCreateFence(device->vk_device, &fence_create_info, NULL, &sync->vk_fence_in_flight[i]);
        if (res != VK_SUCCESS) {
            log_error("(SYNC) vkCreateFence failed (%s).", vk_res_str(res));
            sync->vk_fence_in_flight = VK_NULL_HANDLE;
            sync_destroy(sync, device);
            return false;
        }
        res = vkCreateFence(device->vk_device, &fence_create_info, NULL, &sync->vk_fence_present_done[i]);
        if (res != VK_SUCCESS) {
            log_error("(SYNC) vkCreateFence failed (%s).", vk_res_str(res));
            sync->vk_fence_present_done = VK_NULL_HANDLE;
            sync_destroy(sync, device);
            return false;
        }
    }

    return true;
}

void sync_destroy(sync_t *sync, const device_t *device) {

    if (sync->vk_semaphore_image_available != NULL) {
        for (uint32_t i = 0; i < sync->frame_count; ++i) {
            if (sync->vk_semaphore_image_available[i] != VK_NULL_HANDLE) {
                vkDestroySemaphore(device->vk_device, sync->vk_semaphore_image_available[i], NULL);
            }
        }
        free(sync->vk_semaphore_image_available);
    }

    if (sync->vk_semaphore_render_finished != NULL) {
        for (uint32_t i = 0; i < sync->frame_count; ++i) {
            if (sync->vk_semaphore_render_finished[i] != VK_NULL_HANDLE) {
                vkDestroySemaphore(device->vk_device, sync->vk_semaphore_render_finished[i], NULL);
            }
        }
        free(sync->vk_semaphore_render_finished);
    }

    if (sync->vk_fence_in_flight != NULL) {
        for (uint32_t i = 0; i < sync->frame_count; ++i) {
            if (sync->vk_fence_in_flight[i] != VK_NULL_HANDLE) {
                vkDestroyFence(device->vk_device, sync->vk_fence_in_flight[i], NULL);
            }
        }
        free(sync->vk_fence_in_flight);
    }

    if (sync->vk_fence_present_done != NULL) {
        for (uint32_t i = 0; i < sync->frame_count; ++i) {
            if (sync->vk_fence_present_done[i] != VK_NULL_HANDLE) {
                vkDestroyFence(device->vk_device, sync->vk_fence_present_done[i], NULL);
            }
        }
        free(sync->vk_fence_present_done);
    }

    memset(sync, 0, sizeof(*sync));
}
