#include "vk/debug.h"

#include "util/log.h"
#include "util/util.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                     VkDebugUtilsMessageTypeFlagsEXT types,
                                                     const VkDebugUtilsMessengerCallbackDataEXT *cb,
                                                     void *user_data) {
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        log_error("(VALIDATION) %s", cb->pMessage);
    } else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        log_warn("(VALIDATION) %s", cb->pMessage);
    } else {
        // log_debug("(VALIDATION) %s", cb->pMessage);
    }
    return VK_FALSE;
}

bool debug_utils_messenger_create(VkInstance vk_instance,
                                  const VkDebugUtilsMessengerCreateInfoEXT *vk_debug_utils_messenger_ci,
                                  VkDebugUtilsMessengerEXT *vk_debug_utils_messenger) {
    PFN_vkCreateDebugUtilsMessengerEXT fp;
    fp = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk_instance, "vkCreateDebugUtilsMessengerEXT");
    if (fp == NULL) {
        log_warn("(DEBUG) vkCreateDebugUtilsMessengerEXT not found.");
        return false;
    }

    VkResult result = fp(vk_instance, vk_debug_utils_messenger_ci, NULL, vk_debug_utils_messenger);
    if (result != VK_SUCCESS) {
        log_warn("(DEBUG) vkCreateDebugUtilsMessengerEXT failed (%s).", vk_res_str(result));
        return false;
    }

    return true;
}

void debug_utils_messenger_destroy(VkInstance vk_instance, VkDebugUtilsMessengerEXT vk_debug_utils_messenger) {
    PFN_vkDestroyDebugUtilsMessengerEXT fp;
    fp = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk_instance, "vkDestroyDebugUtilsMessengerEXT");
    if (fp == NULL) {
        log_warn("(DEBUG) vkDestroyDebugUtilsMessengerEXT not found.");
        return;
    }

    fp(vk_instance, vk_debug_utils_messenger, NULL);
}

void debug_utils_messenger_set_create_info(VkDebugUtilsMessengerCreateInfoEXT *vk_debug_utils_messenger_ci) {
    *vk_debug_utils_messenger_ci = (VkDebugUtilsMessengerCreateInfoEXT){0};
    vk_debug_utils_messenger_ci->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    vk_debug_utils_messenger_ci->messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    vk_debug_utils_messenger_ci->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    vk_debug_utils_messenger_ci->pfnUserCallback = debug_callback;
}
