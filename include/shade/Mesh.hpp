#pragma once

#include "./VulkanApplication.hpp"
#include "./IndexBuffer.hpp"
#include "./VertexBuffer.hpp"
#include "./StructuredBuffer.hpp"

#include <vector>

#include <glm/glm.hpp>

namespace Shade
{
    
struct BaseVertex
{
    glm::vec2 inTexCoord;
    glm::vec3 inPosition;
};

class Mesh
{
private:
    Shader *shader;
    IndexBuffer *indexBuffer;
    VertexBuffer *vertexBuffer;

public:
    Mesh(VulkanApplication *app, std::vector<int> indices,
         void *vertices, StructuredBufferLayout vertexLayout,
         uint32_t vertexCount);
    ~Mesh();

    static Mesh* loadFromOBJ(VulkanApplication *app, std::string path);

    IndexBuffer* getIndexBuffer();
    VertexBuffer* getVertexBuffer();

    static const StructuredBufferLayout baseLayout;
};

} // namespace Shade