#include "shade/StructuredBuffer.hpp"

#include <iostream>

using namespace Shade;

StructuredBuffer::StructuredBuffer(VulkanApplication* app,
		StructuredBufferLayout layout, void* data, uint32_t count, BufferType bufferType) :
	Buffer(app, data, layout.getStride(), count, bufferType)
{
	this->layout = layout;
}

StructuredBuffer::~StructuredBuffer()
{

}

void StructuredBuffer::setData(void* data, uint32_t count)
{
	Buffer::setData(data, layout.getStride(), count);
}

// Structured Buffer Layout Implementation

StructuredBufferLayout::StructuredBufferLayout(std::vector<StructuredBufferLayoutEntry> layout)
{
	this->layout = layout;
}

StructuredBufferLayout::~StructuredBufferLayout()
{
}

uint32_t StructuredBufferLayout::getBufferVariableTypeSize(StructuredBufferVariableType type)
{
	switch (type)
	{
	case FLOAT:
		return 4;
	case INT:
		return 4;
	case VEC2:
		return 8;
	case VEC3:
		return 16;
	case VEC4:
		return 16;
	case MAT2:
		return 16;
	case MAT3:
		return 52;
	case MAT4:
		return 64;
	default:
		std::runtime_error("Shade: Unknown variable type in shader layout.");
		break;
	}
}

VkFormat StructuredBufferLayout::getBufferVariableTypeFormat(StructuredBufferVariableType type)
{
	switch (type)
	{
	case FLOAT:
		return VK_FORMAT_R32_SFLOAT;
	case INT:
		return VK_FORMAT_R32_SINT;
	case VEC2:
		return VK_FORMAT_R32G32_SFLOAT;
	case VEC3:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case VEC4:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case MAT2:
		return VK_FORMAT_UNDEFINED;	 // TODO: Find correct format
	case MAT3:
		return VK_FORMAT_UNDEFINED;
	case MAT4:
		return VK_FORMAT_UNDEFINED;
	default:
		std::runtime_error("Shade: Unknown variable type in shader layout.");
		break;
	}
}

uint32_t StructuredBufferLayout::getStride()
{
	uint32_t stride = 0;

	for (const auto entry : layout)
	{
		stride += getBufferVariableTypeSize(entry.type);
	}

	return stride;
}

std::vector<VkVertexInputAttributeDescription> StructuredBufferLayout::_getAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> descriptions(layout.size());

	uint32_t i = 0;
	uint32_t offset = 0;
	for (const auto entry : layout)
	{
		descriptions[i].binding = 0;
		descriptions[i].location = 0;
		descriptions[i].format = getBufferVariableTypeFormat(entry.type);

		// TODO: Check if offset calculation is correct.
		//  e.g. is it on a bit-basis or element index basis?
		descriptions[i].offset = offset;

		i++;
		offset += getBufferVariableTypeSize(entry.type);
	}

	return descriptions;
}