#pragma once

#include <vector>
#include <variant>
#include <string>

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
	std::string name;
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
	VulkanApplication* app;
	VulkanApplicationData* vulkanData;
	
	VkPipeline graphicsPipeline;
	VkPipelineLayout graphicsPipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;

	ShaderLayout shaderLayout;

	// Cached shader modules for window resize optimisation
	VkShaderModule vertexModule;
	VkShaderModule fragmentModule;

	static std::vector<char> readFileBytes(const char *path);

	VkShaderModule createShaderModule(std::vector<char> source);

	void createGraphicsPipeline();
	void destroyGraphicsPipeline();

public:
	static Shader *loadFromSPIRV(VulkanApplication* app, 
								 ShaderLayout shaderLayout,
								 const char *vertPath, const char *fragPath);

	Shader(VulkanApplication* app, ShaderLayout shaderLayout,
		   std::vector<char> vertSource, std::vector<char> fragSource);
	~Shader();

	VkPipeline _getGraphicsPipeline();
	VkPipelineLayout _getGraphicsPipelineLayout();
	VkDescriptorSet _getNewDescriptorSet();
	void _recreateGraphicsPipeline();

	ShaderLayout getShaderLayout();
};
} // namespace Shade