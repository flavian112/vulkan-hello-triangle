#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "vk_device.h"
#include "vk_renderpass.h"

typedef struct {
    VkPipelineLayout layout;
    VkPipeline pipeline;
} vk_pipeline_t;

bool vk_pipeline_create(vk_pipeline_t *pipeline,
                        const vk_device_t *device,
                        const vk_renderpass_t *renderpass,
                        const char *vert_spv_path,
                        const char *frag_spv_path);

void vk_pipeline_destroy(vk_pipeline_t *pipeline, const vk_device_t *device);
