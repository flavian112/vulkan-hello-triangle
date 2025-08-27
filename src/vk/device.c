#include "vk/device.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "util/util.h"

typedef struct {
    uint32_t graphics_queue_family_index;
    uint32_t present_queue_family_index;
    bool has_graphics_queue_family;
    bool has_present_queue_family;
} queue_family_indices_t;

static const char *device_required_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                   VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME,
                                                   "VK_KHR_portability_subset"};
static const size_t device_required_extensions_count =
    sizeof(device_required_extensions) / sizeof(device_required_extensions[0]);

static bool device_has_extension(VkPhysicalDevice device, const char *name) {
    uint32_t count = 0;
    VkResult res;
    res = vkEnumerateDeviceExtensionProperties(device, NULL, &count, NULL);
    if (res != VK_SUCCESS) {
        log_error("(DEVICE) vkEnumerateDeviceExtensionProperties failed (%s).", vk_res_str(res));
        return false;
    }

    if (count == 0) {
        return false;
    }

    VkExtensionProperties *properties = (VkExtensionProperties *)malloc(count * sizeof(*properties));
    if (properties == NULL) {
        log_error("(DEVICE) malloc failed.");
        return false;
    }
    res = vkEnumerateDeviceExtensionProperties(device, NULL, &count, properties);
    if (res != VK_SUCCESS) {
        log_error("(DEVICE) vkEnumerateDeviceExtensionProperties failed (%s).", vk_res_str(res));
        free(properties);
        return false;
    }

    bool found = false;
    for (uint32_t i = 0; i < count; ++i) {
        if (strcmp(properties[i].extensionName, name) == 0) {
            found = true;
            break;
        }
    }

    free(properties);
    return found;
}

static queue_family_indices_t find_queue_families(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface) {
    queue_family_indices_t queue_family_indices = {0};

    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &count, NULL);

    if (count == 0) {
        return queue_family_indices;
    }

    VkQueueFamilyProperties *device_queue_family_properties;
    device_queue_family_properties = (VkQueueFamilyProperties *)malloc(count * sizeof(*device_queue_family_properties));
    if (device_queue_family_properties == NULL) {
        log_error("(DEVICE) malloc failed.");
        return queue_family_indices;
    }
    vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &count, device_queue_family_properties);

    for (uint32_t i = 0; i < count; ++i) {
        if (!queue_family_indices.has_graphics_queue_family) {
            if (device_queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queue_family_indices.graphics_queue_family_index = i;
                queue_family_indices.has_graphics_queue_family = true;
            }
        }

        if (!queue_family_indices.has_present_queue_family) {
            VkBool32 has_surface_support = VK_FALSE;
            VkResult res;
            res = vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device, i, vk_surface, &has_surface_support);
            if (res != VK_SUCCESS) {
                log_warn("(DEVICE) vkGetPhysicalDeviceSurfaceSupportKHR failed (%s).", vk_res_str(res));
                has_surface_support = VK_FALSE;
            }

            if (has_surface_support == VK_TRUE) {
                queue_family_indices.present_queue_family_index = i;
                queue_family_indices.has_present_queue_family = true;
            }
        }

        if (queue_family_indices.has_graphics_queue_family && queue_family_indices.has_present_queue_family) {
            break;
        }
    }

    free(device_queue_family_properties);
    return queue_family_indices;
}

static bool device_has_swapchain_support(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface) {
    uint32_t formats_count = 0;
    uint32_t present_modes_count = 0;

    VkResult res;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface, &formats_count, NULL);
    if (res != VK_SUCCESS) {
        log_warn("(DEVICE) vkGetPhysicalDeviceSurfaceFormatsKHR failed (%s).", vk_res_str(res));
        return false;
    }

    res = vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface, &present_modes_count, NULL);
    if (res != VK_SUCCESS) {
        log_warn("(DEVICE) vkGetPhysicalDeviceSurfacePresentModesKHR failed (%s).", vk_res_str(res));
        return false;
    }

    return formats_count > 0 && present_modes_count > 0;
}

static uint32_t device_score(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface) {
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(vk_physical_device, &device_properties);

    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(vk_physical_device, &device_features);

    for (size_t i = 0; i < device_required_extensions_count; ++i) {
        if (!device_has_extension(vk_physical_device, device_required_extensions[i])) {
            return 0;
        }
    }

    queue_family_indices_t queue_family_indices = find_queue_families(vk_physical_device, vk_surface);
    if (!queue_family_indices.has_graphics_queue_family) {
        return 0;
    }
    if (!queue_family_indices.has_present_queue_family) {
        return 0;
    }

    if (!device_has_swapchain_support(vk_physical_device, vk_surface)) {
        return 0;
    }

    uint32_t score = 0;
    if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    score += device_properties.limits.maxImageDimension2D;

    return score;
}

