#pragma once

#include <vulkan/vulkan.h>

bool debug_utils_messenger_create(VkInstance vk_instance,
                                  const VkDebugUtilsMessengerCreateInfoEXT *vk_debug_utils_messenger_ci,
                                  VkDebugUtilsMessengerEXT *vk_debug_utils_messenger);

void debug_utils_messenger_destroy(VkInstance vk_instance, VkDebugUtilsMessengerEXT vk_debug_utils_messenger);

void debug_utils_messenger_set_create_info(VkDebugUtilsMessengerCreateInfoEXT *vk_debug_utils_messenger_ci);

const char *vk_res_str(VkResult res);
