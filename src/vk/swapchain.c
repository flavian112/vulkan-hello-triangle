#include "vk/swapchain.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "util/util.h"

typedef struct {
    VkSurfaceCapabilitiesKHR vk_surface_capabilities;
    uint32_t vk_surface_formats_count;
    VkSurfaceFormatKHR *vk_surface_formats;
    uint32_t vk_present_modes_count;
    VkPresentModeKHR *vk_present_modes;
} swapchain_support_t;

static void swapchain_support_free(swapchain_support_t *swapchain_support) {
    if (swapchain_support->vk_surface_formats != NULL) {
        free(swapchain_support->vk_surface_formats);
    }
    if (swapchain_support->vk_present_modes != NULL) {
        free(swapchain_support->vk_present_modes);
    }
    memset(swapchain_support, 0, sizeof(*swapchain_support));
}

static bool swapchain_support_query(VkPhysicalDevice vk_physical_device,
                                    VkSurfaceKHR vk_surface,
                                    swapchain_support_t *swapchain_support) {
    memset(swapchain_support, 0, sizeof(*swapchain_support));

    VkResult res;
    res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        vk_physical_device, vk_surface, &swapchain_support->vk_surface_capabilities);
    if (res != VK_SUCCESS) {
        log_error("(SWAPCHAIN) vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed (%s).", vk_res_str(res));
        return false;
    }

    res = vkGetPhysicalDeviceSurfaceFormatsKHR(
        vk_physical_device, vk_surface, &swapchain_support->vk_surface_formats_count, NULL);
    if (res != VK_SUCCESS) {
        log_error("(SWAPCHAIN) vkGetPhysicalDeviceSurfaceFormatsKHR failed (%s).", vk_res_str(res));
        return false;
    }

    if (swapchain_support->vk_surface_formats_count == 0) {
        return false;
    }

    swapchain_support->vk_surface_formats = (VkSurfaceFormatKHR *)malloc(
        swapchain_support->vk_surface_formats_count * sizeof(*swapchain_support->vk_surface_formats));
    if (swapchain_support->vk_surface_formats == NULL) {
        log_error("(SWAPCHAIN) malloc failed.");
        swapchain_support_free(swapchain_support);
        return false;
    }

    res = vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device,
                                               vk_surface,
                                               &swapchain_support->vk_surface_formats_count,
                                               swapchain_support->vk_surface_formats);
    if (res != VK_SUCCESS) {
        log_error("(SWAPCHAIN) vkGetPhysicalDeviceSurfaceFormatsKHR failed (%s).", vk_res_str(res));
        swapchain_support->vk_surface_formats = NULL;
        swapchain_support_free(swapchain_support);
        return false;
    }

    res = vkGetPhysicalDeviceSurfacePresentModesKHR(
        vk_physical_device, vk_surface, &swapchain_support->vk_present_modes_count, NULL);
    if (res != VK_SUCCESS) {
        log_error("(SWAPCHAIN) vkGetPhysicalDeviceSurfacePresentModesKHR failed (%s).", vk_res_str(res));
        swapchain_support_free(swapchain_support);
        return false;
    }

    if (swapchain_support->vk_present_modes_count == 0) {
        swapchain_support_free(swapchain_support);
        return false;
    }

    swapchain_support->vk_present_modes = (VkPresentModeKHR *)malloc(swapchain_support->vk_present_modes_count *
                                                                     sizeof(*swapchain_support->vk_present_modes));
    if (swapchain_support->vk_present_modes == NULL) {
        log_error("(SWAPCHAIN) malloc failed.");
        swapchain_support_free(swapchain_support);
        return false;
    }

    res = vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device,
                                                    vk_surface,
                                                    &swapchain_support->vk_present_modes_count,
                                                    swapchain_support->vk_present_modes);
    if (res != VK_SUCCESS) {
        log_error("(SWAPCHAIN) vkGetPhysicalDeviceSurfacePresentModesKHR failed (%s).", vk_res_str(res));
        swapchain_support->vk_present_modes = NULL;
        swapchain_support_free(swapchain_support);
        return false;
    }

    return true;
}

static VkSurfaceFormatKHR choose_format(const swapchain_support_t *swapchain_support) {
    for (uint32_t i = 0; i < swapchain_support->vk_surface_formats_count; ++i) {
        if (swapchain_support->vk_surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            swapchain_support->vk_surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return swapchain_support->vk_surface_formats[i];
        }
    }

    return swapchain_support->vk_surface_formats[0];
}

