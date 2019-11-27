#pragma once

#include "./Buffer.hpp"
#include "./Shader.hpp"

namespace Shade
{


class Material
{
private:
    VulkanApplicationData* vulkanData;

    Shader* shader;

	VkDescriptorSet descriptorSet;
public:
	Material(VulkanApplication* app, Shader* shader, std::vector<std::variant<Buffer*, UniformTexture*>> uniformData);
    ~Material();

    Shader* getShader();
    
    VkDescriptorSet _getDescriptorSet();
};
} // namespace Shade