#include "shade/Material.hpp"

using namespace Shade;

Material::Material(VulkanApplication* app, Shader* shader, std::vector<std::variant<Buffer*, UniformTexture*>> uniformData)
{
    this->vulkanData = app->_getVulkanData();

    this->shader = shader;

	ShaderLayout shaderLayout = shader->getShaderLayout();
    
    // Create descriptor sets
    this->descriptorSet = shader->_getNewDescriptorSet();

	int i = 0;
	std::vector<VkWriteDescriptorSet> descriptorWrites;
	for (const auto uniformEntry : shaderLayout.uniformsLayout)
	{
		if (std::holds_alternative<StructuredBufferLayout>(uniformEntry.layout))
		{
			auto uniformLayout = std::get<StructuredBufferLayout>(uniformEntry.layout);
			auto buffer = std::get<Buffer*>(uniformData.at(i));

			VkDescriptorBufferInfo bufferInfo;
			bufferInfo.buffer = buffer->_getVkBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = buffer->getTotalSize();

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = uniformEntry.binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;

			descriptorWrites.push_back(descriptorWrite);

		}
		else if (std::holds_alternative<UniformTextureLayout>(uniformEntry.layout))
		{
			auto uniformTexture = std::get<UniformTextureLayout>(uniformEntry.layout);
			auto texture = std::get<UniformTexture*>(uniformData.at(i));

			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = texture->_getTextureImageView();
			imageInfo.sampler = texture->_getTextureSampler();

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

			descriptorWrites.push_back(descriptorWrite);
		}
		i++;
	}

	vkUpdateDescriptorSets(vulkanData->device,
		static_cast<uint32_t>(descriptorWrites.size()),
		descriptorWrites.data(), 0, nullptr);
}

Material::~Material()
{
    // TODO
    //vkFreeDescriptorSets(...)
}

Shader* Material::getShader()
{
    return this->shader;
}

VkDescriptorSet Material::_getDescriptorSet()
{
    return this->descriptorSet;
}