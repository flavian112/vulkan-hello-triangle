#include "platform_window.h"

#include <assert.h>
#include <stdlib.h>

#include "util/log.h"
#include "util/util.h"

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

struct platform_window {
    GLFWwindow *handle;
    bool framebuffer_resized;
};

static void resize_framebuffer(GLFWwindow *handle, int width, int height) {
    platform_window_t *window = (platform_window_t *)glfwGetWindowUserPointer(handle);
    if (window != NULL) {
        window->framebuffer_resized = true;
    }
}

static void log_glfw_error(int error_code, const char *description) {
    log_error("GLFW %s", description);
}

bool platform_init(void) {
    glfwSetErrorCallback(log_glfw_error);
    return glfwInit() == GLFW_TRUE;
}

void platform_deinit(void) {
    glfwTerminate();
}

bool platform_window_create(platform_window_t **window, uint32_t width, uint32_t height, const char *title) {
    assert(window != NULL);
    assert(title != NULL);

    *window = NULL;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *handle = glfwCreateWindow((int)width, (int)height, title, NULL, NULL);
    if (handle == NULL) {
        return false;
    }

    glfwSetWindowUserPointer(handle, *window);
    glfwSetFramebufferSizeCallback(handle, resize_framebuffer);

    *window = (platform_window_t *)malloc(sizeof(platform_window_t));
    assert(*window != NULL);

    (*window)->handle = handle;
    (*window)->framebuffer_resized = false;

    return true;
}

void platform_window_destroy(platform_window_t *window) {
    assert(window != NULL);
    assert(window->handle != NULL);

    glfwDestroyWindow(window->handle);
    window->handle = NULL;
    free(window);
}

void platform_window_poll(const platform_window_t *window) {
    glfwPollEvents();
}

void platform_window_wait(const platform_window_t *window) {
    glfwWaitEvents();
}

bool platform_window_should_close(const platform_window_t *window) {
    assert(window != NULL);
    assert(window->handle != NULL);

    return glfwWindowShouldClose(window->handle) == GLFW_TRUE;
}

bool platform_window_surface_create(const platform_window_t *window, VkInstance instance, VkSurfaceKHR *surface) {
    assert(window != NULL);
    assert(window->handle != NULL);
    assert(instance != VK_NULL_HANDLE);
    assert(surface != NULL);

    VK_CHECK(glfwCreateWindowSurface(instance, window->handle, NULL, surface));

    return true;
}

void platform_window_framebuffer_size(const platform_window_t *window, uint32_t *width, uint32_t *height) {
    assert(window != NULL);
    assert(window->handle != NULL);
    assert(width != NULL);
    assert(height != NULL);

    glfwGetFramebufferSize(window->handle, (int *)width, (int *)height);
}

bool platform_vulkan_supported(void) {
    return glfwVulkanSupported() == GLFW_TRUE;
}

const char **platform_required_instance_extensions(uint32_t *count) {
    return glfwGetRequiredInstanceExtensions(count);
}
