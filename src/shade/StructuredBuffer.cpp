#include "shade/StructuredBuffer.hpp"

#include <iostream>
#include <cmath>

using namespace Shade;

StructuredBuffer::StructuredBuffer(VulkanApplication *app,
								   StructuredBufferLayout layout,
								   void *data, uint32_t count,
								   BufferType bufferType)
	: Buffer(
		  app, layout.alignData(data, count),
		  layout.getStride(),
		  count, bufferType)
{
	this->layout = layout;
}

StructuredBuffer::~StructuredBuffer()
{
}

void StructuredBuffer::setData(void *data, uint32_t count)
{
	Buffer::setData(layout.alignData(data, count), layout.getStride(), count);
}

// Structured Buffer Layout Implementation

StructuredBufferLayout::StructuredBufferLayout(std::vector<StructuredBufferLayoutEntry> layout)
{
	this->layout = layout;
}

StructuredBufferLayout::~StructuredBufferLayout()
{
}

/**
 * Return the buffer variable type size if it was aligned with the given
 * 	alignment value.
 */
uint32_t StructuredBufferLayout::getAlignedBufferVariableTypeSize(
	StructuredBufferVariableType type, uint32_t alignment)
{
	// Get original/nonaligned variable size
	uint32_t originalSize = getBufferVariableTypeSize(type);

	return ceil(originalSize / (float)alignment) * alignment;
}

uint32_t StructuredBufferLayout::getBufferVariableTypeAlignment(StructuredBufferVariableType type)
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
		return 16;
	case MAT4:
		return 16;
	default:
		std::runtime_error("Shade: Unknown variable type in shader layout.");
		break;
	}
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
		return 12;
	case VEC4:
		return 16;
	case MAT2:
		return 16;
	case MAT3:
		return 36;
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
		return VK_FORMAT_UNDEFINED; // TODO: Find correct format
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
	uint32_t largestAlignment = getLargestBufferVariableAlignment();
	uint32_t stride = 0;

	for (const auto entry : layout)
	{
		stride += getAlignedBufferVariableTypeSize(entry.type, 
			getBufferVariableTypeAlignment(entry.type));
	}

	return ceil(stride / (float)largestAlignment) * largestAlignment;;
}

uint32_t StructuredBufferLayout::getUnalignedStructStride()
{
	uint32_t stride = 0;

	for (const auto entry : layout)
	{
		stride += getAlignedBufferVariableTypeSize(entry.type,
			getBufferVariableTypeAlignment(entry.type));
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
		descriptions[i].location = i;
		descriptions[i].format = getBufferVariableTypeFormat(entry.type);

		// TODO: Check if offset calculation is correct.
		//  e.g. is it on a bit-basis or element index basis?
		descriptions[i].offset = offset;

		i++;
		offset += getBufferVariableTypeSize(entry.type);
	}

	return descriptions;
}

/**
 * Return an aligned copy of the given data. The returned data will be valid for
 * use in a Vulkan SPIR-V shader.
 */
void *StructuredBufferLayout::alignData(void *data, uint32_t count)
{
	uint32_t alignment = getLargestBufferVariableAlignment();

	// Allocate new data
	void *newData = malloc(getStride() * count);

	uint32_t uStructSize = getUnalignedStructStride();
	uint32_t aStructSize = getStride();
	uint32_t structSizeDiff = aStructSize - uStructSize;

	uint32_t dataPos = 0;
	uint32_t newDataPos = 0;
	for (uint32_t i = 0; i < count; i++)
	{
		for (const auto entry : layout)
		{
			// TODO: Possibly cache these values...
			uint32_t eAlignment = getBufferVariableTypeAlignment(entry.type);
			uint32_t uSize = getBufferVariableTypeSize(entry.type);
			uint32_t aSize = getAlignedBufferVariableTypeSize(entry.type, eAlignment);
			uint32_t sizeDiff = aSize - uSize;
            
            // Copy original data
            memcpy((char *)newData + newDataPos, (char *)data + dataPos, uSize);

            // Fill empty bytes to align data
            memset((char *)newData + newDataPos + uSize, 0x00, sizeDiff);


			// Move data position forward
			dataPos += uSize;

			// Move new data position forward
			newDataPos += aSize;
		}

		// Fill empty bytes to align struct
		memset((char*)newData + newDataPos, 0x00, structSizeDiff);
		newDataPos += structSizeDiff;
	}

	return newData;
}

/**
 * Return the largest alignment value of a variable in the layout.
 */
uint32_t StructuredBufferLayout::getLargestBufferVariableAlignment()
{
	uint32_t alignment = 0;

	for (const auto entry : layout)
	{
		uint32_t a = getBufferVariableTypeAlignment(entry.type);
		if (a > alignment)
		{
			alignment = a;
		}
	}

	return alignment;
}
