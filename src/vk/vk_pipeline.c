#include "vk/vk_pipeline.h"

#include "log.h"
#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void *read_file(const char *path, size_t *size) {
    assert(path != NULL);
    assert(size != NULL);

    FILE *file = fopen(path, "rb");
    assert(file != NULL);

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    long len = ftell(file);
    if (len < 0) {
        fclose(file);
        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }

    void *data = malloc((size_t)len);
    assert(data != NULL);

    size_t n = fread(data, 1, (size_t)len, file);
    fclose(file);
    assert(n == (size_t)len);

    *size = (size_t)len;

    return data;
}

static VkShaderModule create_shader_module(VkDevice device, const void *code, size_t size) {
    VkShaderModuleCreateInfo smci = {0};
    smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    smci.codeSize = size;
    smci.pCode = (const uint32_t *)code;

    VkShaderModule mod = VK_NULL_HANDLE;

    VK_CHECK(vkCreateShaderModule(device, &smci, NULL, &mod));

    return mod;
}

bool vk_pipeline_create(vk_pipeline_t *pipeline,
                        const vk_device_t *device,
                        const vk_renderpass_t *renderpass,
                        const char *vert_spv_path,
                        const char *frag_spv_path) {
    assert(pipeline != NULL);
    assert(device != NULL);
    assert(renderpass != NULL);
    assert(vert_spv_path != NULL);
    assert(frag_spv_path != NULL);

    memset(pipeline, 0, sizeof(*pipeline));

    size_t vsize = 0;
    size_t fsize = 0;
    void *vcode = read_file(vert_spv_path, &vsize);
    assert(vcode != NULL);
    void *fcode = read_file(frag_spv_path, &fsize);
    assert(fcode != NULL);

    VkShaderModule vsm = VK_NULL_HANDLE;
    VkShaderModule fsm = VK_NULL_HANDLE;

    vsm = create_shader_module(device->logical, vcode, vsize);
    fsm = create_shader_module(device->logical, fcode, fsize);

    free(vcode);
    free(fcode);

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

    VK_CHECK(vkCreatePipelineLayout(device->logical, &plci, NULL, &pipeline->layout));

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

    VK_CHECK(vkCreateGraphicsPipelines(device->logical, VK_NULL_HANDLE, 1, &gpci, NULL, &pipeline->pipeline));

    if (fsm != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device->logical, fsm, NULL);
    }

    if (vsm != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device->logical, vsm, NULL);
    }

    return true;
}

void vk_pipeline_destroy(vk_pipeline_t *pipeline, const vk_device_t *device) {
    assert(pipeline != NULL);
    assert(device != NULL);

    if (pipeline->pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device->logical, pipeline->pipeline, NULL);
        pipeline->pipeline = VK_NULL_HANDLE;
    }

    if (pipeline->layout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device->logical, pipeline->layout, NULL);
        pipeline->layout = VK_NULL_HANDLE;
    }

    memset(pipeline, 0, sizeof(*pipeline));
}
