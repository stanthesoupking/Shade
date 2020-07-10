#include "shade/Buffer.hpp"

#include <iostream>

using namespace Shade;

Buffer::Buffer(VulkanApplication *app, void *data, uint32_t stride,
               uint32_t size, BufferType bufferType, BufferStorage bufferStorage)
{
    this->app = app;
    this->vulkanData = app->_getVulkanData();
    this->bufferType = bufferType;
    this->bufferStorage = bufferStorage;
    this->size = size;
    this->stride = stride;

    this->totalBufferSize = stride * size;

    createBuffer(data);
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

void Buffer::createBuffer(void *data)
{
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
    else if ((bufferType == UNIFORM) || (bufferType == DYNAMIC_UNIFORM))
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
        fillBuffer(data, size, 0);
    }
}

void Buffer::fillBuffer(void *data, uint32_t count, uint32_t offset)
{
    // Total size of data that will be modified
    int dataSize = count * stride;

    if (bufferStorage == CPU)
    {
        // Copy directly to buffer
        void *mappedData;
        vmaMapMemory(vulkanData->allocator, allocation, &mappedData);
        memcpy((char*) mappedData + (stride * offset), data, dataSize);
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
        memcpy(mappedData, data, dataSize);
        vmaUnmapMemory(vulkanData->allocator, stagingBufferAllocation);

        // Copy data from staging buffer to GPU buffer
        VkBufferCopy regions[1];
        regions[0].srcOffset = 0;
        regions[0].dstOffset = offset * stride; 
        regions[0].size = dataSize;

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
    return this->size;
}

uint32_t Buffer::getTotalSize()
{
    return this->totalBufferSize;
}

void Buffer::setData(void *data, uint32_t count, uint32_t offset)
{
    if ((stride * (count + offset)) <= totalBufferSize)
    {
        // Use original buffer
        fillBuffer(data, count, offset);
    }
    else if(offset == 0)
    {
        // Free old buffer data
        freeBuffer();

        size = count;

        // Set new data
        createBuffer(data);
    } else {
        throw std::runtime_error("Buffer resizing is not yet supported for offseted buffer-set commands.");
    }
}