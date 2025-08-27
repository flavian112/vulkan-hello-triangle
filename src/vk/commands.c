#include "vk/commands.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "util/util.h"

bool commands_create(commands_t *commands, const device_t *device, uint32_t frame_count) {
    memset(commands, 0, sizeof(*commands));

    VkCommandPoolCreateInfo command_pool_create_info = {0};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = device->graphics_queue_familiy_index;

    VkResult res;
    res = vkCreateCommandPool(device->vk_device, &command_pool_create_info, NULL, &commands->vk_pool);
    if (res != VK_SUCCESS) {
        log_error("(COMMANDS) vkCreateCommandPool failed (%s).", vk_res_str(res));
        commands->vk_pool = VK_NULL_HANDLE;
        commands_destroy(commands, device);
        return false;
    }

    if (frame_count > 0) {
        commands->vk_buffers = (VkCommandBuffer *)malloc(frame_count * sizeof(*commands->vk_buffers));
        if (commands->vk_buffers == NULL) {
            log_error("(COMMANDS) malloc failed.");
            commands_destroy(commands, device);
            return false;
        }
        commands->vk_buffers_count = frame_count;
    }

    VkCommandBufferAllocateInfo command_buffer_allocate_info = {0};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = commands->vk_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = frame_count;

    res = vkAllocateCommandBuffers(device->vk_device, &command_buffer_allocate_info, commands->vk_buffers);
    if (res != VK_SUCCESS) {
        log_error("(COMMANDS) vkAllocateCommandBuffers failed (%s).", vk_res_str(res));
        commands_destroy(commands, device);
        return false;
    }

    return true;
}

void commands_destroy(commands_t *commands, const device_t *device) {
    if (commands->vk_pool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device->vk_device, commands->vk_pool, NULL);
    }

    if (commands->vk_buffers != NULL) {
        free(commands->vk_buffers);
    }

    memset(commands, 0, sizeof(*commands));
}

bool commands_record_frame(const commands_t *commands,
                           const device_t *device,
                           const renderpass_t *renderpass,
                           const pipeline_t *pipeline,
                           const swapchain_t *swapchain,
                           uint32_t image_index,
                           uint32_t frame_index) {
    assert(frame_index < commands->vk_buffers_count);

    VkCommandBuffer command_buffer = commands->vk_buffers[frame_index];

    VkCommandBufferBeginInfo command_buffer_create_info = {0};
    command_buffer_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkResult res;
    res = vkBeginCommandBuffer(command_buffer, &command_buffer_create_info);
    if (res != VK_SUCCESS) {
        log_error("(COMMANDS) vkBeginCommandBuffer failed (%s).", vk_res_str(res));
        return false;
    }

    VkClearValue clear_value = {0};
    clear_value.color = (VkClearColorValue){{0.0F, 0.0F, 0.0F, 1.0F}};

    VkRenderPassBeginInfo render_pass_begin_info = {0};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = renderpass->vk_render_pass;
    render_pass_begin_info.framebuffer = renderpass->vk_framebuffers[image_index];
    render_pass_begin_info.renderArea.offset = (VkOffset2D){0, 0};
    render_pass_begin_info.renderArea.extent = swapchain->extent;
    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = &clear_value;

    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk_pipeline);

    VkViewport viewport = {0};
    viewport.x = 0.0F;
    viewport.y = 0.0F;
    viewport.width = (float)swapchain->extent.width;
    viewport.height = (float)swapchain->extent.height;
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;

    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = swapchain->extent;

    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    vkCmdDraw(command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(command_buffer);

    res = vkEndCommandBuffer(command_buffer);
    if (res != VK_SUCCESS) {
        log_error("(COMMANDS) vkEndCommandBuffer failed (%s).", vk_res_str(res));
        return false;
    }

    return true;
}
