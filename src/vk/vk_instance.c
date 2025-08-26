#include "vk/vk_instance.h"

#include "log.h"
#include "platform_window.h"
#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                     VkDebugUtilsMessageTypeFlagsEXT types,
                                                     const VkDebugUtilsMessengerCallbackDataEXT *cb,
                                                     void *user_data) {
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        log_error("VULKAN Validation: %s", cb->pMessage);
    } else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        log_warn("VULKAN Validation: %s", cb->pMessage);
    }
    return VK_FALSE;
}

static VkResult create_debug_utils_messenger(VkInstance inst,
                                             const VkDebugUtilsMessengerCreateInfoEXT *ci,
                                             const VkAllocationCallbacks *alloc,
                                             VkDebugUtilsMessengerEXT *out) {
    PFN_vkCreateDebugUtilsMessengerEXT fp =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(inst, "vkCreateDebugUtilsMessengerEXT");
    if (fp == NULL) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    return fp(inst, ci, alloc, out);
}

static void
destroy_debug_utils_messenger(VkInstance inst, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks *alloc) {
    PFN_vkDestroyDebugUtilsMessengerEXT fp =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(inst, "vkDestroyDebugUtilsMessengerEXT");
    if (fp == NULL) {
        return;
    }

    fp(inst, messenger, alloc);
}

static bool has_layer(const char *name) {
    uint32_t count = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&count, NULL));
    VkLayerProperties *props = (VkLayerProperties *)malloc(sizeof(*props) * count);
    assert(props != NULL);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&count, props));

    bool found = false;
    for (uint32_t i = 0; i < count; ++i) {
        if (strcmp(props[i].layerName, name) == 0) {
            found = true;
            break;
        }
    }

    free(props);
    return found;
}

static bool has_extension(const char *name) {
    uint32_t count = 0;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &count, NULL));
    VkExtensionProperties *props = (VkExtensionProperties *)malloc(sizeof(*props) * count);
    assert(props != NULL);
    VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &count, props));

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

bool vk_instance_create(vk_instance_t *instance) {

#ifndef NDEBUG
    const bool validation_requested = true;
#else
    const bool validation_requested = false;
#endif

    const char *validation_layer = "VK_LAYER_KHRONOS_validation";
    const bool has_validation = validation_requested && has_layer(validation_layer);

    if (!platform_vulkan_supported()) {
        return false;
    }

    uint32_t flags = 0;

    uint32_t platform_exts_count = 0;
    const char **platform_exts = platform_required_instance_extensions(&platform_exts_count);
    for (uint32_t i = 0; i < platform_exts_count; ++i) {
        if (!has_extension(platform_exts[i])) {
            return false;
        }
    }

    const char *compat_ext = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    const bool has_compat_ext = has_extension(compat_ext);

    const char *debug_ext = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    const bool has_debug_ext = has_validation && has_extension(debug_ext);

    const uint32_t exts_count = platform_exts_count + (uint32_t)has_compat_ext + (uint32_t)has_debug_ext;
    const char *exts[exts_count];

    uint32_t idx = 0;
    for (uint32_t i = 0; i < platform_exts_count; ++i) {
        exts[idx++] = platform_exts[i];
    }
    if (has_compat_ext) {
        exts[idx++] = compat_ext;
        flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
    if (has_debug_ext) {
        exts[idx++] = debug_ext;
    }

    uint32_t layer_count = 0;
    const char *layers[1] = {validation_layer};
    if (has_debug_ext) {
        layer_count = 1;
    }

    VkApplicationInfo ai = {0};
    ai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ai.pApplicationName = "Vulkan Hello Triangle";
    ai.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    ai.pEngineName = "No Engine";
    ai.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    ai.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo ici = {0};
    ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ici.pApplicationInfo = &ai;
    ici.flags = flags;
    ici.enabledExtensionCount = exts_count;
    ici.ppEnabledExtensionNames = exts;
    ici.enabledLayerCount = layer_count;
    ici.ppEnabledLayerNames = layers;

    VkDebugUtilsMessengerCreateInfoEXT dumci = {0};
    dumci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    dumci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    dumci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    dumci.pfnUserCallback = debug_callback;

    if (has_debug_ext) {
        ici.pNext = &dumci;
    }

    VK_CHECK(vkCreateInstance(&ici, NULL, &instance->instance));

    if (has_debug_ext) {
        instance->debug_messenger = VK_NULL_HANDLE;
        if (create_debug_utils_messenger(instance->instance, &dumci, NULL, &instance->debug_messenger) != VK_SUCCESS) {
            log_warn("VULKAN Failed to create debug messenger.");
        }
    }

    return true;
}

void vk_instance_destroy(vk_instance_t *instance) {
    assert(instance != NULL);

    if (instance->debug_messenger != VK_NULL_HANDLE) {
        destroy_debug_utils_messenger(instance->instance, instance->debug_messenger, NULL);
        instance->debug_messenger = VK_NULL_HANDLE;
    }
    if (instance->instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance->instance, NULL);
        instance->instance = VK_NULL_HANDLE;
    }

    memset(instance, 0, sizeof(*instance));
}
