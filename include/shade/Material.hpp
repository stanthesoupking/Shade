#pragma once

#include "./Buffer.hpp"
#include "./StructuredUniformBuffer.hpp"
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
	Material(VulkanApplication* app, Shader* shader);

    /**
     * Class destructor
     * 
     * Cleans up allocated variables created by the object.
     */
    ~Material();

    /**
     * Get the index in the material for uniform getter and setter methods
     * 
     * @param uniformName name of the uniform to get the index of
     * @return the index of the uniform or '-1' if the uniform index could not
     *  be found.
     */
    int getUniformIndex(std::string uniformName);

    /**
     * Set the buffer of the uniform at the given index.
     * 
     * @param uniformIndex index of the uniform to modify
     * @param buffer buffer to use
     */
    void setUniformStructuredBuffer(int uniformIndex, StructuredUniformBuffer* buffer);
    
    /**
     * Set the texture of the uniform at the given index.
     * 
     * @param uniformIndex index of the uniform to modify
     * @param texture texture to use
     */
    void setUniformTexture(int uniformIndex, UniformTexture* texture);

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