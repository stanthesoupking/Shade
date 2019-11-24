#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "./VulkanApplication.hpp"
#include "./StructuredBuffer.hpp"

namespace Shade
{

class Shader
{
private:
	VulkanApplicationData* vulkanData;
	
	VkPipeline graphicsPipeline;
	VkPipelineLayout graphicsPipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;

	StructuredBufferLayout*uniformLayout;
	StructuredBufferLayout*vertexLayout;

	static std::vector<char> readFileBytes(const char *path);

	VkShaderModule createShaderModule(std::vector<char> source);

public:
	static Shader *FromSPIRVFile(VulkanApplication* app, 
		StructuredBufferLayout*uniformLayout, StructuredBufferLayout*vertexLayout,
								 const char *vertPath, const char *fragPath);

	Shader(VulkanApplication* app, StructuredBufferLayout*uniformLayout,
		StructuredBufferLayout*vertexLayout, std::vector<char> vertSource,
		   std::vector<char> fragSource);
	~Shader();

	VkPipeline _getGraphicsPipeline();
	VkPipelineLayout _getGraphicsPipelineLayout();
	VkDescriptorSet _getNewDescriptorSet();
};
} // namespace Shade