bool device_create(device_t *device, VkInstance vk_instance, VkSurfaceKHR vk_surface) {
    memset(device, 0, sizeof(*device));

    uint32_t count = 0;
    VkResult res;
    res = vkEnumeratePhysicalDevices(vk_instance, &count, NULL);
    if (res != VK_SUCCESS) {
        log_error("(DEVICE) vkEnumeratePhysicalDevices failed (%s).", vk_res_str(res));
        return false;
    }

    if (count == 0) {
        log_error("(DEVICE) No physical devices found.");
        return false;
    }

    VkPhysicalDevice *physical_device_list = (VkPhysicalDevice *)malloc(count * sizeof(*physical_device_list));
    if (physical_device_list == NULL) {
        log_error("(DEVICE) malloc failed.");
        return false;
    }
    res = vkEnumeratePhysicalDevices(vk_instance, &count, physical_device_list);
    if (res != VK_SUCCESS) {
        log_error("(DEVICE) vkEnumeratePhysicalDevices failed (%s).", vk_res_str(res));
        free(physical_device_list);
        return false;
    }

    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    uint32_t max_score = 0;
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t score = device_score(physical_device_list[i], vk_surface);
        if (score > max_score) {
            max_score = score;
            physical_device = physical_device_list[i];
        }
    }

    free(physical_device_list);

    if (physical_device == VK_NULL_HANDLE) {
        log_error("(DEVICE) No supported physical device found.");
        return false;
    }

    device->vk_physical_device = physical_device;

    queue_family_indices_t queue_family_indices = find_queue_families(device->vk_physical_device, vk_surface);

    float priority = 1.0F;
    VkDeviceQueueCreateInfo device_queue_create_infos[2];
    uint32_t device_queue_create_infos_count = 1;

    VkDeviceQueueCreateInfo *queue_create_info = device_queue_create_infos;

    *queue_create_info = (VkDeviceQueueCreateInfo){0};
    queue_create_info->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info->queueFamilyIndex = queue_family_indices.graphics_queue_family_index;
    queue_create_info->queueCount = 1;
    queue_create_info->pQueuePriorities = &priority;
    ++queue_create_info;

    uint32_t graphics_queue_familiy_index = queue_family_indices.graphics_queue_family_index;
    uint32_t present_queue_familiy_index = queue_family_indices.present_queue_family_index;

    if (graphics_queue_familiy_index != present_queue_familiy_index) {
        ++device_queue_create_infos_count;
        *queue_create_info = (VkDeviceQueueCreateInfo){0};
        queue_create_info->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info->queueFamilyIndex = queue_family_indices.present_queue_family_index;
        queue_create_info->queueCount = 1;
        queue_create_info->pQueuePriorities = &priority;
    }

    VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR swapchain_maintenance1_features = {0};
    swapchain_maintenance1_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_KHR;
    swapchain_maintenance1_features.swapchainMaintenance1 = VK_TRUE;

    VkPhysicalDeviceFeatures features = {0};

    VkDeviceCreateInfo device_create_info = {0};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &swapchain_maintenance1_features;
    device_create_info.queueCreateInfoCount = device_queue_create_infos_count;
    device_create_info.pQueueCreateInfos = device_queue_create_infos;
    device_create_info.enabledExtensionCount = device_required_extensions_count;
    device_create_info.ppEnabledExtensionNames = device_required_extensions;
    device_create_info.pEnabledFeatures = &features;

    res = vkCreateDevice(device->vk_physical_device, &device_create_info, NULL, &device->vk_device);
    if (res != VK_SUCCESS) {
        log_error("(DEVICE) vkCreateDevice failed (%s).", vk_res_str(res));
        device->vk_physical_device = VK_NULL_HANDLE;
        return false;
    }

    device->graphics_queue_familiy_index = queue_family_indices.graphics_queue_family_index;
    device->present_queue_family_index = queue_family_indices.present_queue_family_index;
    device->has_graphics_queue = queue_family_indices.has_graphics_queue_family;
    device->has_present_queue = queue_family_indices.has_present_queue_family;

    vkGetDeviceQueue(device->vk_device, device->graphics_queue_familiy_index, 0, &device->graphics_queue);
    vkGetDeviceQueue(device->vk_device, device->present_queue_family_index, 0, &device->present_queue);

    return true;
}

void device_destroy(device_t *device) {
    if (device->vk_device != NULL) {
        vkDestroyDevice(device->vk_device, NULL);
    }

    memset(device, 0, sizeof(*device));
}
