#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

#include "device.h"
#include "renderpass.h"

typedef struct {
    VkPipelineLayout vk_pipeline_layout;
    VkPipeline vk_pipeline;
} pipeline_t;

bool pipeline_create(pipeline_t *pipeline, const device_t *device, const renderpass_t *renderpass);

void pipeline_destroy(pipeline_t *pipeline, const device_t *device);
