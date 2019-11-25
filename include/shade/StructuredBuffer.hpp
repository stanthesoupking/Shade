#pragma once

#include "Buffer.hpp"

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

struct StructuredBufferLayoutEntry
{
	std::string name;
	StructuredBufferVariableType type;
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

	uint32_t getStride();

	void *alignData(void *data, uint32_t count);
	uint32_t getLargestBufferVariableAlignment();
	std::vector<VkVertexInputAttributeDescription> _getAttributeDescriptions();
};

class StructuredBuffer : public Buffer
{
private:
	StructuredBufferLayout layout;

public:
	StructuredBuffer(VulkanApplication *app, StructuredBufferLayout layout, void *data, uint32_t count, BufferType bufferType = VERTEX);
	~StructuredBuffer();

	void setData(void *data, uint32_t count);
};
} // namespace Shade