#include "vk/draw.h"

#include <assert.h>
#include <stdlib.h>

#include "util/log.h"
#include "util/util.h"

draw_result_t draw_frame(const device_t *device,
                         const swapchain_t *swapchain,
                         const renderpass_t *renderpass,
                         const pipeline_t *pipeline,
                         const commands_t *commands,
                         const sync_t *sync,
                         uint32_t *current_frame) {
    assert(device != NULL);
    assert(swapchain != NULL);
    assert(renderpass != NULL);
    assert(commands != NULL);
    assert(sync != NULL);
    assert(current_frame != NULL);

    *current_frame %= sync->count;

    VK_CHECK(vkWaitForFences(device->vk_device, 1, &sync->in_flight[*current_frame], VK_TRUE, UINT64_MAX));
    VK_CHECK(vkResetFences(device->vk_device, 1, &sync->in_flight[*current_frame]));

    VK_CHECK(vkWaitForFences(device->vk_device, 1, &sync->present_done[*current_frame], VK_TRUE, UINT64_MAX));
    VK_CHECK(vkResetFences(device->vk_device, 1, &sync->present_done[*current_frame]));

    uint32_t image_index = 0;
    VkResult res = vkAcquireNextImageKHR(device->vk_device,
                                         swapchain->handle,
                                         UINT64_MAX,
                                         sync->image_available[*current_frame],
                                         VK_NULL_HANDLE,
                                         &image_index);

    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        return VK_DRAW_NEED_RECREATE;
    } else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
        log_error("VULKAN Failed to acquire image.");
        return VK_DRAW_ERROR;
    }

    VK_CHECK(vkResetCommandBuffer(commands->vk_buffers[*current_frame], 0));
    if (!commands_record_frame(commands, device, renderpass, pipeline, swapchain, image_index, *current_frame)) {
        return VK_DRAW_ERROR;
    }

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo si = {0};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = &sync->image_available[*current_frame];
    si.pWaitDstStageMask = &wait_stage;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &commands->vk_buffers[*current_frame];
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = &sync->render_finished[*current_frame];

    VK_CHECK(vkQueueSubmit(device->graphics_queue, 1, &si, sync->in_flight[*current_frame]));

    VkSwapchainPresentFenceInfoKHR spfi = {0};
    spfi.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_KHR;
    spfi.swapchainCount = 1;
    spfi.pFences = &sync->present_done[*current_frame];

    VkPresentInfoKHR pi = {0};
    pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pi.pNext = &spfi;
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = &sync->render_finished[*current_frame];
    pi.swapchainCount = 1;
    pi.pSwapchains = &swapchain->handle;
    pi.pImageIndices = &image_index;

    res = vkQueuePresentKHR(device->present_queue, &pi);

    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
        *current_frame = (*current_frame + 1) % sync->count;
        return VK_DRAW_NEED_RECREATE;
    } else if (res != VK_SUCCESS) {
        log_error("VULKAN Failed to present.");
        return VK_DRAW_ERROR;
    }

    *current_frame = (*current_frame + 1) % sync->count;
    return VK_DRAW_OK;
}
