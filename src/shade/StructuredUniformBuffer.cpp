#include "shade/StructuredUniformBuffer.hpp"

using namespace Shade;

StructuredUniformBuffer::StructuredUniformBuffer(VulkanApplication *_app,
                                                 StructuredBufferLayout _uniformLayout, void *data,
                                                 uint32_t _size, bool _dynamic)
    : StructuredBuffer(_app, _uniformLayout, data, _size, _dynamic ? DYNAMIC_UNIFORM : UNIFORM)
{
    app = _app;
    uniformLayout = _uniformLayout;
    size = _size;
    dynamic = _dynamic;
}

StructuredUniformBuffer::~StructuredUniformBuffer() {}

void StructuredUniformBuffer::setData(void *data, uint32_t count, uint32_t offset)
{
    StructuredBuffer::setData(data, count, offset);
}

bool StructuredUniformBuffer::getDynamic() { return dynamic; }

uint32_t StructuredUniformBuffer::getStride()
{
    return uniformLayout.getStride(app, BufferUsage::UNIFORM);
}