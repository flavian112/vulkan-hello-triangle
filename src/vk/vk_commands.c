#include "vk/vk_commands.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

bool vk_commands_create(vk_commands_t *commands, const vk_device_t *device, uint32_t frame_count) {
    assert(commands != NULL);
    assert(device != NULL);

    memset(commands, 0, sizeof(*commands));

    VkCommandPoolCreateInfo cpci = {0};
    cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cpci.queueFamilyIndex = device->graphics_family;

    VK_CHECK(vkCreateCommandPool(device->logical, &cpci, NULL, &commands->pool));

    commands->buffers = (VkCommandBuffer *)malloc(frame_count * sizeof(*commands->buffers));
    assert(commands->buffers != NULL);
    commands->count = frame_count;

    VkCommandBufferAllocateInfo cbai = {0};
    cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbai.commandPool = commands->pool;
    cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbai.commandBufferCount = frame_count;

    VK_CHECK(vkAllocateCommandBuffers(device->logical, &cbai, commands->buffers));

    return true;
}

void vk_commands_destroy(vk_commands_t *commands, const vk_device_t *device) {
    assert(commands != NULL);
    assert(device != NULL);

    if (commands->pool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device->logical, commands->pool, NULL);
        commands->pool = VK_NULL_HANDLE;
    }

    if (commands->buffers != NULL) {
        free(commands->buffers);
        commands->buffers = NULL;
    }

    memset(commands, 0, sizeof(*commands));
}

bool vk_commands_record_frame(const vk_commands_t *commands,
                              const vk_device_t *device,
                              const vk_renderpass_t *renderpass,
                              const vk_pipeline_t *pipeline,
                              const vk_swapchain_t *swapchain,
                              uint32_t image_index,
                              uint32_t frame_index) {
    assert(commands != NULL);
    assert(device != NULL);
    assert(renderpass != NULL);
    assert(pipeline != NULL);
    assert(swapchain != NULL);
    assert(frame_index < commands->count);

    VkCommandBuffer cmd = commands->buffers[frame_index];

    VkCommandBufferBeginInfo cbbi = {0};
    cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_CHECK(vkBeginCommandBuffer(cmd, &cbbi));

    VkClearValue clear = {0};
    clear.color = (VkClearColorValue){{0.0F, 0.0F, 0.0F, 1.0F}};

    VkRenderPassBeginInfo rpbi = {0};
    rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpbi.renderPass = renderpass->render_pass;
    rpbi.framebuffer = renderpass->framebuffers[image_index];
    rpbi.renderArea.offset = (VkOffset2D){0, 0};
    rpbi.renderArea.extent = swapchain->extent;
    rpbi.clearValueCount = 1, rpbi.pClearValues = &clear;

    vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);

    VkViewport viewport = {0};
    viewport.x = 0.0F;
    viewport.y = 0.0F;
    viewport.width = (float)swapchain->extent.width;
    viewport.height = (float)swapchain->extent.height;
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;

    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = swapchain->extent;

    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdDraw(cmd, 3, 1, 0, 0);

    vkCmdEndRenderPass(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    return true;
}
