#include "vk/vk_renderpass.h"

#include "log.h"
#include "util.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void destroy_framebuffers(vk_renderpass_t *renderpass, const vk_device_t *device) {
    assert(renderpass != NULL);
    assert(device != NULL);

    if (renderpass->framebuffers != NULL) {
        for (uint32_t i = 0; i < renderpass->framebuffer_count; ++i) {
            if (renderpass->framebuffers[i]) {
                vkDestroyFramebuffer(device->logical, renderpass->framebuffers[i], NULL);
            }
        }
        free(renderpass->framebuffers);
        renderpass->framebuffers = NULL;
    }
    renderpass->framebuffer_count = 0;
}

static bool
create_framebuffers(vk_renderpass_t *renderpass, const vk_device_t *device, const vk_swapchain_t *swapchain) {
    assert(renderpass != NULL);
    assert(device != NULL);
    assert(swapchain != NULL);

    destroy_framebuffers(renderpass, device);

    renderpass->framebuffer_count = swapchain->image_count;
    renderpass->framebuffers =
        (VkFramebuffer *)malloc(renderpass->framebuffer_count * sizeof(*renderpass->framebuffers));
    assert(renderpass->framebuffers != NULL);

    for (uint32_t i = 0; i < swapchain->image_count; ++i) {
        VkImageView attachments[] = {swapchain->image_views[i]};

        VkFramebufferCreateInfo fbci = {0};
        fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbci.renderPass = renderpass->render_pass;
        fbci.attachmentCount = 1;
        fbci.pAttachments = attachments;
        fbci.width = swapchain->extent.width;
        fbci.height = swapchain->extent.height;
        fbci.layers = 1;

        VK_CHECK(vkCreateFramebuffer(device->logical, &fbci, NULL, &renderpass->framebuffers[i]));
    }

    return true;
}

bool vk_renderpass_create(vk_renderpass_t *renderpass, const vk_device_t *device, const vk_swapchain_t *swapchain) {
    assert(renderpass != NULL);
    assert(device != NULL);
    assert(swapchain != NULL);

    memset(renderpass, 0, sizeof(*renderpass));

    VkAttachmentDescription attachment_desc = {0};
    attachment_desc.format = swapchain->image_format;
    attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference attachment_ref = {0};
    attachment_ref.attachment = 0;
    attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_desc = {0};
    subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc.colorAttachmentCount = 1;
    subpass_desc.pColorAttachments = &attachment_ref;

    VkSubpassDependency subpass_dep = {0};
    subpass_dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dep.dstSubpass = 0;
    subpass_dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dep.srcAccessMask = 0;
    subpass_dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo rpci = {0};
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attachment_desc;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass_desc;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &subpass_dep;

    VK_CHECK(vkCreateRenderPass(device->logical, &rpci, NULL, &renderpass->render_pass));
    renderpass->color_format = swapchain->image_format;

    if (!create_framebuffers(renderpass, device, swapchain)) {
        vkDestroyRenderPass(device->logical, renderpass->render_pass, NULL);
        memset(renderpass, 0, sizeof(*renderpass));
        return false;
    }

    return true;
}

void vk_renderpass_destroy(vk_renderpass_t *renderpass, const vk_device_t *device) {
    assert(renderpass != NULL);
    assert(device != NULL);

    destroy_framebuffers(renderpass, device);

    if (renderpass->render_pass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(device->logical, renderpass->render_pass, NULL);
        renderpass->render_pass = VK_NULL_HANDLE;
    }

    memset(renderpass, 0, sizeof(*renderpass));
}

bool vk_renderpass_recreate_framebuffers(vk_renderpass_t *renderpass,
                                         const vk_device_t *device,
                                         const vk_swapchain_t *swapchain) {
    assert(renderpass != NULL);
    assert(device != NULL);
    assert(swapchain != NULL);
    assert(renderpass->render_pass != VK_NULL_HANDLE);

    if (vk_renderpass_format_mismatch(renderpass, swapchain)) {
        return false;
    }

    return create_framebuffers(renderpass, device, swapchain);
}
