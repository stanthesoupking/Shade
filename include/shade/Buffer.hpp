#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

#include "./VulkanApplication.hpp"

namespace Shade
{

enum BufferType
{
    VERTEX,
    INDEX,
    UNIFORM
};

class Buffer
{
private:
    VulkanApplicationData *vulkanData;

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;

    BufferType bufferType = VERTEX;

    uint32_t elementSize;
    uint32_t elementCount;
    uint32_t totalBufferSize;

    uint32_t findMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);
    void createBuffer(void *data, uint32_t elementSize, uint32_t elementCount);
    void freeBuffer();

public:
    Buffer(VulkanApplication *app, void *data, uint32_t elementSize, uint32_t elementCount, BufferType bufferType = VERTEX);
    ~Buffer();

    VkBuffer _getVkBuffer();

    uint32_t getElementCount();
    uint32_t getTotalSize();

    void setData(void *data, uint32_t elementSize, uint32_t elementCount);
};
} // namespace Shade