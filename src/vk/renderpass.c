#include "vk/renderpass.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "vk/debug.h"

static void renderpass_destroy_framebuffers(renderpass_t *renderpass, const device_t *device) {
    if (renderpass->vk_framebuffers != NULL) {
        for (uint32_t i = 0; i < renderpass->vk_framebuffers_count; ++i) {
            if (renderpass->vk_framebuffers[i]) {
                vkDestroyFramebuffer(device->vk_device, renderpass->vk_framebuffers[i], NULL);
            }
        }
        free(renderpass->vk_framebuffers);
        renderpass->vk_framebuffers = NULL;
    }
    renderpass->vk_framebuffers_count = 0;
}

static bool renderpass_create_framebuffers(
    renderpass_t      *renderpass,
    const device_t    *device,
    const swapchain_t *swapchain
) {
    renderpass_destroy_framebuffers(renderpass, device);

    renderpass->vk_framebuffers_count = swapchain->vk_image_count;
    renderpass->vk_framebuffers       = (VkFramebuffer *)malloc(
        renderpass->vk_framebuffers_count * sizeof(*renderpass->vk_framebuffers)
    );
    if (renderpass->vk_framebuffers == NULL) {
        log_error("(RENDERPASS) malloc failed.");
        renderpass->vk_framebuffers_count = 0;
        return false;
    }

    for (uint32_t i = 0; i < swapchain->vk_image_count; ++i) {
        VkImageView image_view_attachments[] = {swapchain->vk_image_views[i]};

        VkFramebufferCreateInfo framebuffer_create_info = {0};
        framebuffer_create_info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass              = renderpass->vk_render_pass;
        framebuffer_create_info.attachmentCount         = 1;
        framebuffer_create_info.pAttachments            = image_view_attachments;
        framebuffer_create_info.width                   = swapchain->extent.width;
        framebuffer_create_info.height                  = swapchain->extent.height;
        framebuffer_create_info.layers                  = 1;

        VkResult res;
        res = vkCreateFramebuffer(
            device->vk_device, &framebuffer_create_info, NULL, &renderpass->vk_framebuffers[i]
        );
        if (res != VK_SUCCESS) {
            log_error("(RENDERPASS) vkCreateFramebuffer failed (%s).", vk_res_str(res));
            renderpass_destroy_framebuffers(renderpass, device);
            return false;
        }
    }

    return true;
}

bool renderpass_create(
    renderpass_t      *renderpass,
    const device_t    *device,
    const swapchain_t *swapchain
) {
    memset(renderpass, 0, sizeof(*renderpass));

    VkAttachmentDescription attachment_description = {0};
    attachment_description.format                  = swapchain->vk_image_format;
    attachment_description.samples                 = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment_description.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_description.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_description.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference attachment_reference = {0};
    attachment_reference.attachment            = 0;
    attachment_reference.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description = {0};
    subpass_description.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments    = &attachment_reference;

    VkSubpassDependency subpass_dependency = {0};
    subpass_dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass          = 0;
    subpass_dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask       = 0;
    subpass_dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_create_info = {0};
    render_pass_create_info.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount        = 1;
    render_pass_create_info.pAttachments           = &attachment_description;
    render_pass_create_info.subpassCount           = 1;
    render_pass_create_info.pSubpasses             = &subpass_description;
    render_pass_create_info.dependencyCount        = 1;
    render_pass_create_info.pDependencies          = &subpass_dependency;

    VkResult res;
    res = vkCreateRenderPass(
        device->vk_device, &render_pass_create_info, NULL, &renderpass->vk_render_pass
    );
    if (res != VK_SUCCESS) {
        renderpass->vk_render_pass = VK_NULL_HANDLE;
        log_error("(RENDERPASS) vkCreateRenderPass failed (%s).", vk_res_str(res));
        return false;
    }

    renderpass->vk_color_format = swapchain->vk_image_format;

    if (!renderpass_create_framebuffers(renderpass, device, swapchain)) {
        vkDestroyRenderPass(device->vk_device, renderpass->vk_render_pass, NULL);
        memset(renderpass, 0, sizeof(*renderpass));
        return false;
    }

    return true;
}

void renderpass_destroy(renderpass_t *renderpass, const device_t *device) {
    if (renderpass == NULL) {
        return;
    }

    renderpass_destroy_framebuffers(renderpass, device);

    if (renderpass->vk_render_pass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(device->vk_device, renderpass->vk_render_pass, NULL);
    }

    memset(renderpass, 0, sizeof(*renderpass));
}

bool renderpass_recreate_framebuffers(
    renderpass_t      *renderpass,
    const device_t    *device,
    const swapchain_t *swapchain
) {
    if (renderpass_has_format_mismatch(renderpass, swapchain)) {
        return false;
    }

    return renderpass_create_framebuffers(renderpass, device, swapchain);
}

bool renderpass_has_format_mismatch(const renderpass_t *renderpass, const swapchain_t *swapchain) {
    return renderpass->vk_render_pass != VK_NULL_HANDLE
        && renderpass->vk_color_format != swapchain->vk_image_format;
}
