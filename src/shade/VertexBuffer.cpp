#include "shade/VertexBuffer.hpp"

using namespace Shade;

VertexBuffer::VertexBuffer(VulkanApplication* app, void* vertices, uint32_t stride, uint32_t count)
	: Buffer(app, vertices, stride, count, VERTEX)
{
	this->vertices = vertices;
}

VertexBuffer::~VertexBuffer()
{
}