#include "vk/pipeline.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "util/shader.h"
#include "util/util.h"

static VkShaderModule create_shader_module(VkDevice device, const void *code, size_t size) {
    VkShaderModuleCreateInfo smci = {0};
    smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    smci.codeSize = size;
    smci.pCode = (const uint32_t *)code;

    VkShaderModule mod = VK_NULL_HANDLE;

    VK_CHECK(vkCreateShaderModule(device, &smci, NULL, &mod));

    return mod;
}

bool pipeline_create(pipeline_t *pipeline, const device_t *device, const renderpass_t *renderpass) {
    assert(pipeline != NULL);
    assert(device != NULL);
    assert(renderpass != NULL);

    memset(pipeline, 0, sizeof(*pipeline));

    uint32_t vsize = 0;
    const void *vdata = shader_get_vertex_spv_data(&vsize);
    VkShaderModule vsm = create_shader_module(device->vk_device, vdata, vsize);

    uint32_t fsize = 0;
    const void *fdata = shader_get_fragment_spv_data(&fsize);
    VkShaderModule fsm = create_shader_module(device->vk_device, fdata, fsize);

    VkPipelineShaderStageCreateInfo pssci[2];
    pssci[0] = (VkPipelineShaderStageCreateInfo){0};
    pssci[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pssci[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    pssci[0].module = vsm;
    pssci[0].pName = "main";
    pssci[1] = (VkPipelineShaderStageCreateInfo){0};
    pssci[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pssci[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    pssci[1].module = fsm;
    pssci[1].pName = "main";

    VkPipelineVertexInputStateCreateInfo pvisci = {0};
    pvisci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pvisci.vertexBindingDescriptionCount = 0;
    pvisci.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo piasci = {0};
    piasci.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    piasci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    piasci.primitiveRestartEnable = VK_FALSE;

    VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo pdsci = {0};
    pdsci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pdsci.dynamicStateCount = (uint32_t)(sizeof(dyn_states) / sizeof(dyn_states[0]));
    pdsci.pDynamicStates = dyn_states;

    VkPipelineViewportStateCreateInfo pvpsci = {0};
    pvpsci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pvpsci.viewportCount = 1;
    pvpsci.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo prsci = {0};
    prsci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    prsci.depthClampEnable = VK_FALSE;
    prsci.rasterizerDiscardEnable = VK_FALSE;
    prsci.polygonMode = VK_POLYGON_MODE_FILL;
    prsci.cullMode = VK_CULL_MODE_BACK_BIT;
    prsci.frontFace = VK_FRONT_FACE_CLOCKWISE;
    prsci.depthBiasEnable = VK_FALSE;
    prsci.lineWidth = 1.0F;

    VkPipelineMultisampleStateCreateInfo pmsci = {0};
    pmsci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pmsci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pmsci.sampleShadingEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo pcbsci = {0};
    pcbsci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pcbsci.logicOpEnable = VK_FALSE;
    pcbsci.attachmentCount = 1;
    pcbsci.pAttachments = &color_blend_attachment;

    VkPipelineLayoutCreateInfo plci = {0};
    plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VK_CHECK(vkCreatePipelineLayout(device->vk_device, &plci, NULL, &pipeline->layout));

    VkGraphicsPipelineCreateInfo gpci = {0};
    gpci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    gpci.stageCount = 2;
    gpci.pStages = pssci;
    gpci.pVertexInputState = &pvisci;
    gpci.pInputAssemblyState = &piasci;
    gpci.pViewportState = &pvpsci;
    gpci.pRasterizationState = &prsci;
    gpci.pMultisampleState = &pmsci;
    gpci.pDepthStencilState = NULL;
    gpci.pColorBlendState = &pcbsci;
    gpci.pDynamicState = &pdsci;
    gpci.layout = pipeline->layout;
    gpci.renderPass = renderpass->render_pass;
    gpci.subpass = 0;
    gpci.basePipelineHandle = VK_NULL_HANDLE;
    gpci.basePipelineIndex = -1;

    VK_CHECK(vkCreateGraphicsPipelines(device->vk_device, VK_NULL_HANDLE, 1, &gpci, NULL, &pipeline->pipeline));

    if (fsm != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device->vk_device, fsm, NULL);
    }

    if (vsm != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device->vk_device, vsm, NULL);
    }

    return true;
}

void pipeline_destroy(pipeline_t *pipeline, const device_t *device) {
    assert(pipeline != NULL);
    assert(device != NULL);

    if (pipeline->pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device->vk_device, pipeline->pipeline, NULL);
        pipeline->pipeline = VK_NULL_HANDLE;
    }

    if (pipeline->layout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device->vk_device, pipeline->layout, NULL);
        pipeline->layout = VK_NULL_HANDLE;
    }

    memset(pipeline, 0, sizeof(*pipeline));
}
