#include "shade/Buffer.hpp"

#include <iostream>

using namespace Shade;

Buffer::Buffer(VulkanApplication *app, void *data, uint32_t stride, uint32_t count, BufferType bufferType)
{
    this->vulkanData = app->_getVulkanData();
    this->bufferType = bufferType;

    createBuffer(data, stride, count);
}

Buffer::~Buffer()
{
    freeBuffer();
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

    throw std::runtime_error("Shade: Failed to find suitable memory type!");
}

void Buffer::createBuffer(void *data, uint32_t stride, uint32_t count)
{
    this->stride = stride;
    this->count = count;
    this->totalBufferSize = stride * count;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0;
    bufferInfo.size = totalBufferSize;

    if (bufferType == VERTEX)
    {
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    else if (bufferType == INDEX)
    {
        bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    else if (bufferType == UNIFORM)
    {
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    else if (bufferType == TRANSFER)
    {
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }

    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    // TODO: Provide option for this
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;//VMA_MEMORY_USAGE_GPU_TO_CPU; //VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateBuffer(vulkanData->allocator, &bufferInfo, &allocInfo, &buffer,
                        &allocation, &allocationInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to create buffer!");
    }

    // Fill buffer
    void *mappedData;
    vmaMapMemory(vulkanData->allocator, allocation, &mappedData);
    memcpy(mappedData, data, totalBufferSize);
    vmaUnmapMemory(vulkanData->allocator, allocation);
}

void Buffer::freeBuffer()
{
    vmaDestroyBuffer(vulkanData->allocator, buffer, allocation);
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
    if (stride * count == totalBufferSize)
    {
        void *mappedData;
        vmaMapMemory(vulkanData->allocator, allocation, &mappedData);
        memcpy(mappedData, data, totalBufferSize);
        vmaUnmapMemory(vulkanData->allocator, allocation);
    }
    else
    {
        // Free old buffer data
        freeBuffer();

        // Set new data
        createBuffer(data, stride, count);
    }
}