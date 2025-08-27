#include "vk/instance.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform_window.h"
#include "util/log.h"
#include "util/util.h"
#include "vk/debug.h"

#ifndef NDEBUG
static const bool instance_enable_validation = true;
#else
static const bool instance_enable_validation = false;
#endif

static const char *instance_required_extensions[] = {VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME,
                                                     VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME};
static const size_t instance_required_extensions_count =
    sizeof(instance_required_extensions) / sizeof(instance_required_extensions[0]);

static bool instance_has_layer(const char *name) {
    uint32_t count = 0;
    VkResult res;
    res = vkEnumerateInstanceLayerProperties(&count, NULL);
    if (res != VK_SUCCESS) {
        log_error("(INSTANCE) vkEnumerateInstanceLayerProperties failed (%s).", vk_res_str(res));
        return false;
    }

    if (count == 0) {
        return false;
    }

    VkLayerProperties *properties = (VkLayerProperties *)malloc(sizeof(*properties) * count);
    if (properties == NULL) {
        log_error("(INSTANCE) malloc failed.");
        return false;
    }

    res = vkEnumerateInstanceLayerProperties(&count, properties);
    if (res != VK_SUCCESS) {
        log_error("(INSTANCE) vkEnumerateInstanceLayerProperties failed (%s).", vk_res_str(res));
        free(properties);
        return false;
    }

    bool found = false;
    for (uint32_t i = 0; i < count; ++i) {
        if (strcmp(properties[i].layerName, name) == 0) {
            found = true;
            break;
        }
    }

    free(properties);
    return found;
}

static bool instance_has_extension(const char *name) {
    uint32_t count = 0;
    VkResult res;
    res = vkEnumerateInstanceExtensionProperties(NULL, &count, NULL);
    if (res != VK_SUCCESS) {
        log_error("(INSTANCE) vkEnumerateInstanceExtensionProperties failed (%s).", vk_res_str(res));
        return false;
    }

    if (count == 0) {
        return false;
    }

    VkExtensionProperties *properties = (VkExtensionProperties *)malloc(sizeof(*properties) * count);
    if (properties == NULL) {
        log_error("(INSTANCE) malloc failed.");
        return false;
    }

    res = vkEnumerateInstanceExtensionProperties(NULL, &count, properties);
    if (res != VK_SUCCESS) {
        log_error("(INSTANCE) vkEnumerateInstanceExtensionProperties failed (%s).", vk_res_str(res));
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

bool instance_create(instance_t *instance) {
    memset(instance, 0, sizeof(*instance));

    if (!platform_vulkan_supported()) {
        return false;
    }

    const char *validation_layer = "VK_LAYER_KHRONOS_validation";
    const bool has_validation = instance_enable_validation && instance_has_layer(validation_layer);

    uint32_t flags = 0;

    uint32_t platform_extensions_count = 0;
    const char **platform_extensions = platform_required_instance_extensions(&platform_extensions_count);
    for (uint32_t i = 0; i < platform_extensions_count; ++i) {
        if (!instance_has_extension(platform_extensions[i])) {
            log_error("(INSTANCE) platform required instance extension not available (%s).", platform_extensions[i]);
            return false;
        }
    }

    for (uint32_t i = 0; i < instance_required_extensions_count; ++i) {
        if (!instance_has_extension(instance_required_extensions[i])) {
            log_error("(INSTANCE) required instance extension not available (%s).", instance_required_extensions[i]);
            return false;
        }
    }

    const bool has_portability_enumeration_extension =
        instance_has_extension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    const bool has_debug_utils_extension = has_validation && instance_has_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    uint32_t extensions_count = platform_extensions_count + instance_required_extensions_count;
    if (has_portability_enumeration_extension) {
        ++extensions_count;
    }
    if (has_debug_utils_extension) {
        ++extensions_count;
    }

    const char *extensions[extensions_count];

    const char **exts = extensions;
    for (uint32_t i = 0; i < platform_extensions_count; ++i) {
        *(exts++) = platform_extensions[i];
    }
    for (uint32_t i = 0; i < instance_required_extensions_count; ++i) {
        *(exts++) = instance_required_extensions[i];
    }
    if (has_portability_enumeration_extension) {
        *(exts++) = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
        flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
    if (has_debug_utils_extension) {
        *(exts++) = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    uint32_t layer_count = 0;
    const char *layers[1] = {validation_layer};
    if (has_debug_utils_extension) {
        layer_count = 1;
    }

    VkApplicationInfo application_info = {0};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "Vulkan Hello Triangle";
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName = "No Engine";
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo instance_create_info = {0};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &application_info;
    instance_create_info.flags = flags;
    instance_create_info.enabledExtensionCount = extensions_count;
    instance_create_info.ppEnabledExtensionNames = extensions;
    instance_create_info.enabledLayerCount = layer_count;
    instance_create_info.ppEnabledLayerNames = layers;

    VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info = {0};
    debug_utils_messenger_set_create_info(&debug_utils_messenger_create_info);
    if (has_debug_utils_extension) {
        instance_create_info.pNext = &debug_utils_messenger_create_info;
    }

    VkResult res;
    res = vkCreateInstance(&instance_create_info, NULL, &instance->vk_instance);
    if (res != VK_SUCCESS) {
        log_error("(INSTANCE) vkCreateInstance failed (%s).", vk_res_str(res));
        instance->vk_instance = VK_NULL_HANDLE;
        return false;
    }

    if (has_debug_utils_extension) {
        if (!debug_utils_messenger_create(
                instance->vk_instance, &debug_utils_messenger_create_info, &instance->vk_debug_utils_messenger)) {
            instance->vk_debug_utils_messenger = VK_NULL_HANDLE;
            log_warn("VULKAN Failed to create debug messenger.");
        }
    }

    return true;
}

void instance_destroy(instance_t *instance) {

    if (instance->vk_debug_utils_messenger != VK_NULL_HANDLE) {
        debug_utils_messenger_destroy(instance->vk_instance, instance->vk_debug_utils_messenger);
    }

    if (instance->vk_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance->vk_instance, NULL);
    }

    memset(instance, 0, sizeof(*instance));
}
