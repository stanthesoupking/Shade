#pragma once

#include "./Buffer.hpp"
#include "./Shader.hpp"

namespace Shade
{
typedef std::variant<Buffer*, UniformTexture*> UniformBufferData;

/**
 * Material implementation for the Shade engine.
 * 
 * Represents a set of uniforms that may be used to render a mesh. Materials
 *  only work with the shader that is supplied upon material creation.
 */
class Material
{
private:
    VulkanApplicationData* vulkanData;

    // Shader that this material is linked to
    Shader* shader;

    // Descriptor set used for binding the material
	VkDescriptorSet descriptorSet;
public:
    /**
     * Class constructor
     * 
     * @param app Vulkan application that the material is valid in.
     * @param shader the shader that the material links to
     * @param uniformData initial uniform data of the material
     */
	Material(VulkanApplication* app, Shader* shader, std::vector<UniformBufferData> uniformData);

    /**
     * Class destructor
     * 
     * Cleans up allocated variables created by the object.
     */
    ~Material();

    /**
     * Get the shader that the material is valid in.
     * 
     * @return material's linked shader
     */
    Shader* getShader();
    
    /**
     * ~INTERNAL METHOD~
     *  
     *  Return vulkan descriptor sets for binding the material at render time.
     * 
     * @return the material's uniform descriptor sets
     */
    VkDescriptorSet _getDescriptorSet();
};
} // namespace Shade