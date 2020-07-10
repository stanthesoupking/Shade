#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

#include "./vendor/vk_mem_alloc.hpp"

#include "./VulkanApplication.hpp"

namespace Shade
{

enum BufferType
{
    VERTEX,
    INDEX,
    UNIFORM,
    DYNAMIC_UNIFORM,
	TRANSFER
};

enum BufferStorage
{
    CPU,
    GPU
};

class Buffer
{
private:
    VulkanApplication *app;
    VulkanApplicationData *vulkanData;

    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;

    BufferType bufferType;
    BufferStorage bufferStorage;

    uint32_t stride;
    uint32_t size;
    uint32_t totalBufferSize;

    uint32_t findMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);
    void createBuffer(void *data);
    void fillBuffer(void *data, uint32_t count, uint32_t offset);
    void freeBuffer();

public:
    Buffer(VulkanApplication *app, void *data, uint32_t stride, uint32_t size,
        BufferType bufferType = VERTEX, BufferStorage = GPU);
    ~Buffer();

    VkBuffer _getVkBuffer();

    uint32_t getElementCount();
    uint32_t getTotalSize();

    void setData(void *data, uint32_t count = 1, uint32_t offset = 0);
};
} // namespace Shade