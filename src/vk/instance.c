#include "vk/instance.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform_window.h"
#include "util/log.h"
#include "util/util.h"
#include "vk/debug.h"

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

bool instance_create(instance_t *instance) {

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

    const char *surface_ext = VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME;
    const bool has_surface_ext = has_extension(surface_ext);

    const char *get_surface_ext = VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME;
    const bool has_get_surface_ext = has_extension(get_surface_ext);

    const uint32_t exts_count = platform_exts_count + (uint32_t)has_compat_ext + (uint32_t)has_debug_ext +
                                (uint32_t)has_surface_ext + (uint32_t)has_get_surface_ext;
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
    if (has_surface_ext) {
        exts[idx++] = surface_ext;
    }
    if (has_get_surface_ext) {
        exts[idx++] = get_surface_ext;
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
    debug_utils_messenger_set_create_info(&dumci);
    if (has_debug_ext) {
        ici.pNext = &dumci;
    }

    VK_CHECK(vkCreateInstance(&ici, NULL, &instance->instance));

    if (has_debug_ext) {
        if (!debug_utils_messenger_create(instance->instance, &dumci, &instance->debug_messenger)) {
            log_warn("VULKAN Failed to create debug messenger.");
        }
    }

    return true;
}

void instance_destroy(instance_t *instance) {
    assert(instance != NULL);

    if (instance->debug_messenger != VK_NULL_HANDLE) {
        debug_utils_messenger_destroy(instance->instance, instance->debug_messenger);
        instance->debug_messenger = VK_NULL_HANDLE;
    }
    if (instance->instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance->instance, NULL);
        instance->instance = VK_NULL_HANDLE;
    }

    memset(instance, 0, sizeof(*instance));
}
