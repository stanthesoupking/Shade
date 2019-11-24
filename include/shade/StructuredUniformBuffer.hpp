#pragma once

#include "./VulkanApplication.hpp"
#include "./StructuredBuffer.hpp"

namespace Shade
{
class StructuredUniformBuffer : public StructuredBuffer
{
private:
public:
	StructuredUniformBuffer(VulkanApplication* app,
		StructuredBufferLayout uniformLayout, void* data,
		uint32_t location = 0);
	~StructuredUniformBuffer();
};
};