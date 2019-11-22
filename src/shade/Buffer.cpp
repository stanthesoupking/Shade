#include "shade/Buffer.hpp"

#include <iostream>

using namespace Shade;

Buffer::Buffer(VkPhysicalDevice physicalDevice, VkDevice device, void *data, uint32_t size)
{
    this->physicalDevice = physicalDevice;
    this->device = device;

    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.size = size;
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
    void * mappedData;
    vkMapMemory(device, bufferMemory, 0, createInfo.size, 0, &mappedData);
    memcpy(mappedData, data, size);
    vkUnmapMemory(device, bufferMemory);
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

VkBuffer Buffer::_getVkBuffer()
{
    return this->buffer;
}