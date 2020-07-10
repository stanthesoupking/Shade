#include "shade/Material.hpp"

#include <iostream>

using namespace Shade;

/**
 * Class constructor
 * 
 * @param app Vulkan application that the material is valid in.
 * @param shader the shader that the material links to
 * @param uniformData initial uniform data of the material
 */
Material::Material(VulkanApplication *_app, Shader *shader)
{
	app = _app;
	vulkanData = app->_getVulkanData();

	this->shader = shader;

	// Create descriptor sets
	descriptorSet = shader->_getNewDescriptorSet();

	// Create default offsets
	dynamicUniformOffsets = new std::vector<uint32_t>();

	int totalDynamicUniforms = shader->getShaderLayout().getDynamicUniformStrides(app).size();
	for(int i = 0; i < totalDynamicUniforms; i++)
	{
		dynamicUniformOffsets->push_back(0);
	}
}

/**
 * Class destructor
 * 
 * Cleans up allocated variables created by the object.
 */
Material::~Material()
{
	vkFreeDescriptorSets(vulkanData->device, vulkanData->descriptorPool, 1,
						 &descriptorSet);

	delete dynamicUniformOffsets;
}

/**
 * Get the index in the material for uniform getter and setter methods
 * 
 * @param uniformName name of the uniform to get the index of
 * @return the index of the uniform or '-1' if the uniform index could not
 *  be found.
 */
int Material::getUniformIndex(std::string uniformName)
{
	// Get shader uniform layout
	std::vector<UniformLayoutEntry> uniformLayout =
		shader->getShaderLayout().uniformsLayout;

	// Find the matching uniform
	int i = 0;
	for (const auto uniformEntry : uniformLayout)
	{
		if (!uniformEntry.name.compare(uniformName))
		{
			// Match found
			return i;
		}
		i++;
	}

	return -1;
}

/**
 * Set the buffer of the uniform at the given index.
 * 
 * @param uniformIndex index of the uniform to modify
 * @param buffer buffer to use
 */
void Material::setUniformStructuredBuffer(int uniformIndex, StructuredUniformBuffer *buffer)
{
	// Update vulkan descriptor set
	VkDescriptorBufferInfo bufferInfo;
	bufferInfo.buffer = buffer->_getVkBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = buffer->getStride();

	ShaderLayout shaderLayout = shader->getShaderLayout();
	UniformLayoutEntry uniformEntry = shaderLayout.uniformsLayout.at(uniformIndex);

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = uniformEntry.binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = buffer->getDynamic() ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = nullptr;
	descriptorWrite.pTexelBufferView = nullptr;

	// Write to GPU
	vkUpdateDescriptorSets(vulkanData->device,
						   1,
						   &descriptorWrite, 0, nullptr);
}

/**
 * Set the texture of the uniform at the given index.
 * 
 * @param uniformIndex index of the uniform to modify
 * @param texture texture to use
 */
void Material::setUniformTexture(int uniformIndex, UniformTexture *texture)
{
	// Update vulkan descriptor set
	VkDescriptorImageInfo imageInfo;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = texture->_getTextureImageView();
	imageInfo.sampler = texture->_getTextureSampler();

	ShaderLayout shaderLayout = shader->getShaderLayout();
	UniformLayoutEntry uniformEntry = shaderLayout.uniformsLayout.at(uniformIndex);

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = uniformEntry.binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pImageInfo = &imageInfo;
	descriptorWrite.pTexelBufferView = nullptr;

	// Write to GPU
	vkUpdateDescriptorSets(vulkanData->device,
						   1,
						   &descriptorWrite, 0, nullptr);
}

/**
 * Get the shader that the material is valid in.
 * 
 * @return material's linked shader
 */
Shader *Material::getShader()
{
	return this->shader;
}

/**
 * ~INTERNAL METHOD~
 *  
 *  Return vulkan descriptor sets for binding the material at render time.
 * 
 * @return the material's uniform descriptor sets
 */
VkDescriptorSet Material::_getDescriptorSet()
{
	return this->descriptorSet;
}

/**
 * Get list of dynamic uniform offsets
 */
std::vector<uint32_t> *Material::getDynamicUniformOffsets()
{
	return dynamicUniformOffsets;
}

/**
 * Get list of vulkan dynamic uniform offsets
 */
std::vector<uint32_t> Material::_getVkDynamicUniformOffsets()
{
	std::vector<uint32_t> offsets = shader->getShaderLayout().getDynamicUniformStrides(app);

	// Apply offsets
	int i = 0;
	for(auto offset : *dynamicUniformOffsets)
	{
		offsets[i++] *= offset;
	}

	return offsets;
}