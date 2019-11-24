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

    uint32_t stride;
    uint32_t count;
    uint32_t totalBufferSize;

    uint32_t findMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);
    void createBuffer(void *data, uint32_t stride, uint32_t count);
    void freeBuffer();

public:
    Buffer(VulkanApplication *app, void *data, uint32_t stride, uint32_t count, BufferType bufferType = VERTEX);
    ~Buffer();

    VkBuffer _getVkBuffer();

    uint32_t getElementCount();
    uint32_t getTotalSize();

    void setData(void *data, uint32_t stride, uint32_t count);
};
} // namespace Shade