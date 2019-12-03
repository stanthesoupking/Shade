#pragma once

#include "./Buffer.hpp"
#include "./Shader.hpp"

namespace Shade
{
typedef std::variant<Buffer*, UniformTexture*> UniformBufferData;

class Material
{
private:
    VulkanApplicationData* vulkanData;

    Shader* shader;

	VkDescriptorSet descriptorSet;
public:
	Material(VulkanApplication* app, Shader* shader, std::vector<UniformBufferData> uniformData);
    ~Material();

    Shader* getShader();
    
    VkDescriptorSet _getDescriptorSet();
};
} // namespace Shade