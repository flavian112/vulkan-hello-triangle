#pragma once

#include <stdlib.h>

#include <vulkan/vulkan.h>

#include "log.h"

#define VK_CHECK(x)                                                                                                    \
    do {                                                                                                               \
        VkResult _r = (x);                                                                                             \
        if (_r != VK_SUCCESS) {                                                                                        \
            log_error("VULKAN %s", vk_res_str(_r));                                                                    \
            exit(1);                                                                                                   \
        }                                                                                                              \
    } while (0)

const char *vk_res_str(VkResult res);

uint32_t clamp_u32(uint32_t n, uint32_t min, uint32_t max);
