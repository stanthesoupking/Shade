#include "shade/Buffer.hpp"

#include <iostream>

using namespace Shade;

/**
 * Class constructor
 *
 * @param app the application instance the the buffer belongs to
 * @param data initial data to copy into the buffer
 * @param stride stride between data elements/element size in bytes
 * @param size total number data elements supplied as initial data
 * @param bufferUsage usage of the buffer (vertex, index, uniform etc.)
 * @param bufferStorage where should the buffer be stored? (CPU or GPU)
 */
Buffer::Buffer(VulkanApplication *app, void *data, uint32_t stride, uint32_t size,
               BufferUsage bufferUsage, BufferStorage bufferStorage)
{
    this->app = app;
    this->vulkanData = app->_getVulkanData();
    this->bufferUsage = bufferUsage;
    this->bufferStorage = bufferStorage;
    this->size = size;
    this->stride = stride;

    this->totalBufferSize = stride * size;

    createBuffer(data);
}

/**
 * Class destructor
 *
 * Frees all allocated buffer memory
 */
Buffer::~Buffer() { freeBuffer(); }

/**
 * Get internal Vulkan buffer
 *
 * @returns Vulkan buffer that is linked to this buffer
 */
VkBuffer Buffer::_getVkBuffer() { return this->buffer; }

/**
 * Get the total number of elements currently stored inside the buffer.
 *
 * @returns total number of elements stored by buffer
 */
uint32_t Buffer::getElementCount() { return this->size; }

/**
 * Get the total size of the buffer.
 *
 * @returns total size of the buffer in bytes
 */
uint32_t Buffer::getTotalSize() { return this->totalBufferSize; }

/**
 * Set the current contents of the buffer.
 *
 * @param data slice of data to insert into buffer
 * @param count number of elements to insert
 * @param offset offset in terms of data elements
 */
void Buffer::setData(void *data, uint32_t count, uint32_t offset)
{
    if ((count + offset) <= size)
    {
        // Use original buffer
        fillBuffer(data, count, offset);
    }
    else if (offset == 0)
    {
        // Free old buffer
        freeBuffer();

        size = count;
        totalBufferSize = size * stride;

        // Set new data
        createBuffer(data);
    }
    else
    {
        uint32_t newSize = offset + count;
        uint32_t newBufferSize = newSize * stride;

        // Get current data
        void *oldData = getData();

        // Free old buffer
        freeBuffer();

        void *newData = malloc(totalBufferSize);

        // Copy old data
        memcpy(newData, oldData, totalBufferSize);

        // Append new data
        memcpy(((char *)newData) + offset * stride, data, count * stride);

        // Set new data
        size = newSize;
        totalBufferSize = newBufferSize;

        createBuffer(newData);

        free(newData);
        free(oldData);
    }
}

/**
 * Get current contents of the buffer.
 *
 * WARNING: It is your responsibility to free this pointer after you are finished with it.
 *
 * @param count number of elements to return (set to 0 to gather all elements)
 * @param offset offset in terms of data elements
 *
 * @returns slice of current buffer contents
 */
void *Buffer::getData(uint32_t count, uint32_t offset)
{
    if (count == 0)
    {
        // Select entire buffer
        count = size;
    }

    // Total buffer memory size (bytes)
    uint32_t dataSize = count * stride;

    // Allocate memory to store buffer data
    void *bufferData = malloc(dataSize);

    if (bufferStorage == CPU)
    {
        void *mappedData;
        vmaMapMemory(vulkanData->allocator, allocation, &mappedData);
        memcpy(bufferData, (char *)mappedData + (stride * offset), dataSize);
        vmaUnmapMemory(vulkanData->allocator, allocation);
    }
    else if (bufferStorage == GPU)
    {
        // Create staging buffer that will recieve data from GPU buffer
        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation;
        VmaAllocationInfo stagingBufferAllocationInfo;

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.pNext = nullptr;
        bufferInfo.flags = 0;
        bufferInfo.size = dataSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

        if (vmaCreateBuffer(vulkanData->allocator, &bufferInfo, &allocInfo, &stagingBuffer,
                            &stagingBufferAllocation, &stagingBufferAllocationInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Shade: Failed to create staging buffer!");
        }

        // Copy data from staging buffer to GPU buffer
        VkBufferCopy regions[1];
        regions[0].srcOffset = offset * stride;
        regions[0].dstOffset = 0;
        regions[0].size = dataSize;

        // Call copy command (copy from internal buffer to transfer buffer)
        VkCommandBuffer commandBuffer = app->_beginSingleTimeCommands();
        vkCmdCopyBuffer(commandBuffer, buffer, stagingBuffer, 1, regions);
        app->_endSingleTimeCommands(commandBuffer);

        // Copy data from staging buffer
        void *mappedData;
        vmaMapMemory(vulkanData->allocator, stagingBufferAllocation, &mappedData);
        memcpy(bufferData, mappedData, dataSize);
        vmaUnmapMemory(vulkanData->allocator, stagingBufferAllocation);

        // Cleanup staging buffer
        vmaDestroyBuffer(vulkanData->allocator, stagingBuffer, stagingBufferAllocation);
    }
    else if (bufferStorage == GPU_WRITE_ONLY)
    {
        // Write only storage cannot be read from
        throw std::runtime_error(
            "Shade: Attempted to getData from buffer of storage type GPU_WRITE_ONLY. Getting data "
            "is not available for buffers with this storage type.");
    }

    return bufferData;
}

//--------------------
// Internal functions
//--------------------

uint32_t Buffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
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

    if (bufferUsage == VERTEX)
    {
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    else if (bufferUsage == INDEX)
    {
        bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    else if ((bufferUsage == UNIFORM) || (bufferUsage == DYNAMIC_UNIFORM))
    {
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    else if (bufferUsage == TRANSFER)
    {
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }

    if ((bufferStorage == GPU) || (bufferStorage == GPU_WRITE_ONLY))
    {
        // Enable transfers to buffer (using staging buffers)
        bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        if (bufferStorage != GPU_WRITE_ONLY)
        {
            // Enable reading from this buffer
            bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        }
    }

    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage =
        (bufferStorage == GPU) ? VMA_MEMORY_USAGE_GPU_ONLY : VMA_MEMORY_USAGE_CPU_ONLY;

    if (vmaCreateBuffer(vulkanData->allocator, &bufferInfo, &allocInfo, &buffer, &allocation,
                        &allocationInfo) != VK_SUCCESS)
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
        memcpy((char *)mappedData + (stride * offset), data, dataSize);
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

        if (vmaCreateBuffer(vulkanData->allocator, &bufferInfo, &allocInfo, &stagingBuffer,
                            &stagingBufferAllocation, &stagingBufferAllocationInfo) != VK_SUCCESS)
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
        vmaDestroyBuffer(vulkanData->allocator, stagingBuffer, stagingBufferAllocation);
    }
}

void Buffer::freeBuffer() { vmaDestroyBuffer(vulkanData->allocator, buffer, allocation); }
