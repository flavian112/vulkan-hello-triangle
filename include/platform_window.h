#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

typedef struct platform_window platform_window_t;

bool platform_init(void);
void platform_deinit(void);

bool platform_window_create(platform_window_t **window, uint32_t width, uint32_t height, const char *title);
void platform_window_destroy(platform_window_t *window);

void platform_window_poll(const platform_window_t *window);
void platform_window_wait(const platform_window_t *window);
bool platform_window_should_close(const platform_window_t *window);
bool platform_window_surface_create(const platform_window_t *window, VkInstance instance, VkSurfaceKHR *surface);
void platform_window_framebuffer_size(const platform_window_t *window, uint32_t *width, uint32_t *height);

bool platform_vulkan_supported(void);
const char **platform_required_instance_extensions(uint32_t *count);
