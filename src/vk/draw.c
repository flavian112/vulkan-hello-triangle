#include "vk/draw.h"

#include <assert.h>
#include <stdlib.h>

#include "util/log.h"
#include "vk/debug.h"

draw_result_t draw_frame(
    const device_t     *device,
    const swapchain_t  *swapchain,
    const renderpass_t *renderpass,
    const pipeline_t   *pipeline,
    const commands_t   *commands,
    const sync_t       *sync,
    uint32_t           *current_frame
) {
    assert(*current_frame < sync->frame_count);

    VkResult res;
    res = vkWaitForFences(
        device->vk_device, 1, &sync->vk_fence_in_flight[*current_frame], VK_TRUE, UINT64_MAX
    );
    if (res != VK_SUCCESS) {
        log_error("(DRAW) vkWaitForFences failed (%s).", vk_res_str(res));
        return DRAW_ERROR;
    }
    res = vkResetFences(device->vk_device, 1, &sync->vk_fence_in_flight[*current_frame]);
    if (res != VK_SUCCESS) {
        log_error("(DRAW) vkResetFences failed (%s).", vk_res_str(res));
        return DRAW_ERROR;
    }

    res = vkWaitForFences(
        device->vk_device, 1, &sync->vk_fence_present_done[*current_frame], VK_TRUE, UINT64_MAX
    );
    if (res != VK_SUCCESS) {
        log_error("(DRAW) vkWaitForFences failed (%s).", vk_res_str(res));
        return DRAW_ERROR;
    }
    res = vkResetFences(device->vk_device, 1, &sync->vk_fence_present_done[*current_frame]);
    if (res != VK_SUCCESS) {
        log_error("(DRAW) vkResetFences failed (%s).", vk_res_str(res));
        return DRAW_ERROR;
    }

    uint32_t image_index = 0;
    res                  = vkAcquireNextImageKHR(
        device->vk_device,
        swapchain->vk_swapchain,
        UINT64_MAX,
        sync->vk_semaphore_image_available[*current_frame],
        VK_NULL_HANDLE,
        &image_index
    );
    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        return DRAW_NEED_RECREATE;
    } else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
        log_error("(DRAW) vkAcquireNextImageKHR failed (%s).", vk_res_str(res));
        return DRAW_ERROR;
    }

    res = vkResetCommandBuffer(commands->vk_buffers[*current_frame], 0);
    if (res != VK_SUCCESS) {
        log_error("(DRAW) vkResetCommandBuffer failed (%s).", vk_res_str(res));
        return DRAW_ERROR;
    }

    if (!commands_record_frame(
            commands, device, renderpass, pipeline, swapchain, image_index, *current_frame
        )) {
        return DRAW_ERROR;
    }

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info         = {0};
    submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount   = 1;
    submit_info.pWaitSemaphores      = &sync->vk_semaphore_image_available[*current_frame];
    submit_info.pWaitDstStageMask    = &wait_stage;
    submit_info.commandBufferCount   = 1;
    submit_info.pCommandBuffers      = &commands->vk_buffers[*current_frame];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = &sync->vk_semaphore_render_finished[*current_frame];

    res = vkQueueSubmit(
        device->graphics_queue, 1, &submit_info, sync->vk_fence_in_flight[*current_frame]
    );
    if (res != VK_SUCCESS) {
        log_error("(DRAW) vkQueueSubmit failed (%s).", vk_res_str(res));
        return DRAW_ERROR;
    }

    VkSwapchainPresentFenceInfoKHR swapchain_present_fence_info = {0};
    swapchain_present_fence_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_KHR;
    swapchain_present_fence_info.swapchainCount = 1;
    swapchain_present_fence_info.pFences        = &sync->vk_fence_present_done[*current_frame];

    VkPresentInfoKHR present_info   = {0};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext              = &swapchain_present_fence_info;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = &sync->vk_semaphore_render_finished[*current_frame];
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = &swapchain->vk_swapchain;
    present_info.pImageIndices      = &image_index;

    res = vkQueuePresentKHR(device->present_queue, &present_info);
    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
        *current_frame = (*current_frame + 1) % sync->frame_count;
        return DRAW_NEED_RECREATE;
    } else if (res != VK_SUCCESS) {
        log_error("(DRAW) vkQueuePresentKHR failed (%s).", vk_res_str(res));
        return DRAW_ERROR;
    }

    *current_frame = (*current_frame + 1) % sync->frame_count;

    return DRAW_SUCCESS;
}
