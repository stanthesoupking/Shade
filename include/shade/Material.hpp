#pragma once

#include "./Buffer.hpp"
#include "./Shader.hpp"

namespace Shade
{
class Material
{
private:
    VulkanApplicationData* vulkanData;

	Buffer* uniformBuffer;
    Shader* shader;

	VkDescriptorSet descriptorSet;
public:
	Material(VulkanApplication* app, Shader* shader, std::vector<Buffer*> uniformBuffers);
    ~Material();

    Shader* getShader();
	Buffer* getUniformBuffer();
    
    VkDescriptorSet _getDescriptorSet();
};
} // namespace Shade