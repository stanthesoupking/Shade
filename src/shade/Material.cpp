#include "shade/Material.hpp"

using namespace Shade;

Material::Material(VulkanApplication* app, Shader* shader, std::vector<Buffer*> uniformBuffers)
{
    this->vulkanData = app->_getVulkanData();

    this->shader = shader;
	this->uniformBuffer = uniformBuffer;

	ShaderLayout shaderLayout = shader->getShaderLayout();
    
    // Create descriptor sets
    this->descriptorSet = shader->_getNewDescriptorSet();

	int i = 0;
	for (const auto uniformEntry : shaderLayout.uniformsLayout)
	{
		if (std::holds_alternative<StructuredBufferLayout>(uniformEntry.layout))
		{
			auto uniformLayout = std::get<StructuredBufferLayout>(uniformEntry.layout);

			VkDescriptorBufferInfo bufferInfo;
			bufferInfo.buffer = uniformBuffers.at(i)->_getVkBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = uniformBuffers.at(i)->getTotalSize();

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

			vkUpdateDescriptorSets(vulkanData->device, 1, &descriptorWrite, 0, nullptr);
		}
		i++;
	}
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

Buffer* Material::getUniformBuffer()
{
	return this->uniformBuffer;
}

VkDescriptorSet Material::_getDescriptorSet()
{
    return this->descriptorSet;
}