#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

#include "device.h"
#include "renderpass.h"

typedef struct {
    VkPipelineLayout layout;
    VkPipeline pipeline;
} pipeline_t;

bool pipeline_create(pipeline_t *pipeline,
                     const device_t *device,
                     const renderpass_t *renderpass,
                     const char *vert_spv_path,
                     const char *frag_spv_path);

void pipeline_destroy(pipeline_t *pipeline, const device_t *device);
