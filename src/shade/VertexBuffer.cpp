#include "shade/VertexBuffer.hpp"

using namespace Shade;

VertexBuffer::VertexBuffer(VulkanApplication* app, StructuredBufferLayout vertexLayout, void* vertices, uint32_t count)
	: StructuredBuffer(app, vertexLayout, vertices, count, VERTEX)
{
	this->vertices = vertices;
}

VertexBuffer::~VertexBuffer()
{
}