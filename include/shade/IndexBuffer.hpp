#pragma once

#include "./Buffer.hpp"
#include "./VulkanApplication.hpp"

#include <vector>

namespace Shade
{
class IndexBuffer: public Buffer
{
private:
    std::vector<int> indices;

public:
    IndexBuffer(VulkanApplication* app, std::vector<int> indices);
    ~IndexBuffer();

    void createBuffer(void *data, uint32_t elementSize, uint32_t elementCount, BufferType bufferType = INDEX);
};
} // namespace Shade
