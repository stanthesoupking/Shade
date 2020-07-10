#pragma once

#include "./StructuredBuffer.hpp"
#include "./VulkanApplication.hpp"

#include <vector>

namespace Shade
{
class VertexBuffer: public StructuredBuffer
{
private:
    void* vertices;

public:
    VertexBuffer(VulkanApplication* app, StructuredBufferLayout vertexLayout, void * vertices, uint32_t count);
    ~VertexBuffer();
};
} // namespace Shade
