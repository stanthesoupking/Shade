#include "shade/Material.hpp"

using namespace Shade;

/**
 * Class constructor
 * 
 * @param app Vulkan application that the material is valid in.
 * @param shader the shader that the material links to
 * @param uniformData initial uniform data of the material
 */
Material::Material(VulkanApplication *app, Shader *shader, std::vector<UniformBufferData> uniformData)
{
	this->vulkanData = app->_getVulkanData();

	this->shader = shader;

	ShaderLayout shaderLayout = shader->getShaderLayout();

	// Create descriptor sets
	this->descriptorSet = shader->_getNewDescriptorSet();

	// Keep track of allocated info structs
	std::vector<VkDescriptorBufferInfo *> allocBufferInfos;
	std::vector<VkDescriptorImageInfo *> allocImageInfos;

	int i = 0;
	std::vector<VkWriteDescriptorSet> descriptorWrites;
	for (const auto uniformEntry : shaderLayout.uniformsLayout)
	{
		if (std::holds_alternative<StructuredBufferLayout>(uniformEntry.layout))
		{
			auto uniformLayout = std::get<StructuredBufferLayout>(uniformEntry.layout);
			auto buffer = std::get<Buffer *>(uniformData.at(i));

			VkDescriptorBufferInfo *bufferInfo = (VkDescriptorBufferInfo *)malloc(sizeof(VkDescriptorBufferInfo));
			bufferInfo->buffer = buffer->_getVkBuffer();
			bufferInfo->offset = 0;
			bufferInfo->range = buffer->getTotalSize();

			// Keep track of allocated struct to future cleanup
			allocBufferInfos.push_back(bufferInfo);

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = uniformEntry.binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = bufferInfo;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;

			descriptorWrites.push_back(descriptorWrite);
		}
		else if (std::holds_alternative<UniformTextureLayout>(uniformEntry.layout))
		{
			auto uniformTexture = std::get<UniformTextureLayout>(uniformEntry.layout);
			auto texture = std::get<UniformTexture *>(uniformData.at(i));

			VkDescriptorImageInfo *imageInfo =
				(VkDescriptorImageInfo *)malloc(sizeof(VkDescriptorImageInfo));
			imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo->imageView = texture->_getTextureImageView();
			imageInfo->sampler = texture->_getTextureSampler();

			// Keep track of allocated struct to future cleanup
			allocImageInfos.push_back(imageInfo);

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = uniformEntry.binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = nullptr;
			descriptorWrite.pImageInfo = imageInfo;
			descriptorWrite.pTexelBufferView = nullptr;

			descriptorWrites.push_back(descriptorWrite);
		}
		i++;
	}

	vkUpdateDescriptorSets(vulkanData->device,
						   static_cast<uint32_t>(descriptorWrites.size()),
						   descriptorWrites.data(), 0, nullptr);

	// Cleanup allocated structs:

	// Cleanup Buffer Info structs
	for (auto bufferInfo : allocBufferInfos)
	{
		delete bufferInfo;
	}

	// Cleanup Image Info structs
	for (auto imageInfo : allocImageInfos)
	{
		delete imageInfo;
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