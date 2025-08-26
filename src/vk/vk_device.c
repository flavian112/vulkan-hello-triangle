#include "vk/vk_device.h"

#include "log.h"
#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t graphics_family;
    uint32_t present_family;
    bool has_graphics;
    bool has_present;
} queue_indices_t;

static bool has_device_extension(VkPhysicalDevice device, const char *name) {
    assert(device != VK_NULL_HANDLE);
    assert(name != NULL);

    uint32_t count = 0;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(device, NULL, &count, NULL));
    VkExtensionProperties *props = (VkExtensionProperties *)malloc(count * sizeof(*props));
    assert(props != NULL);
    VK_CHECK(vkEnumerateDeviceExtensionProperties(device, NULL, &count, props));

    bool found = false;
    for (uint32_t i = 0; i < count; ++i) {
        if (strcmp(props[i].extensionName, name) == 0) {
            found = true;
            break;
        }
    }

    free(props);
    return found;
}

static queue_indices_t find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {
    assert(device != VK_NULL_HANDLE);
    assert(surface != VK_NULL_HANDLE);

    queue_indices_t queue = {0};
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, NULL);
    VkQueueFamilyProperties *props = (VkQueueFamilyProperties *)malloc(count * sizeof(*props));
    assert(props != NULL);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, props);

    for (uint32_t i = 0; i < count; ++i) {
        if (!queue.has_graphics && (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            queue.graphics_family = i;
            queue.has_graphics = true;
        }
        VkBool32 present = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present));
        if (!queue.has_present && present) {
            queue.present_family = i;
            queue.has_present = true;
        }
        if (queue.has_graphics && queue.has_present) {
            break;
        }
    }
    free(props);
    return queue;
}

static bool swapchain_supported(VkPhysicalDevice device, VkSurfaceKHR surface) {
    assert(device != VK_NULL_HANDLE);
    assert(surface != VK_NULL_HANDLE);

    uint32_t formats_count = 0;
    uint32_t present_modes_count = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formats_count, NULL));
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_modes_count, NULL));
    return formats_count > 0 && present_modes_count > 0;
}

static uint32_t score_device(VkPhysicalDevice device, VkSurfaceKHR surface) {
    assert(device != VK_NULL_HANDLE);
    assert(surface != VK_NULL_HANDLE);

    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures feats;
    vkGetPhysicalDeviceProperties(device, &props);
    vkGetPhysicalDeviceFeatures(device, &feats);

    if (!has_device_extension(device, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
        return 0;
    }

    queue_indices_t queue = find_queue_families(device, surface);
    if (!queue.has_graphics || !queue.has_present) {
        return 0;
    }

    if (!swapchain_supported(device, surface)) {
        return 0;
    }

    uint32_t score = 0;
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    score += props.limits.maxImageDimension2D;

    return score;
}

bool vk_device_create(vk_device_t *device, VkInstance instance, VkSurfaceKHR surface) {
    assert(device != NULL);
    assert(instance != VK_NULL_HANDLE);
    assert(surface != VK_NULL_HANDLE);

    memset(device, 0, sizeof(*device));

    uint32_t count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &count, NULL));
    assert(count > 0);
    VkPhysicalDevice *list = (VkPhysicalDevice *)malloc(count * sizeof(*list));
    assert(list != NULL);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &count, list));

    VkPhysicalDevice best_device = VK_NULL_HANDLE;
    uint32_t best_score = 0;
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t score = score_device(list[i], surface);
        if (score > best_score) {
            best_score = score;
            best_device = list[i];
        }
    }
    free(list);

    assert(best_device != VK_NULL_HANDLE);

    device->physical = best_device;

    queue_indices_t queue = find_queue_families(best_device, surface);

    float priority = 1.0F;
    VkDeviceQueueCreateInfo dqci[2];
    uint32_t dqci_count = 0;

    dqci[dqci_count] = (VkDeviceQueueCreateInfo){0};
    dqci[dqci_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    dqci[dqci_count].queueFamilyIndex = queue.graphics_family;
    dqci[dqci_count].queueCount = 1;
    dqci[dqci_count].pQueuePriorities = &priority;
    ++dqci_count;

    if (queue.present_family != queue.graphics_family) {
        dqci[dqci_count] = (VkDeviceQueueCreateInfo){0};
        dqci[dqci_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        dqci[dqci_count].queueFamilyIndex = queue.present_family;
        dqci[dqci_count].queueCount = 1;
        dqci[dqci_count].pQueuePriorities = &priority;
        ++dqci_count;
    }

    const char *portability_ext = "VK_KHR_portability_subset";
    const char *dev_exts[3];
    uint32_t dev_exts_count = 0;

    const char *swapchain_ext = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    if (!has_device_extension(device->physical, swapchain_ext)) {
        return false;
    }
    dev_exts[dev_exts_count++] = swapchain_ext;

    const char *maintenance_ext = VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME;
    if (!has_device_extension(device->physical, maintenance_ext)) {
        return false;
    }
    dev_exts[dev_exts_count++] = maintenance_ext;
    VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR pdsm1 = {0};
    pdsm1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_KHR;
    pdsm1.swapchainMaintenance1 = VK_TRUE;

    if (has_device_extension(device->physical, portability_ext)) {
        dev_exts[dev_exts_count++] = portability_ext;
    }



    VkPhysicalDeviceFeatures features = {0};
    vkGetPhysicalDeviceFeatures(device->physical, &features);

    VkDeviceCreateInfo dci = {0};
    dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.pNext = &pdsm1;
    dci.queueCreateInfoCount = dqci_count;
    dci.pQueueCreateInfos = dqci;
    dci.enabledExtensionCount = dev_exts_count;
    dci.ppEnabledExtensionNames = dev_exts;
    dci.pEnabledFeatures = &features;

    VK_CHECK(vkCreateDevice(device->physical, &dci, NULL, &device->logical));

    device->graphics_family = queue.graphics_family;
    device->present_family = queue.present_family;
    device->has_graphics = queue.has_graphics;
    device->has_present = queue.has_present;

    vkGetDeviceQueue(device->logical, device->graphics_family, 0, &device->graphics_queue);
    vkGetDeviceQueue(device->logical, device->present_family, 0, &device->present_queue);

    return true;
}

void vk_device_destroy(vk_device_t *device) {
    assert(device != NULL);

    if (device->logical != NULL) {
        vkDestroyDevice(device->logical, NULL);
        device->logical = VK_NULL_HANDLE;
    }

    memset(device, 0, sizeof(*device));
}
