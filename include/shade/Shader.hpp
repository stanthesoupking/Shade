#pragma once

#include <vector>
#include <variant>

#include <vulkan/vulkan.h>

#include "./VulkanApplication.hpp"
#include "./UniformTexture.hpp"
#include "./StructuredBuffer.hpp"

namespace Shade
{
enum class ShaderStage
{
	VERTEX,
	FRAGMENT
};

struct UniformLayoutEntry
{
	uint32_t binding;
	ShaderStage stage;
	std::variant<StructuredBufferLayout, UniformTextureLayout> layout;
};

class ShaderLayout
{
	private:
	public:
		std::vector<UniformLayoutEntry> uniformsLayout;
		StructuredBufferLayout vertexLayout;

		ShaderLayout() { uniformsLayout = {}; vertexLayout = {}; }
		ShaderLayout(std::vector<UniformLayoutEntry> uniformsLayout, StructuredBufferLayout vertexLayout);
		~ShaderLayout();
};

class Shader
{
private:
	VulkanApplicationData* vulkanData;
	
	VkPipeline graphicsPipeline;
	VkPipelineLayout graphicsPipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;

	ShaderLayout shaderLayout;

	static std::vector<char> readFileBytes(const char *path);

	VkShaderModule createShaderModule(std::vector<char> source);

public:
	static Shader *FromSPIRVFile(VulkanApplication* app, 
								 ShaderLayout shaderLayout,
								 const char *vertPath, const char *fragPath);

	Shader(VulkanApplication* app, ShaderLayout shaderLayout,
		   std::vector<char> vertSource, std::vector<char> fragSource);
	~Shader();

	VkPipeline _getGraphicsPipeline();
	VkPipelineLayout _getGraphicsPipelineLayout();
	VkDescriptorSet _getNewDescriptorSet();

	ShaderLayout getShaderLayout();
};
} // namespace Shade