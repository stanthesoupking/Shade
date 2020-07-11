#pragma once

#include "Buffer.hpp"

#include "shade/VulkanApplication.hpp"

#include <optional>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

namespace Shade
{

enum StructuredBufferVariableType
{
    FLOAT,
    INT,
    VEC2,
    VEC3,
    VEC4,
    MAT2,
    MAT3,
    MAT4
};

enum StructuredBufferLayoutEntryFlag
{
    SHADE_FLAG_NONE,
    SHADE_FLAG_POSITION,
    SHADE_FLAG_NORMAL,
    SHADE_FLAG_TEXCOORD
};

struct StructuredBufferLayoutEntry
{
    std::string name;
    StructuredBufferVariableType type;
    StructuredBufferLayoutEntryFlag flag = SHADE_FLAG_NONE;
};

class StructuredBufferLayout
{
private:
    std::vector<StructuredBufferLayoutEntry> layout;

    uint32_t getAlignedBufferVariableTypeSize(StructuredBufferVariableType type,
                                              uint32_t alignment);

    uint32_t getBufferVariableTypeAlignment(StructuredBufferVariableType type);
    uint32_t getBufferVariableTypeSize(StructuredBufferVariableType type);
    VkFormat getBufferVariableTypeFormat(StructuredBufferVariableType type);

    uint32_t getUnalignedStructStride();

public:
    StructuredBufferLayout() { layout = {}; }
    StructuredBufferLayout(std::vector<StructuredBufferLayoutEntry> layout);
    ~StructuredBufferLayout();

    uint32_t getStride(VulkanApplication *app, BufferUsage bufferUsage);
    uint32_t getAlignedStride(VulkanApplication *app, BufferUsage bufferUsage);
    uint32_t getUnalignedStride();

    void *alignData(VulkanApplication *app, void *data, uint32_t count, BufferUsage bufferUsage);
    uint32_t getLargestBufferVariableAlignment();
    std::vector<VkVertexInputAttributeDescription> _getAttributeDescriptions();

    std::optional<uint32_t> getPropertyOffset(StructuredBufferLayoutEntryFlag flag);
};

class StructuredBuffer : public Buffer
{
private:
    VulkanApplication *app;
    StructuredBufferLayout layout;
    BufferUsage bufferUsage;

    static void *prepareData(VulkanApplication *app, void *data, uint32_t count,
                             BufferUsage bufferUsage, StructuredBufferLayout layout);

public:
    StructuredBuffer(VulkanApplication *_app, StructuredBufferLayout layout, void *data,
                     uint32_t count, BufferUsage bufferUsage = VERTEX,
                     BufferStorage bufferStorage = GPU);
    ~StructuredBuffer();

    void setData(void *data, uint32_t count, uint32_t offset = 0);
};
} // namespace Shade