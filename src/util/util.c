#include "util/util.h"

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
