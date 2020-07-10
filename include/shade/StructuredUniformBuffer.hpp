#pragma once

#include "./VulkanApplication.hpp"
#include "./StructuredBuffer.hpp"

namespace Shade
{
	class StructuredUniformBuffer : public StructuredBuffer
	{
	private:
		VulkanApplication *app;
		uint32_t size;
		bool dynamic;
		StructuredBufferLayout uniformLayout;

	public:
		StructuredUniformBuffer(VulkanApplication *_app,
								StructuredBufferLayout _uniformLayout, void *data, uint32_t size = 1, bool dynamic = false);
		~StructuredUniformBuffer();

		void setData(void *data, uint32_t count = 1, uint32_t offset = 0);

		bool getDynamic();

		uint32_t getStride();
	};
}; // namespace Shade