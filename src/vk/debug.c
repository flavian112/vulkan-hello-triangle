#include "vk/debug.h"

#include "util/log.h"

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

const char *vk_res_str(VkResult res) {
    switch (res) {
    case VK_SUCCESS:
        return "success";
    case VK_NOT_READY:
        return "not ready";
    case VK_TIMEOUT:
        return "timeout";
    case VK_EVENT_SET:
        return "event set";
    case VK_EVENT_RESET:
        return "event reset";
    case VK_INCOMPLETE:
        return "incomplete";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "error out of host memory";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "error out of device memory";
    case VK_ERROR_INITIALIZATION_FAILED:
        return "error initialization failed";
    case VK_ERROR_DEVICE_LOST:
        return "error device lost";
    case VK_ERROR_MEMORY_MAP_FAILED:
        return "error memory map failed";
    case VK_ERROR_LAYER_NOT_PRESENT:
        return "error layer not present";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        return "error extension not present";
    case VK_ERROR_FEATURE_NOT_PRESENT:
        return "error feature not present";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return "error incompatible driver";
    case VK_ERROR_TOO_MANY_OBJECTS:
        return "error too many objects";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return "error format not supported";
    case VK_ERROR_FRAGMENTED_POOL:
        return "error fragmented pool";
    case VK_ERROR_UNKNOWN:
        return "error unknown";
    case VK_ERROR_VALIDATION_FAILED:
        return "error validation failed";
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        return "error out of pool memory";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
        return "error invalid external handle";
    case VK_ERROR_FRAGMENTATION:
        return "error fragmentation";
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
        return "error invalid opaque capture address";
    case VK_PIPELINE_COMPILE_REQUIRED:
        return "pipeline compile required";
    case VK_ERROR_NOT_PERMITTED:
        return "error not permitted";
    case VK_ERROR_SURFACE_LOST_KHR:
        return "error surface lost khr";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return "error native window in use khr";
    case VK_SUBOPTIMAL_KHR:
        return "suboptimal khr";
    case VK_ERROR_OUT_OF_DATE_KHR:
        return "error out of date khr";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return "error incompatible display khr";
    case VK_ERROR_INVALID_SHADER_NV:
        return "error invalid shader nv";
    case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
        return "error image usage not supported khr";
    case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
        return "error video picture layout not supported khr";
    case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
        return "error video profile operation not supported khr";
    case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
        return "error video profile format not supported khr";
    case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
        return "error video profile codec not supported khr";
    case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
        return "error video std version not supported khr";
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
        return "error invalid drm format modifier plane layout ext";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        return "error full screen exclusive mode lost ext";
    case VK_THREAD_IDLE_KHR:
        return "thread idle khr";
    case VK_THREAD_DONE_KHR:
        return "thread done khr";
    case VK_OPERATION_DEFERRED_KHR:
        return "operation deferred khr";
    case VK_OPERATION_NOT_DEFERRED_KHR:
        return "operation not deferred khr";
    case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:
        return "error invalid video std parameters khr";
    case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
        return "error compression exhausted ext";
    case VK_INCOMPATIBLE_SHADER_BINARY_EXT:
        return "incompatible shader binary ext";
    case VK_PIPELINE_BINARY_MISSING_KHR:
        return "pipeline binary missing khr";
    case VK_ERROR_NOT_ENOUGH_SPACE_KHR:
        return "error not enough space khr";
    default:
        return "unknown";
    }
}
