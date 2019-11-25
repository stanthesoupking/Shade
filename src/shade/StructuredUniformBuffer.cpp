#include "shade/StructuredUniformBuffer.hpp"

using namespace Shade;

StructuredUniformBuffer::StructuredUniformBuffer(VulkanApplication* app,
	StructuredBufferLayout uniformLayout, void* data,
	uint32_t location) : StructuredBuffer(app, uniformLayout, data, 1, UNIFORM)
{

}

StructuredUniformBuffer::~StructuredUniformBuffer()
{

}

void StructuredUniformBuffer::setData(void* data)
{
	StructuredBuffer::setData(data, 1);
}