static VkPresentModeKHR choose_present_mode(const swapchain_support_t *swapchain_support) {
    for (uint32_t i = 0; i < swapchain_support->vk_present_modes_count; ++i) {
        if (swapchain_support->vk_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D choose_extent(const swapchain_support_t *swapchain_support, const platform_window_t *window) {
    if (swapchain_support->vk_surface_capabilities.currentExtent.width != UINT32_MAX) {
        return swapchain_support->vk_surface_capabilities.currentExtent;
    }

    uint32_t width = 0;
    uint32_t height = 0;
    platform_window_framebuffer_size(window, &width, &height);
    VkExtent2D extent = {width, height};

    if (extent.width < swapchain_support->vk_surface_capabilities.minImageExtent.width) {
        extent.width = swapchain_support->vk_surface_capabilities.minImageExtent.width;
    }
    if (extent.width > swapchain_support->vk_surface_capabilities.maxImageExtent.width) {
        extent.width = swapchain_support->vk_surface_capabilities.maxImageExtent.width;
    }
    if (extent.height < swapchain_support->vk_surface_capabilities.minImageExtent.height) {
        extent.height = swapchain_support->vk_surface_capabilities.minImageExtent.height;
    }
    if (extent.height > swapchain_support->vk_surface_capabilities.maxImageExtent.height) {
        extent.height = swapchain_support->vk_surface_capabilities.maxImageExtent.height;
    }

    return extent;
}

static bool create_image_views(swapchain_t *swapchain, const device_t *device) {
    swapchain->vk_image_views = (VkImageView *)malloc(swapchain->vk_image_count * sizeof(*swapchain->vk_image_views));
    assert(swapchain->vk_image_views != NULL);

    for (uint32_t i = 0; i < swapchain->vk_image_count; ++i) {
        VkImageViewCreateInfo ivci = {0};
        ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivci.image = swapchain->vk_images[i];
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.format = swapchain->vk_image_format;
        ivci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        ivci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        ivci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        ivci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ivci.subresourceRange.baseMipLevel = 0;
        ivci.subresourceRange.levelCount = 1;
        ivci.subresourceRange.baseArrayLayer = 0;
        ivci.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(device->vk_device, &ivci, NULL, &swapchain->vk_image_views[i]));
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
    if (swapchain_support.vk_surface_formats_count == 0 || swapchain_support.vk_present_modes_count == 0) {
        swapchain_support_free(&swapchain_support);
        log_error("VULKAN Swapchain has no compatible formats / present modes.");
        return false;
    }

    VkSurfaceFormatKHR format = choose_format(&swapchain_support);
    VkPresentModeKHR present_mode = choose_present_mode(&swapchain_support);
    VkExtent2D extent = choose_extent(&swapchain_support, window);

    uint32_t image_count = swapchain_support.vk_surface_capabilities.minImageCount + 1;
    if (swapchain_support.vk_surface_capabilities.maxImageCount > 0 &&
        image_count > swapchain_support.vk_surface_capabilities.maxImageCount) {
        image_count = swapchain_support.vk_surface_capabilities.maxImageCount;
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
    sci.preTransform = swapchain_support.vk_surface_capabilities.currentTransform;
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

    VK_CHECK(vkCreateSwapchainKHR(device->vk_device, &sci, NULL, &swapchain->vk_swapchain));

    VK_CHECK(vkGetSwapchainImagesKHR(device->vk_device, swapchain->vk_swapchain, &image_count, NULL));
    swapchain->vk_images = (VkImage *)malloc(image_count * sizeof(*swapchain->vk_images));
    assert(swapchain->vk_images != NULL);
    VK_CHECK(vkGetSwapchainImagesKHR(device->vk_device, swapchain->vk_swapchain, &image_count, swapchain->vk_images));

    swapchain->vk_image_count = image_count;
    swapchain->vk_image_format = format.format;
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

    VkSwapchainKHR old_handle = swapchain->vk_swapchain;
    VkImageView *old_views = swapchain->vk_image_views;
    VkImage *old_images = swapchain->vk_images;
    uint32_t old_count = swapchain->vk_image_count;

    swapchain->vk_swapchain = VK_NULL_HANDLE;
    swapchain->vk_image_views = NULL;
    swapchain->vk_images = NULL;
    swapchain->vk_image_count = 0;

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

    if (swapchain->vk_image_views != NULL) {
        for (uint32_t i = 0; i < swapchain->vk_image_count; ++i) {
            vkDestroyImageView(device->vk_device, swapchain->vk_image_views[i], NULL);
        }
        free(swapchain->vk_image_views);
        swapchain->vk_image_views = NULL;
    }

    if (swapchain->vk_images != NULL) {
        free(swapchain->vk_images);
        swapchain->vk_images = NULL;
    }

    if (swapchain->vk_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device->vk_device, swapchain->vk_swapchain, NULL);
        swapchain->vk_swapchain = VK_NULL_HANDLE;
    }

    memset(swapchain, 0, sizeof(*swapchain));
}
