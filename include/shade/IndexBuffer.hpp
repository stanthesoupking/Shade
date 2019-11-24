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
};
} // namespace Shade
