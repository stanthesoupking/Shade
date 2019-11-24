#pragma once

#include "./Buffer.hpp"
#include "./VulkanApplication.hpp"

#include <vector>

namespace Shade
{
class VertexBuffer: public Buffer
{
private:
    void* vertices;

public:
    VertexBuffer(VulkanApplication* app, void * vertices, uint32_t stride, uint32_t count);
    ~VertexBuffer();
};
} // namespace Shade
