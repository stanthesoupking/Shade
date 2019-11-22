#include "shade/Shader.hpp"

#include <iostream>
#include <fstream>

using namespace Shade;

Shader *Shader::FromSPIRVFile(VkDevice device, VkExtent2D swapChainExtent,
                              VkRenderPass renderPass, ShaderLayout shaderLayout,
                              const char *vertPath, const char *fragPath)
{
    return new Shader(
        device, swapChainExtent, renderPass, shaderLayout,
        readFileBytes(vertPath),
        readFileBytes(fragPath));
}

Shader::Shader(VkDevice device, VkExtent2D swapChainExtent, VkRenderPass renderPass,
               ShaderLayout shaderLayout, std::vector<char> vertSource,
               std::vector<char> fragSource)
{
    this->device = device;
    this->layout = shaderLayout;

    // Create shader modules
    VkShaderModule vertMod = createShaderModule(vertSource);
    VkShaderModule fragMod = createShaderModule(fragSource);

    // Create graphics pipeline
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertMod;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragMod;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] =
        {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = shaderLayout.stride();
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    auto attributeDescriptions = shaderLayout._getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                          VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT |
                                          VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Note: skipped dynamic state, page 115

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                               &graphicsPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = graphicsPipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
                                  &pipelineInfo, nullptr,
                                  &graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to create graphics pipeline!");
    }

    // Cleanup shader modules
    vkDestroyShaderModule(device, vertMod, nullptr);
    vkDestroyShaderModule(device, fragMod, nullptr);
}

Shader::~Shader()
{
}

std::vector<char> Shader::readFileBytes(const char *path)
{
    std::ifstream file;
    file.open(path, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        throw std::runtime_error("Shade: Failed to open shader file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule Shader::createShaderModule(std::vector<char> source)
{
    VkShaderModuleCreateInfo modInfo = {};
    modInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    modInfo.pNext = nullptr;
    modInfo.flags = 0;
    modInfo.codeSize = source.size();
    modInfo.pCode = reinterpret_cast<const uint32_t *>(source.data());

    VkShaderModule mod;

    if (vkCreateShaderModule(device, &modInfo, nullptr, &mod) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to create shader module!");
    }

    return mod;
}

// Shader Layout Implementation

ShaderLayout::ShaderLayout(std::vector<ShaderVariableType> layout)
{
    this->layout = layout;
}

ShaderLayout::~ShaderLayout()
{
}

uint32_t ShaderLayout::getShaderVariableTypeSize(ShaderVariableType type)
{
    switch (type)
    {
    case FLOAT:
        return 4;
    case INT:
        return 4;
    case VEC2:
        return 8;
    default:
        std::runtime_error("Shade: Unknown variable type in shader layout.");
        break;
    }
}

VkFormat ShaderLayout::getShaderVariableTypeFormat(ShaderVariableType type)
{
    switch (type)
    {
    case FLOAT:
        return VK_FORMAT_R32_SFLOAT;
    case INT:
        return VK_FORMAT_R32_SINT;
    case VEC2:
        return VK_FORMAT_R32G32_SFLOAT;
    default:
        std::runtime_error("Shade: Unknown variable type in shader layout.");
        break;
    }
}

uint32_t ShaderLayout::stride()
{
    uint32_t stride = 0;

    for (const auto type : layout)
    {
        stride += getShaderVariableTypeSize(type);
    }

    return stride;
}

std::vector<VkVertexInputAttributeDescription> ShaderLayout::_getAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> descriptions(layout.size());

    uint32_t i = 0;
    uint32_t offset = 0;
    for (const auto type : layout)
    {
        descriptions[i].binding = 0;
        descriptions[i].location = 0;
        descriptions[i].format = getShaderVariableTypeFormat(type);

        // TODO: Check if offset calculation is correct.
        //  e.g. is it on a bit-basis or element index basis?
        descriptions[i].offset = offset;

        i++;
        offset += getShaderVariableTypeSize(type);
    }

    return descriptions;
}

VkPipeline Shader::_getGraphicsPipeline()
{
    return this->graphicsPipeline;
}