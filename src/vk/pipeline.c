#include "vk/pipeline.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "util/shader.h"
#include "vk/debug.h"

static VkShaderModule
pipeline_create_shader_module(VkDevice vk_device, const void *code, size_t size) {
    VkShaderModuleCreateInfo shader_module_create_info = {0};
    shader_module_create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize = size;
    shader_module_create_info.pCode    = (const uint32_t *)code;

    VkShaderModule mod;

    VkResult res;
    res = vkCreateShaderModule(vk_device, &shader_module_create_info, NULL, &mod);
    if (res != VK_SUCCESS) {
        log_error("(PIPELINE) vkCreateShaderModule failed (%s).", vk_res_str(res));
        return VK_NULL_HANDLE;
    }

    return mod;
}

bool pipeline_create(pipeline_t *pipeline, const device_t *device, const renderpass_t *renderpass) {
    memset(pipeline, 0, sizeof(*pipeline));

    uint32_t       vertex_shader_spv_size = 0;
    const void    *vertex_shader_spv_data = shader_get_vertex_spv_data(&vertex_shader_spv_size);
    VkShaderModule vertex_shader_module   = pipeline_create_shader_module(
        device->vk_device, vertex_shader_spv_data, vertex_shader_spv_size
    );
    if (vertex_shader_module == VK_NULL_HANDLE) {
        return false;
    }

    uint32_t    fragment_shader_spv_size  = 0;
    const void *fragment_shader_spv_data  = shader_get_fragment_spv_data(&fragment_shader_spv_size);
    VkShaderModule fragment_shader_module = pipeline_create_shader_module(
        device->vk_device, fragment_shader_spv_data, fragment_shader_spv_size
    );
    if (fragment_shader_module == VK_NULL_HANDLE) {
        vkDestroyShaderModule(device->vk_device, vertex_shader_module, NULL);
        return false;
    }

    VkPipelineShaderStageCreateInfo shader_stage_create_infos[2];
    shader_stage_create_infos[0]        = (VkPipelineShaderStageCreateInfo){0};
    shader_stage_create_infos[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_create_infos[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stage_create_infos[0].module = vertex_shader_module;
    shader_stage_create_infos[0].pName  = "main";
    shader_stage_create_infos[1]        = (VkPipelineShaderStageCreateInfo){0};
    shader_stage_create_infos[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_create_infos[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stage_create_infos[1].module = fragment_shader_module;
    shader_stage_create_infos[1].pName  = "main";

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {0};
    vertex_input_state_create_info.sType
        = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.vertexBindingDescriptionCount   = 0;
    vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {0};
    input_assembly_state_create_info.sType
        = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state_create_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info = {0};
    pipeline_dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipeline_dynamic_state_create_info.dynamicStateCount
        = (uint32_t)(sizeof(dynamic_states) / sizeof(dynamic_states[0]));
    pipeline_dynamic_state_create_info.pDynamicStates = dynamic_states;

    VkPipelineViewportStateCreateInfo viewport_state_create_info = {0};
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.scissorCount  = 1;

    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {0};
    rasterization_state_create_info.sType
        = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state_create_info.depthClampEnable        = VK_FALSE;
    rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_create_info.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterization_state_create_info.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterization_state_create_info.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterization_state_create_info.depthBiasEnable         = VK_FALSE;
    rasterization_state_create_info.lineWidth               = 1.0F;

    VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {0};
    multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_state_create_info.sampleShadingEnable  = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {0};
    color_blend_attachment_state.colorWriteMask
        = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
        | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment_state.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {0};
    color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.logicOpEnable   = VK_FALSE;
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments    = &color_blend_attachment_state;

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {0};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkResult res;
    res = vkCreatePipelineLayout(
        device->vk_device, &pipeline_layout_create_info, NULL, &pipeline->vk_pipeline_layout
    );
    if (res != VK_SUCCESS) {
        log_error("(PIPELINE) vkCreatePipelineLayout failed (%s).", vk_res_str(res));
        pipeline->vk_pipeline_layout = VK_NULL_HANDLE;
        vkDestroyShaderModule(device->vk_device, vertex_shader_module, NULL);
        vkDestroyShaderModule(device->vk_device, fragment_shader_module, NULL);
        return false;
    }

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {0};
    graphics_pipeline_create_info.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_create_info.stageCount = 2;
    graphics_pipeline_create_info.pStages    = shader_stage_create_infos;
    graphics_pipeline_create_info.pVertexInputState   = &vertex_input_state_create_info;
    graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
    graphics_pipeline_create_info.pViewportState      = &viewport_state_create_info;
    graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
    graphics_pipeline_create_info.pMultisampleState   = &multisample_state_create_info;
    graphics_pipeline_create_info.pDepthStencilState  = NULL;
    graphics_pipeline_create_info.pColorBlendState    = &color_blend_state_create_info;
    graphics_pipeline_create_info.pDynamicState       = &pipeline_dynamic_state_create_info;
    graphics_pipeline_create_info.layout              = pipeline->vk_pipeline_layout;
    graphics_pipeline_create_info.renderPass          = renderpass->vk_render_pass;
    graphics_pipeline_create_info.subpass             = 0;
    graphics_pipeline_create_info.basePipelineHandle  = VK_NULL_HANDLE;
    graphics_pipeline_create_info.basePipelineIndex   = -1;

    res = vkCreateGraphicsPipelines(
        device->vk_device,
        VK_NULL_HANDLE,
        1,
        &graphics_pipeline_create_info,
        NULL,
        &pipeline->vk_pipeline
    );
    if (res != VK_SUCCESS) {
        log_error("(PIPELINE) vkCreateGraphicsPipelines failed (%s).", vk_res_str(res));
        pipeline->vk_pipeline = VK_NULL_HANDLE;
        vkDestroyShaderModule(device->vk_device, vertex_shader_module, NULL);
        vkDestroyShaderModule(device->vk_device, fragment_shader_module, NULL);
        pipeline_destroy(pipeline, device);
        return false;
    }

    vkDestroyShaderModule(device->vk_device, vertex_shader_module, NULL);
    vkDestroyShaderModule(device->vk_device, fragment_shader_module, NULL);

    return true;
}

void pipeline_destroy(pipeline_t *pipeline, const device_t *device) {
    if (pipeline == NULL) {
        return;
    }

    if (pipeline->vk_pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device->vk_device, pipeline->vk_pipeline, NULL);
    }

    if (pipeline->vk_pipeline_layout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device->vk_device, pipeline->vk_pipeline_layout, NULL);
    }

    memset(pipeline, 0, sizeof(*pipeline));
}
