#include "platform_window.h"

#include <assert.h>
#include <stdlib.h>

#include "util/log.h"
#include "vk/debug.h"

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

struct platform_window {
    GLFWwindow *glfw_window;
    bool        framebuffer_resized;
};

static void resize_framebuffer(GLFWwindow *handle, int width, int height) {
    platform_window_t *window = (platform_window_t *)glfwGetWindowUserPointer(handle);
    if (window == NULL) {
        log_error("(GLFW WINDOW) glfwGetWindowUserPointer failed.");
        return;
    }

    window->framebuffer_resized = true;
}

static void log_glfw_error(int error_code, const char *description) {
    log_error("(GLFW WINDOW) %s", description);
}

bool platform_init(void) {
    glfwSetErrorCallback(log_glfw_error);

    int res = glfwInit();

    if (res != GLFW_TRUE) {
        log_error("(GLFW WINDOW) glfwInit failed.");
        return false;
    }

    return true;
}

void platform_deinit(void) {
    glfwTerminate();
}

bool platform_window_create(
    platform_window_t **window,
    uint32_t            width,
    uint32_t            height,
    const char         *title
) {
    *window = (platform_window_t *)malloc(sizeof(platform_window_t));
    if (*window == NULL) {
        log_error("(GLFW WINDOW) malloc failed.");
        return false;
    }

    (*window)->glfw_window         = NULL;
    (*window)->framebuffer_resized = false;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow *handle = glfwCreateWindow((int)width, (int)height, title, NULL, NULL);
    if (handle == NULL) {
        log_error("(GLFW WINDOW) glfwCreateWindow failed.");
        free(*window);
        *window = NULL;
        return false;
    }

    (*window)->glfw_window = handle;

    glfwSetWindowUserPointer(handle, *window);
    glfwSetFramebufferSizeCallback(handle, resize_framebuffer);

    return true;
}

void platform_window_destroy(platform_window_t *window) {
    glfwDestroyWindow(window->glfw_window);
    free(window);
}

void platform_window_poll(const platform_window_t *window) {
    glfwPollEvents();
}

void platform_window_wait(const platform_window_t *window) {
    glfwWaitEvents();
}

bool platform_window_should_close(const platform_window_t *window) {
    return glfwWindowShouldClose(window->glfw_window) == GLFW_TRUE;
}

bool platform_window_surface_create(
    const platform_window_t *window,
    VkInstance               vk_instance,
    VkSurfaceKHR            *vk_surface
) {
    VkResult res;
    res = glfwCreateWindowSurface(vk_instance, window->glfw_window, NULL, vk_surface);
    if (res != VK_SUCCESS) {
        log_error("(GLFW WINDOW) glfwCreateWindowSurface failed (%s).", vk_res_str(res));
        return false;
    }

    return true;
}

void platform_window_framebuffer_size(
    const platform_window_t *window,
    uint32_t                *width,
    uint32_t                *height
) {
    glfwGetFramebufferSize(window->glfw_window, (int *)width, (int *)height);
}

bool platform_vulkan_supported(void) {
    return glfwVulkanSupported() == GLFW_TRUE;
}

const char **platform_required_instance_extensions(uint32_t *count) {
    return glfwGetRequiredInstanceExtensions(count);
}
