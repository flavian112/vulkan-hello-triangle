#include "app.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "util/util.h"
#include "vk/draw.h"

const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

bool app_create(app_t *app) {
    assert(app != NULL);
    memset(app, 0, sizeof(*app));

    if (!platform_init()) {
        log_error("APP Failed to initialize platform.");
        app_destroy(app);
        return false;
    }

    if (!platform_window_create(&app->window, 800, 600, "Vulkan Hello Triangle")) {
        log_error("APP Failed to create platform window.");
        app_destroy(app);
        return false;
    }

    if (!instance_create(&app->instance)) {
        log_error("APP Failed to create vulkan instance.");
        app_destroy(app);
        return false;
    }

    if (!platform_window_surface_create(app->window, app->instance.instance, &app->surface)) {
        log_error("APP Failed to create surface.");
        app_destroy(app);
        return false;
    }

    if (!device_create(&app->device, app->instance.instance, app->surface)) {
        log_error("APP Failed to create device.");
        app_destroy(app);
        return false;
    }

    if (!swapchain_create(&app->swapchain, &app->device, app->surface, app->window)) {
        log_error("APP Failed to create swapchain.");
        app_destroy(app);
        return false;
    }

    if (!renderpass_create(&app->renderpass, &app->device, &app->swapchain)) {
        log_error("APP Failed to create renderpass.");
        app_destroy(app);
        return false;
    }

    if (!pipeline_create(&app->pipeline, &app->device, &app->renderpass)) {
        log_error("APP Failed to create pipeline.");
        app_destroy(app);
        return false;
    }

    if (!commands_create(&app->commands, &app->device, MAX_FRAMES_IN_FLIGHT)) {
        log_error("APP Failed to create commands.");
        app_destroy(app);
        return false;
    }

    if (!sync_create(&app->sync, &app->device, MAX_FRAMES_IN_FLIGHT)) {
        log_error("APP Failed to create commands.");
        app_destroy(app);
        return false;
    }

    app->current_frame = 0;

    return true;
}

void app_run(app_t *app) {
    while (!platform_window_should_close(app->window)) {
        platform_window_poll(app->window);
        draw_result_t draw_result = draw_frame(&app->device,
                                               &app->swapchain,
                                               &app->renderpass,
                                               &app->pipeline,
                                               &app->commands,
                                               &app->sync,
                                               &app->current_frame);

        if (draw_result == VK_DRAW_NEED_RECREATE) {
            if (!swapchain_recreate(&app->swapchain, &app->device, app->surface, app->window)) {
                continue;
            }

            if (renderpass_has_format_mismatch(&app->renderpass, &app->swapchain)) {
                renderpass_destroy(&app->renderpass, &app->device);
                pipeline_destroy(&app->pipeline, &app->device);
                renderpass_create(&app->renderpass, &app->device, &app->swapchain);
                pipeline_create(&app->pipeline, &app->device, &app->renderpass);
            } else {
                renderpass_recreate_framebuffers(&app->renderpass, &app->device, &app->swapchain);
            }
        } else if (draw_result == VK_DRAW_ERROR) {
            break;
        }
    }
    VK_CHECK(vkDeviceWaitIdle(app->device.vk_device));
}

void app_destroy(app_t *app) {
    assert(app != NULL);

    if (app->device.vk_device != VK_NULL_HANDLE) {
        VK_CHECK(vkDeviceWaitIdle(app->device.vk_device));
    }

    sync_destroy(&app->sync, &app->device);
    commands_destroy(&app->commands, &app->device);
    swapchain_destroy(&app->swapchain, &app->device);
    pipeline_destroy(&app->pipeline, &app->device);
    renderpass_destroy(&app->renderpass, &app->device);
    device_destroy(&app->device);

    if (app->instance.instance != VK_NULL_HANDLE && app->surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(app->instance.instance, app->surface, NULL);
        app->surface = VK_NULL_HANDLE;
    }

    instance_destroy(&app->instance);

    if (app->window != NULL) {
        platform_window_destroy(app->window);
        app->window = NULL;
    }
    platform_deinit();
}
