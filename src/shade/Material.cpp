#include "shade/Material.hpp"

using namespace Shade;

Material::Material(VulkanApplication* app, Shader* shader, Buffer* uniformBuffer)
{
    this->vulkanData = app->_getVulkanData();
    
    this->shader = shader;
	this->uniformBuffer = uniformBuffer;
    
    // Create descriptor set
    this->descriptorSet = shader->_getNewDescriptorSet();
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = uniformBuffer->_getVkBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = uniformBuffer->getTotalSize();
    
    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(vulkanData->device, 1, &descriptorWrite, 0, nullptr);
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