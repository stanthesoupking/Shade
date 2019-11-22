#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

namespace Shade
{
class Buffer
{
private:
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;

    uint32_t findMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);
public:
    Buffer(VkPhysicalDevice physicalDevice, VkDevice device, void *data, uint32_t size);
    ~Buffer();

    VkBuffer _getVkBuffer();
};
} // namespace Shade