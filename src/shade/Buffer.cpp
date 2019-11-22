#include "shade/Buffer.hpp"

#include <iostream>

using namespace Shade;

Buffer::Buffer(VkPhysicalDevice physicalDevice, VkDevice device, void *data, uint32_t elementSize, uint32_t elementCount)
{
    this->physicalDevice = physicalDevice;
    this->device = device;

    createBuffer(data, elementSize, elementCount);
}

Buffer::~Buffer()
{
    vkDestroyBuffer(device, buffer, nullptr);
    vkFreeMemory(device, bufferMemory, nullptr);
}

uint32_t Buffer::findMemoryType(uint32_t typeFilter,
                                VkMemoryPropertyFlags properties)
{
    // Get available memory types
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

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

void Buffer::createBuffer(void *data, uint32_t elementSize, uint32_t elementCount)
{
    this->elementSize = elementSize;
    this->elementCount = elementCount;
    this->totalBufferSize = elementSize * elementCount;

    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.size = totalBufferSize;
    createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &createInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    // Allocate buffer memory
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    allocInfo.allocationSize = memRequirements.size;

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to allocate buffer!");
    }

    // Bind allocated memory to buffer
    vkBindBufferMemory(device, buffer, bufferMemory, 0);

    // Fill buffer
    void *mappedData;
    vkMapMemory(device, bufferMemory, 0, createInfo.size, 0, &mappedData);
    memcpy(mappedData, data, totalBufferSize);
    vkUnmapMemory(device, bufferMemory);
}

void Buffer::freeBuffer()
{
    vkDestroyBuffer(device, buffer, nullptr);
    vkFreeMemory(device, bufferMemory, nullptr);
}

VkBuffer Buffer::_getVkBuffer()
{
    return this->buffer;
}

uint32_t Buffer::getElementCount()
{
    return this->elementCount;
}

void Buffer::setData(void *data, uint32_t elementSize, uint32_t elementCount)
{
    // TODO: Optimise this so it doesn't free+alloc buffer when element count & 
    //  size are the same as previous values.

    // Free old buffer data
    freeBuffer();

    // Set new data
    createBuffer(data, elementSize, elementCount);
}