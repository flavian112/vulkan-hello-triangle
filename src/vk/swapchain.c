#include "vk/swapchain.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "vk/debug.h"

typedef struct {
    VkSurfaceCapabilitiesKHR vk_surface_capabilities;
    uint32_t vk_surface_formats_count;
    VkSurfaceFormatKHR *vk_surface_formats;
    uint32_t vk_present_modes_count;
    VkPresentModeKHR *vk_present_modes;
} swapchain_support_t;

static void swapchain_support_destroy(swapchain_support_t *swapchain_support) {
    if (swapchain_support->vk_surface_formats != NULL) {
        free(swapchain_support->vk_surface_formats);
    }
    if (swapchain_support->vk_present_modes != NULL) {
        free(swapchain_support->vk_present_modes);
    }
    memset(swapchain_support, 0, sizeof(*swapchain_support));
}

static bool swapchain_support_create(VkPhysicalDevice vk_physical_device,
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
        log_error("(SWAPCHAIN) no surface formats found.");
        return false;
    }

    swapchain_support->vk_surface_formats = (VkSurfaceFormatKHR *)malloc(
        swapchain_support->vk_surface_formats_count * sizeof(*swapchain_support->vk_surface_formats));
    if (swapchain_support->vk_surface_formats == NULL) {
        log_error("(SWAPCHAIN) malloc failed.");
        swapchain_support_destroy(swapchain_support);
        return false;
    }

    res = vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device,
                                               vk_surface,
                                               &swapchain_support->vk_surface_formats_count,
                                               swapchain_support->vk_surface_formats);
    if (res != VK_SUCCESS) {
        log_error("(SWAPCHAIN) vkGetPhysicalDeviceSurfaceFormatsKHR failed (%s).", vk_res_str(res));
        swapchain_support->vk_surface_formats = NULL;
        swapchain_support_destroy(swapchain_support);
        return false;
    }

    res = vkGetPhysicalDeviceSurfacePresentModesKHR(
        vk_physical_device, vk_surface, &swapchain_support->vk_present_modes_count, NULL);
    if (res != VK_SUCCESS) {
        log_error("(SWAPCHAIN) vkGetPhysicalDeviceSurfacePresentModesKHR failed (%s).", vk_res_str(res));
        swapchain_support_destroy(swapchain_support);
        return false;
    }

    if (swapchain_support->vk_present_modes_count == 0) {
        swapchain_support_destroy(swapchain_support);
        return false;
    }

    swapchain_support->vk_present_modes = (VkPresentModeKHR *)malloc(swapchain_support->vk_present_modes_count *
                                                                     sizeof(*swapchain_support->vk_present_modes));
    if (swapchain_support->vk_present_modes == NULL) {
        log_error("(SWAPCHAIN) malloc failed.");
        swapchain_support_destroy(swapchain_support);
        return false;
    }

    res = vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device,
                                                    vk_surface,
                                                    &swapchain_support->vk_present_modes_count,
                                                    swapchain_support->vk_present_modes);
    if (res != VK_SUCCESS) {
        log_error("(SWAPCHAIN) vkGetPhysicalDeviceSurfacePresentModesKHR failed (%s).", vk_res_str(res));
        swapchain_support->vk_present_modes = NULL;
        swapchain_support_destroy(swapchain_support);
        return false;
    }

    return true;
}

static VkSurfaceFormatKHR swapchain_choose_format(const swapchain_support_t *swapchain_support) {
    assert(swapchain_support->vk_surface_formats_count > 0);
    for (uint32_t i = 0; i < swapchain_support->vk_surface_formats_count; ++i) {
        if (swapchain_support->vk_surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            swapchain_support->vk_surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return swapchain_support->vk_surface_formats[i];
        }
    }

    return swapchain_support->vk_surface_formats[0];
}

