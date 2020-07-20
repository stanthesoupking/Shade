#pragma once

#include <string>
#include <variant>
#include <vector>

#include <vulkan/vulkan.h>

#include "./StructuredBuffer.hpp"
#include "./UniformTexture.hpp"
#include "./VulkanApplication.hpp"

namespace Shade
{
enum ShaderFlags
{
    DISABLE_DEPTH_TEST = 1,
    DISABLE_DEPTH_WRITE = 2,
    WIREFRAME = 4
};

enum ShaderStage
{
    VERTEX_BIT = 1,
    FRAGMENT_BIT = 2
};

struct UniformLayoutEntry
{
    std::string name;
    uint32_t binding;
    uint32_t stage; // Shader Stage (use ShaderStage bits)
    std::variant<StructuredBufferLayout, UniformTextureLayout> layout;
    bool dynamic = false;
};

class ShaderLayout
{
private:
public:
    std::vector<UniformLayoutEntry> uniformsLayout;
    StructuredBufferLayout vertexLayout;

    ShaderLayout()
    {
        uniformsLayout = {};
        vertexLayout = {};
    }
    ShaderLayout(std::vector<UniformLayoutEntry> uniformsLayout,
                 StructuredBufferLayout vertexLayout);
    ~ShaderLayout();

    std::vector<uint32_t> getDynamicUniformStrides(VulkanApplication *app);
};

class Shader
{
private:
    VulkanApplication *app;
    VulkanApplicationData *vulkanData;

    VkPipeline graphicsPipeline;
    VkPipelineLayout graphicsPipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;

    ShaderLayout shaderLayout;

    int shaderFlags;

    // Cached shader modules for window resize optimisation
    VkShaderModule vertexModule;
    VkShaderModule fragmentModule;

    static std::vector<char> readFileBytes(const char *path);

    VkShaderModule createShaderModule(std::vector<char> source);

    void createGraphicsPipeline();
    void destroyGraphicsPipeline();

public:
    static Shader *loadFromSPIRV(VulkanApplication *app, ShaderLayout shaderLayout,
                                 const char *vertPath, const char *fragPath, int shaderFlags = 0);

    Shader(VulkanApplication *app, ShaderLayout shaderLayout, std::vector<char> vertSource,
           std::vector<char> fragSource, int shaderFlags = 0);
    ~Shader();

    VkPipeline _getGraphicsPipeline();
    VkPipelineLayout _getGraphicsPipelineLayout();
    VkDescriptorSet _getNewDescriptorSet();
    void _recreateGraphicsPipeline();

    ShaderLayout getShaderLayout();
};
} // namespace Shade