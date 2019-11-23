#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "./VulkanApplication.hpp"

namespace Shade
{
enum ShaderVariableType
{
	FLOAT,
	INT,
	VEC2,
	VEC3,
	VEC4
};

class ShaderLayout
{
private:
	std::vector<ShaderVariableType> layout;

	uint32_t getShaderVariableTypeSize(ShaderVariableType type);
	VkFormat getShaderVariableTypeFormat(ShaderVariableType type);

public:
	ShaderLayout(std::vector<ShaderVariableType> layout = {});
	~ShaderLayout();

	uint32_t stride();
	std::vector<VkVertexInputAttributeDescription> _getAttributeDescriptions();
};

class Shader
{
private:
	VulkanApplicationData* vulkanData;
	
	VkPipeline graphicsPipeline;
	VkPipelineLayout graphicsPipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;

	ShaderLayout *uniformLayout;
	ShaderLayout *vertexLayout;

	static std::vector<char> readFileBytes(const char *path);

	VkShaderModule createShaderModule(std::vector<char> source);

public:
	static Shader *FromSPIRVFile(VulkanApplication* app, 
								 ShaderLayout *uniformLayout, ShaderLayout *vertexLayout,
								 const char *vertPath, const char *fragPath);

	Shader(VulkanApplication* app, ShaderLayout *uniformLayout,
		   ShaderLayout *vertexLayout, std::vector<char> vertSource,
		   std::vector<char> fragSource);
	~Shader();

	VkPipeline _getGraphicsPipeline();
	VkPipelineLayout _getGraphicsPipelineLayout();
	VkDescriptorSet _getNewDescriptorSet();
};
} // namespace Shade