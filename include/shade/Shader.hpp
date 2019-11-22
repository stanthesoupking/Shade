#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace Shade
{
enum ShaderVariableType
{
    FLOAT,
    INT,
    VEC2
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
    VkDevice device;
    VkPipeline graphicsPipeline;
    VkPipelineLayout graphicsPipelineLayout;

    ShaderLayout layout;

    static std::vector<char> readFileBytes(const char *path);

    VkShaderModule createShaderModule(std::vector<char> source);

public:
    static Shader *FromSPIRVFile(VkDevice device, VkExtent2D swapChainExtent,
                                 VkRenderPass renderPass,
                                 ShaderLayout shaderLayout,
                                 const char *vertPath, const char *fragPath);

    Shader(VkDevice device, VkExtent2D swapChainExtent, VkRenderPass renderPass,
           ShaderLayout shaderLayout,
           std::vector<char> vertSource, std::vector<char> fragSource);
    ~Shader();

    VkPipeline _getGraphicsPipeline();
};
} // namespace Shade