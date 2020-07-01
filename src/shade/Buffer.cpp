#include "shade/Buffer.hpp"

#include <iostream>

using namespace Shade;

Buffer::Buffer(VulkanApplication *app, void *data, uint32_t stride,
               uint32_t count, BufferType bufferType, BufferStorage bufferStorage)
{
    this->app = app;
    this->vulkanData = app->_getVulkanData();
    this->bufferType = bufferType;
    this->bufferStorage = bufferStorage;

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

    if (bufferStorage == GPU)
    {
        bufferInfo.usage = bufferInfo.usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = (bufferStorage == GPU) ? VMA_MEMORY_USAGE_GPU_ONLY
                                             : VMA_MEMORY_USAGE_CPU_ONLY;

    if (vmaCreateBuffer(vulkanData->allocator, &bufferInfo, &allocInfo, &buffer,
                        &allocation, &allocationInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to create buffer!");
    }

    if (data != nullptr)
    {
        // Fill buffer
        fillBuffer(data);
    }
}

void Buffer::fillBuffer(void *data)
{
    if (bufferStorage == CPU)
    {
        // Copy directly to buffer
        void *mappedData;
        vmaMapMemory(vulkanData->allocator, allocation, &mappedData);
        memcpy(mappedData, data, totalBufferSize);
        vmaUnmapMemory(vulkanData->allocator, allocation);
    }
    else if (bufferStorage == GPU)
    {
        // Copy data in stages

        // Create staging buffer
        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation;
        VmaAllocationInfo stagingBufferAllocationInfo;

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.pNext = nullptr;
        bufferInfo.flags = 0;
        bufferInfo.size = totalBufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

        if (vmaCreateBuffer(vulkanData->allocator, &bufferInfo, &allocInfo,
                            &stagingBuffer, &stagingBufferAllocation,
                            &stagingBufferAllocationInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Shade: Failed to create staging buffer!");
        }

        // Fill staging buffer
        void *mappedData;
        vmaMapMemory(vulkanData->allocator, stagingBufferAllocation, &mappedData);
        memcpy(mappedData, data, totalBufferSize);
        vmaUnmapMemory(vulkanData->allocator, stagingBufferAllocation);

        // Copy data from staging buffer to GPU buffer
        VkBufferCopy regions[1];
        regions[0].srcOffset = 0;//stagingBufferAllocationInfo.offset;
        regions[0].dstOffset = 0;//allocationInfo.offset;
        regions[0].size = totalBufferSize;

        VkCommandBuffer commandBuffer = app->_beginSingleTimeCommands();
        vkCmdCopyBuffer(commandBuffer, stagingBuffer, buffer, 1, regions);
        app->_endSingleTimeCommands(commandBuffer);

        // Cleanup staging buffer
        vmaDestroyBuffer(vulkanData->allocator, stagingBuffer,
            stagingBufferAllocation);
    }
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
        fillBuffer(data);
    }
    else
    {
        // Free old buffer data
        freeBuffer();

        // Set new data
        createBuffer(data, stride, count);
    }
}