#include "vk/swapchain.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "util/util.h"

typedef struct {
    VkSurfaceCapabilitiesKHR caps;
    uint32_t format_count;
    VkSurfaceFormatKHR *formats;
    uint32_t present_count;
    VkPresentModeKHR *present_modes;
} swapchain_support_t;

static bool
swapchain_support_query(VkPhysicalDevice physical, VkSurfaceKHR surface, swapchain_support_t *swapchain_support) {
    assert(physical != VK_NULL_HANDLE);
    assert(surface != VK_NULL_HANDLE);
    assert(swapchain_support != NULL);

    memset(swapchain_support, 0, sizeof(*swapchain_support));

    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical, surface, &swapchain_support->caps));

    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &swapchain_support->format_count, NULL));
    swapchain_support->formats =
        (VkSurfaceFormatKHR *)malloc(swapchain_support->format_count * sizeof(*swapchain_support->formats));
    assert(swapchain_support->formats != NULL);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical, surface, &swapchain_support->format_count, swapchain_support->formats));

    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical, surface, &swapchain_support->present_count, NULL));
    swapchain_support->present_modes =
        (VkPresentModeKHR *)malloc(swapchain_support->present_count * sizeof(*swapchain_support->present_modes));
    assert(swapchain_support->present_modes != NULL);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical, surface, &swapchain_support->present_count, swapchain_support->present_modes));

    return true;
}

static void swapchain_support_free(swapchain_support_t *swapchain_support) {
    assert(swapchain_support != NULL);

    free(swapchain_support->formats);
    swapchain_support->formats = NULL;
    free(swapchain_support->present_modes);
    swapchain_support->present_modes = NULL;
    memset(swapchain_support, 0, sizeof(*swapchain_support));
}

static VkSurfaceFormatKHR choose_format(const swapchain_support_t *swapchain_support) {
    assert(swapchain_support != NULL);
    assert(swapchain_support->formats != NULL);
    assert(swapchain_support->format_count > 0);

    for (uint32_t i = 0; i < swapchain_support->format_count; ++i) {
        if (swapchain_support->formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            swapchain_support->formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return swapchain_support->formats[i];
        }
    }

    return swapchain_support->formats[0];
}

