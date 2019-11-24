#include "shade/Buffer.hpp"

#include <iostream>

using namespace Shade;

Buffer::Buffer(VulkanApplication* app, void *data, uint32_t stride, uint32_t count, BufferType bufferType)
{
    this->vulkanData = app->_getVulkanData();
    this->bufferType = bufferType;

    createBuffer(data, stride, count);
}

Buffer::~Buffer()
{
    vkDestroyBuffer(vulkanData->device, buffer, nullptr);
    vkFreeMemory(vulkanData->device, bufferMemory, nullptr);
}

uint32_t Buffer::findMemoryType(uint32_t typeFilter,
                                VkMemoryPropertyFlags properties)
{
    // Get available memory types
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vulkanData->physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

void Buffer::createBuffer(void *data, uint32_t stride, uint32_t count)
{
    this->stride = stride;
    this->count = count;
    this->totalBufferSize = stride * count;

    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.size = totalBufferSize;

    if (bufferType == VERTEX)
    {
        createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    else if (bufferType == INDEX)
    {
        createInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    else if (bufferType == UNIFORM)
    {
        createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }

    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vulkanData->device, &createInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vulkanData->device, buffer, &memRequirements);

    // Allocate buffer memory
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    allocInfo.allocationSize = memRequirements.size;

    if (vkAllocateMemory(vulkanData->device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to allocate buffer!");
    }

    // Bind allocated memory to buffer
    vkBindBufferMemory(vulkanData->device, buffer, bufferMemory, 0);

    // Fill buffer
    void *mappedData;
    vkMapMemory(vulkanData->device, bufferMemory, 0, createInfo.size, 0, &mappedData);
    memcpy(mappedData, data, totalBufferSize);
    vkUnmapMemory(vulkanData->device, bufferMemory);
}

void Buffer::freeBuffer()
{
    vkDestroyBuffer(vulkanData->device, buffer, nullptr);
    vkFreeMemory(vulkanData->device, bufferMemory, nullptr);
}

VkBuffer Buffer::_getVkBuffer()
{
    return this->buffer;
}

uint32_t Buffer::getElementCount()
{
    return this->count;
}

uint32_t Buffer::getTotalSize()
{
    return this->totalBufferSize;
}

void Buffer::setData(void *data, uint32_t stride, uint32_t count)
{
    // TODO: Optimise this so it doesn't free+alloc buffer when element count &
    //  size are the same as previous values.

    // Free old buffer data
    freeBuffer();

    // Set new data
    createBuffer(data, stride, count);
}