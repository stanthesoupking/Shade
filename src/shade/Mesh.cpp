#include "shade/Mesh.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "shade/vendor/tiny_obj_loader.hpp"

#include <iostream>

using namespace Shade;

const StructuredBufferLayout Mesh::baseLayout = {
    {
        {"inPosition", VEC3},
        {"inTexCoord", VEC2}
    }
};

Mesh::Mesh(VulkanApplication *app, std::vector<int> indices,
           void *vertices, StructuredBufferLayout vertexLayout, uint32_t vertexCount)
{
    this->indexBuffer = new IndexBuffer(app, indices);
    this->vertexBuffer = new VertexBuffer(app, vertexLayout, vertices,
                                          vertexCount);
}

Mesh::~Mesh()
{
    delete indexBuffer;
    delete vertexBuffer;
}

IndexBuffer *Mesh::getIndexBuffer()
{
    return this->indexBuffer;
}

VertexBuffer *Mesh::getVertexBuffer()
{
    return this->vertexBuffer;
}

Mesh *Mesh::loadFromOBJ(VulkanApplication *app, std::string path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warning;
    std::string error;

    bool loadSuccess = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning,
                                        &error, path.c_str());

    if (!warning.empty())
    {
        std::cout << warning << std::endl;
    }

    if (!error.empty())
    {
        std::cerr << error << std::endl;
    }

    if (!loadSuccess)
    {
        throw std::runtime_error("Shade: Failed to load from OBJ file.");
    }

    // Select first shape
    size_t s = 0;
    tinyobj::shape_t shape = shapes[0];

    std::vector<BaseVertex> vertices;
    std::vector<int> indices;
    for (const auto &index : shape.mesh.indices)
    {
        BaseVertex vertex = {};

        vertex.inPosition = {
            attrib.vertices[3 * index.vertex_index + 0],
            attrib.vertices[3 * index.vertex_index + 1],
            attrib.vertices[3 * index.vertex_index + 2]};
        
        vertex.inTexCoord = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

        vertices.push_back(vertex);
        indices.push_back(indices.size());
    }

    return new Mesh(app, indices, vertices.data(), baseLayout, vertices.size());
}