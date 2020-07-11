#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

#include "./vendor/vk_mem_alloc.hpp"

#include "./VulkanApplication.hpp"

namespace Shade
{

// Buffer usage types
enum BufferUsage
{
    VERTEX,
    INDEX,
    UNIFORM,
    DYNAMIC_UNIFORM,
    TRANSFER
};

// Buffer storage locations
enum BufferStorage
{
    CPU,            // Faster access times for CPU, access from GPU
    GPU,            // Faster access times for GPU, access from CPU
    GPU_WRITE_ONLY, // Fastest access for GPU, cannot be read from
};

/**
 * Buffer object that simplifies transferring data into GPU accessible memory.
 */
class Buffer
{
private:
    VulkanApplication *app;            // The application instance this buffer belongs to
    VulkanApplicationData *vulkanData; // Easy access to vulkan application data

    VkBuffer buffer;                  // Vulkan buffer
    VmaAllocation allocation;         // Allocation for AMD's Vulkan Memory Allocator
    VmaAllocationInfo allocationInfo; // VMA Allocation info

    BufferUsage bufferUsage;     // Buffer type/usage
    BufferStorage bufferStorage; // Buffer storage location (CPU/GPU)

    uint32_t stride;          // Stride between data elements/element size in bytes
    uint32_t size;            // Total number data elements currently contained inside the buffer
    uint32_t totalBufferSize; // Current buffer size in bytes

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(void *data);
    void fillBuffer(void *data, uint32_t count, uint32_t offset);
    void freeBuffer();

public:
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
    Buffer(VulkanApplication *app, void *data, uint32_t stride, uint32_t size,
           BufferUsage bufferUsage = VERTEX, BufferStorage bufferStorage = GPU);

    /**
     * Class destructor
     *
     * Frees all allocated buffer memory
     */
    ~Buffer();

    /**
     * Get internal Vulkan buffer
     *
     * @returns Vulkan buffer that is linked to this buffer
     */
    VkBuffer _getVkBuffer();

    /**
     * Get the total number of elements currently stored inside the buffer.
     *
     * @returns total number of elements stored by buffer
     */
    uint32_t getElementCount();

    /**
     * Get the total size of the buffer.
     *
     * @returns total size of the buffer in bytes
     */
    uint32_t getTotalSize();

    /**
     * Set the current contents of the buffer.
     *
     * @param data slice of data to insert into buffer
     * @param count number of elements to insert
     * @param offset offset in terms of data elements
     */
    void setData(void *data, uint32_t count = 1, uint32_t offset = 0);

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
    void *getData(uint32_t count = 0, uint32_t offset = 0);
};
} // namespace Shade