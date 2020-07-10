#include "shade/IndexBuffer.hpp"

using namespace Shade;

IndexBuffer::IndexBuffer(VulkanApplication *app, std::vector<int> indices) : Buffer(app, indices.data(), sizeof(int), indices.size(), INDEX)
{
    this->indices = indices;
}

IndexBuffer::~IndexBuffer()
{
}