static VkPresentModeKHR swapchain_choose_present_mode(const swapchain_support_t *swapchain_support) {
    for (uint32_t i = 0; i < swapchain_support->vk_present_modes_count; ++i) {
        if (swapchain_support->vk_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D swapchain_choose_extent(const swapchain_support_t *swapchain_support,
                                          const platform_window_t *window) {
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

static bool swapchain_create_image_views(swapchain_t *swapchain, const device_t *device) {
    swapchain->vk_image_views = (VkImageView *)malloc(swapchain->vk_image_count * sizeof(*swapchain->vk_image_views));
    if (swapchain->vk_image_views == NULL) {
        log_error("(SWAPCHAIN) malloc failed.");
        return false;
    }

    for (uint32_t i = 0; i < swapchain->vk_image_count; ++i) {
        VkImageViewCreateInfo image_view_create_info = {0};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = swapchain->vk_images[i];
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = swapchain->vk_image_format;
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;

        VkResult res;
        res = vkCreateImageView(device->vk_device, &image_view_create_info, NULL, &swapchain->vk_image_views[i]);
        if (res != VK_SUCCESS) {
            log_error("(SWAPCHAIN) vkCreateImageView failed (%s).", vk_res_str(res));
            for (uint32_t j = 0; j < i; ++j) {
                vkDestroyImageView(device->vk_device, swapchain->vk_image_views[j], NULL);
                swapchain->vk_image_views[j] = VK_NULL_HANDLE;
            }
            free(swapchain->vk_image_views);
            return false;
        }
    }
    return true;
}

static bool swapchain_create_core(swapchain_t *swapchain,
                                  const device_t *device,
                                  VkSurfaceKHR vk_surface,
                                  const platform_window_t *window,
                                  VkSwapchainKHR vk_old_swapchain) {
    swapchain_support_t swapchain_support = {0};

    if (!swapchain_support_create(device->vk_physical_device, vk_surface, &swapchain_support)) {
        log_error("(SWAPCHAIN) failed to query swapchain support.");
        return false;
    }

    if (swapchain_support.vk_surface_formats_count == 0) {
        log_error("(SWAPCHAIN) no surface formats found.");
        swapchain_support_destroy(&swapchain_support);
        return false;
    }

    if (swapchain_support.vk_present_modes_count == 0) {
        log_error("(SWAPCHAIN) no surface present modes found.");
        swapchain_support_destroy(&swapchain_support);
        return false;
    }

    VkSurfaceFormatKHR format = swapchain_choose_format(&swapchain_support);
    VkPresentModeKHR present_mode = swapchain_choose_present_mode(&swapchain_support);
    VkExtent2D extent = swapchain_choose_extent(&swapchain_support, window);

    uint32_t image_count = swapchain_support.vk_surface_capabilities.minImageCount + 1;
    if (swapchain_support.vk_surface_capabilities.maxImageCount > 0) {
        if (image_count > swapchain_support.vk_surface_capabilities.maxImageCount) {
            image_count = swapchain_support.vk_surface_capabilities.maxImageCount;
        }
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = {0};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = vk_surface;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = format.format;
    swapchain_create_info.imageColorSpace = format.colorSpace;
    swapchain_create_info.imageExtent = extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.preTransform = swapchain_support.vk_surface_capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = vk_old_swapchain;

    uint32_t queue_family_indices[2] = {device->graphics_queue_familiy_index, device->present_queue_family_index};
    if (device->graphics_queue_familiy_index != device->present_queue_family_index) {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkResult res;
    res = vkCreateSwapchainKHR(device->vk_device, &swapchain_create_info, NULL, &swapchain->vk_swapchain);
    if (res != VK_SUCCESS) {
        log_error("(SWAPCHAIN) vkCreateSwapchainKHR failed (%s).", vk_res_str(res));
        swapchain_support_destroy(&swapchain_support);
        return false;
    }

    res = vkGetSwapchainImagesKHR(device->vk_device, swapchain->vk_swapchain, &image_count, NULL);
    if (res != VK_SUCCESS) {
        log_error("(SWAPCHAIN) vkGetSwapchainImagesKHR failed (%s).", vk_res_str(res));
        swapchain_support_destroy(&swapchain_support);
        return false;
    }

    if (image_count == 0) {
        log_error("(SWAPCHAIN) no images found.");
        swapchain_support_destroy(&swapchain_support);
        return false;
    }

    swapchain->vk_images = (VkImage *)malloc(image_count * sizeof(*swapchain->vk_images));
    if (swapchain->vk_images == NULL) {
        log_error("(SWAPCHAIN) malloc failed.");
        swapchain_support_destroy(&swapchain_support);
        return false;
    }

    res = vkGetSwapchainImagesKHR(device->vk_device, swapchain->vk_swapchain, &image_count, swapchain->vk_images);
    if (res != VK_SUCCESS) {
        log_error("(SWAPCHAIN) vkGetSwapchainImagesKHR failed (%s).", vk_res_str(res));
        swapchain_support_destroy(&swapchain_support);
        return false;
    }

    swapchain->vk_image_count = image_count;
    swapchain->vk_image_format = format.format;
    swapchain->extent = extent;

    bool success = swapchain_create_image_views(swapchain, device);
    swapchain_support_destroy(&swapchain_support);

    return success;
}

bool swapchain_create(swapchain_t *swapchain,
                      const device_t *device,
                      VkSurfaceKHR vk_surface,
                      const platform_window_t *window) {
    memset(swapchain, 0, sizeof(*swapchain));

    if (!swapchain_create_core(swapchain, device, vk_surface, window, VK_NULL_HANDLE)) {
        swapchain_destroy(swapchain, device);
        return false;
    }

    return true;
}

bool swapchain_recreate(swapchain_t *swapchain,
                        const device_t *device,
                        VkSurfaceKHR vk_surface,
                        const platform_window_t *window) {
    uint32_t width = 0;
    uint32_t height = 0;
    platform_window_framebuffer_size(window, &width, &height);

    while (width == 0 || height == 0) {
        platform_window_wait(window);
        platform_window_framebuffer_size(window, &width, &height);
    }

    VkResult res;
    res = vkDeviceWaitIdle(device->vk_device);
    if (res != VK_SUCCESS) {
        log_error("(SWAPCHAIN) vkDeviceWaitIdle failed (%s).", vk_res_str(res));
        swapchain_destroy(swapchain, device);
        return false;
    }

    VkSwapchainKHR old_handle = swapchain->vk_swapchain;
    VkImageView *old_views = swapchain->vk_image_views;
    VkImage *old_images = swapchain->vk_images;
    uint32_t old_count = swapchain->vk_image_count;

    swapchain->vk_swapchain = VK_NULL_HANDLE;
    swapchain->vk_image_views = NULL;
    swapchain->vk_images = NULL;
    swapchain->vk_image_count = 0;

    bool success = swapchain_create_core(swapchain, device, vk_surface, window, old_handle);

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

    return success;
}

void swapchain_destroy(swapchain_t *swapchain, const device_t *device) {
    if (swapchain == NULL) {
        return;
    }

    if (swapchain->vk_image_views != NULL) {
        for (uint32_t i = 0; i < swapchain->vk_image_count; ++i) {
            vkDestroyImageView(device->vk_device, swapchain->vk_image_views[i], NULL);
        }
        free(swapchain->vk_image_views);
    }

    if (swapchain->vk_images != NULL) {
        free(swapchain->vk_images);
    }

    if (swapchain->vk_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device->vk_device, swapchain->vk_swapchain, NULL);
    }

    memset(swapchain, 0, sizeof(*swapchain));
}