static VkPresentModeKHR choose_present_mode(const swapchain_support_t *swapchain_support) {
    assert(swapchain_support != NULL);

    for (uint32_t i = 0; i < swapchain_support->present_count; ++i) {
        if (swapchain_support->present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D choose_extent(const swapchain_support_t *swapchain_support, const platform_window_t *window) {
    assert(swapchain_support != NULL);
    assert(window != NULL);

    if (swapchain_support->caps.currentExtent.width != UINT32_MAX) {
        return swapchain_support->caps.currentExtent;
    }

    uint32_t width = 0;
    uint32_t height = 0;
    platform_window_framebuffer_size(window, &width, &height);
    VkExtent2D extent = {width, height};

    if (extent.width < swapchain_support->caps.minImageExtent.width) {
        extent.width = swapchain_support->caps.minImageExtent.width;
    }
    if (extent.width > swapchain_support->caps.maxImageExtent.width) {
        extent.width = swapchain_support->caps.maxImageExtent.width;
    }
    if (extent.height < swapchain_support->caps.minImageExtent.height) {
        extent.height = swapchain_support->caps.minImageExtent.height;
    }
    if (extent.height > swapchain_support->caps.maxImageExtent.height) {
        extent.height = swapchain_support->caps.maxImageExtent.height;
    }

    return extent;
}

static bool create_image_views(swapchain_t *swapchain, const device_t *device) {
    assert(swapchain != NULL);
    assert(device != NULL);

    swapchain->image_views = (VkImageView *)malloc(swapchain->image_count * sizeof(*swapchain->image_views));
    assert(swapchain->image_views != NULL);

    for (uint32_t i = 0; i < swapchain->image_count; ++i) {
        VkImageViewCreateInfo ivci = {0};
        ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivci.image = swapchain->images[i];
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.format = swapchain->image_format;
        ivci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        ivci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        ivci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        ivci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ivci.subresourceRange.baseMipLevel = 0;
        ivci.subresourceRange.levelCount = 1;
        ivci.subresourceRange.baseArrayLayer = 0;
        ivci.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(device->vk_device, &ivci, NULL, &swapchain->image_views[i]));
    }
    return true;
}

static bool create_core(swapchain_t *swapchain,
                        const device_t *device,
                        VkSurfaceKHR surface,
                        const platform_window_t *window,
                        VkSwapchainKHR old_swapchain) {
    assert(swapchain != NULL);
    assert(device != NULL);
    assert(surface != VK_NULL_HANDLE);
    assert(window != NULL);

    swapchain_support_t swapchain_support;
    if (!swapchain_support_query(device->vk_physical_device, surface, &swapchain_support)) {
        log_error("VULKAN Failed to query swapchain support.");
        return false;
    }
    if (swapchain_support.format_count == 0 || swapchain_support.present_count == 0) {
        swapchain_support_free(&swapchain_support);
        log_error("VULKAN Swapchain has no compatible formats / present modes.");
        return false;
    }

    VkSurfaceFormatKHR format = choose_format(&swapchain_support);
    VkPresentModeKHR present_mode = choose_present_mode(&swapchain_support);
    VkExtent2D extent = choose_extent(&swapchain_support, window);

    uint32_t image_count = swapchain_support.caps.minImageCount + 1;
    if (swapchain_support.caps.maxImageCount > 0 && image_count > swapchain_support.caps.maxImageCount) {
        image_count = swapchain_support.caps.maxImageCount;
    }

    VkSwapchainCreateInfoKHR sci = {0};
    sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sci.surface = surface;
    sci.minImageCount = image_count;
    sci.imageFormat = format.format;
    sci.imageColorSpace = format.colorSpace;
    sci.imageExtent = extent;
    sci.imageArrayLayers = 1;
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sci.preTransform = swapchain_support.caps.currentTransform;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.presentMode = present_mode;
    sci.clipped = VK_TRUE;
    sci.oldSwapchain = old_swapchain;

    uint32_t queue_family[2] = {device->graphics_queue_familiy_index, device->present_queue_family_index};
    if (device->graphics_queue_familiy_index != device->present_queue_family_index) {
        sci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        sci.queueFamilyIndexCount = 2;
        sci.pQueueFamilyIndices = queue_family;
    } else {
        sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VK_CHECK(vkCreateSwapchainKHR(device->vk_device, &sci, NULL, &swapchain->handle));

    VK_CHECK(vkGetSwapchainImagesKHR(device->vk_device, swapchain->handle, &image_count, NULL));
    swapchain->images = (VkImage *)malloc(image_count * sizeof(*swapchain->images));
    assert(swapchain->images != NULL);
    VK_CHECK(vkGetSwapchainImagesKHR(device->vk_device, swapchain->handle, &image_count, swapchain->images));

    swapchain->image_count = image_count;
    swapchain->image_format = format.format;
    swapchain->extent = extent;

    bool res = create_image_views(swapchain, device);
    swapchain_support_free(&swapchain_support);
    return res;
}

bool swapchain_create(swapchain_t *swapchain,
                      const device_t *device,
                      VkSurfaceKHR surface,
                      const platform_window_t *window) {
    assert(swapchain != NULL);
    assert(device != NULL);
    assert(surface != VK_NULL_HANDLE);
    assert(window != NULL);

    memset(swapchain, 0, sizeof(*swapchain));

    return create_core(swapchain, device, surface, window, VK_NULL_HANDLE);
}

bool swapchain_recreate(swapchain_t *swapchain,
                        const device_t *device,
                        VkSurfaceKHR surface,
                        const platform_window_t *window) {
    assert(swapchain != NULL);
    assert(device != NULL);
    assert(surface != VK_NULL_HANDLE);
    assert(window != NULL);

    uint32_t width = 0;
    uint32_t height = 0;
    platform_window_framebuffer_size(window, &width, &height);
    while (width == 0 || height == 0) {
        platform_window_wait(window);
        platform_window_framebuffer_size(window, &width, &height);
    }

    VK_CHECK(vkDeviceWaitIdle(device->vk_device));

    VkSwapchainKHR old_handle = swapchain->handle;
    VkImageView *old_views = swapchain->image_views;
    VkImage *old_images = swapchain->images;
    uint32_t old_count = swapchain->image_count;

    swapchain->handle = VK_NULL_HANDLE;
    swapchain->image_views = NULL;
    swapchain->images = NULL;
    swapchain->image_count = 0;

    bool res = create_core(swapchain, device, surface, window, old_handle);

    if (old_views != NULL) {
        for (uint32_t i = 0; i < old_count; ++i) {
            vkDestroyImageView(device->vk_device, old_views[i], NULL);
        }
        free(old_views);
    }

    if (old_images != NULL) {
        free(old_images);
    }
    if (old_handle != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device->vk_device, old_handle, NULL);
    }

    return res;
}

void swapchain_destroy(swapchain_t *swapchain, const device_t *device) {
    assert(swapchain != NULL);
    assert(device != NULL);

    if (swapchain->image_views != NULL) {
        for (uint32_t i = 0; i < swapchain->image_count; ++i) {
            vkDestroyImageView(device->vk_device, swapchain->image_views[i], NULL);
        }
        free(swapchain->image_views);
        swapchain->image_views = NULL;
    }

    if (swapchain->images != NULL) {
        free(swapchain->images);
        swapchain->images = NULL;
    }

    if (swapchain->handle != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device->vk_device, swapchain->handle, NULL);
        swapchain->handle = VK_NULL_HANDLE;
    }

    memset(swapchain, 0, sizeof(*swapchain));
}
