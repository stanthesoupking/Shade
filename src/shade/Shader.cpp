#include "shade/Shader.hpp"

#include <iostream>
#include <fstream>

using namespace Shade;

Shader *Shader::FromSPIRVFile(VulkanApplication* app,
							  StructuredBufferLayout*uniformLayout,
							  StructuredBufferLayout*vertexLayout,
							  const char *vertPath, const char *fragPath)
{
	return new Shader(
		app,
		uniformLayout, vertexLayout,
		readFileBytes(vertPath),
		readFileBytes(fragPath));
}

Shader::Shader(VulkanApplication* app, StructuredBufferLayout*uniformLayout,
			   StructuredBufferLayout*vertexLayout, std::vector<char> vertSource,
			   std::vector<char> fragSource)
{
	this->vulkanData = app->_getVulkanData();

	this->uniformLayout = uniformLayout;
	this->vertexLayout = vertexLayout;

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

	// Create vertex input binding description

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = vertexLayout->getStride();
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	auto attributeDescriptions = vertexLayout->_getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	// Create uniform input binding description
	if (uniformLayout != nullptr)
	{
		VkDescriptorSetLayoutBinding uniformLayoutBinding = {};
		uniformLayoutBinding.binding = 0;
		uniformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformLayoutBinding.descriptorCount = 1;
		uniformLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uniformLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uniformLayoutBinding;

		if (vkCreateDescriptorSetLayout(vulkanData->device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Shade: Failed to create descriptor set layout!");
		}
	}

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)vulkanData->swapChainExtent.width;
	viewport.height = (float)vulkanData->swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = vulkanData->swapChainExtent;

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
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

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

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	if (uniformLayout != nullptr)
	{
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	}
	else
	{
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
	}
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(vulkanData->device, &pipelineLayoutInfo, nullptr,
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
	pipelineInfo.renderPass = vulkanData->renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(vulkanData->device, VK_NULL_HANDLE, 1,
								  &pipelineInfo, nullptr,
								  &graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("Shade: Failed to create graphics pipeline!");
	}

	// Cleanup shader modules
	vkDestroyShaderModule(vulkanData->device, vertMod, nullptr);
	vkDestroyShaderModule(vulkanData->device, fragMod, nullptr);
}

Shader::~Shader()
{
	vkDestroyDescriptorSetLayout(vulkanData->device, descriptorSetLayout, nullptr);
	vkDestroyPipelineLayout(vulkanData->device, graphicsPipelineLayout, nullptr);
	vkDestroyPipeline(vulkanData->device, graphicsPipeline, nullptr);
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

	if (vkCreateShaderModule(vulkanData->device, &modInfo, nullptr, &mod) != VK_SUCCESS)
	{
		throw std::runtime_error("Shade: Failed to create shader module!");
	}

	return mod;
}

VkPipeline Shader::_getGraphicsPipeline()
{
	return this->graphicsPipeline;
}

VkPipelineLayout Shader::_getGraphicsPipelineLayout()
{
	return this->graphicsPipelineLayout;
}

VkDescriptorSet Shader::_getNewDescriptorSet()
{
	VkDescriptorSet descriptorSet;

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.descriptorPool = vulkanData->descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	if (vkAllocateDescriptorSets(vulkanData->device, &allocInfo, &descriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("Shade: Failed to create new shader descriptor set.");
	}

	return descriptorSet;
}