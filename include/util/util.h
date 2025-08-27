#pragma once

#include <vulkan/vulkan.h>

#define VK_CHECK(x)                                                                                                    \
    do {                                                                                                               \
        VkResult _r = (x);                                                                                             \
        if (_r != VK_SUCCESS) {                                                                                        \
            log_error("VULKAN %s", vk_res_str(_r));                                                                    \
            exit(1);                                                                                                   \
        }                                                                                                              \
    } while (0)

const char *vk_res_str(VkResult res);
