#include "shade/Mesh.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "shade/vendor/tiny_obj_loader.hpp"

#include <iostream>

using namespace Shade;

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

Mesh *Mesh::loadFromOBJ(VulkanApplication *app, std::string path,
                        StructuredBufferLayout vertexLayout)
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

    uint32_t vertexCount = static_cast<uint32_t>(shape.mesh.indices.size());
    uint32_t vertexStride = vertexLayout.getUnalignedStride();

    // Get vertex variable offsets:

    // Attempt to get position property
    bool positionPropertySet = false;
    uint32_t positionPropertyOffset = 0;
    {
        std::optional<uint32_t> tOffset = vertexLayout.getPropertyOffset(
            SHADE_FLAG_POSITION);

        if (tOffset.has_value())
        {
            positionPropertyOffset = tOffset.value();
            positionPropertySet = true;
        }
    }

    // Attempt to get normal property
    bool normalPropertySet = false;
    uint32_t normalPropertyOffset = 0;
    {
        std::optional<uint32_t> tOffset = vertexLayout.getPropertyOffset(
            SHADE_FLAG_NORMAL);

        if (tOffset.has_value())
        {
            normalPropertyOffset = tOffset.value();
            normalPropertySet = true;
        }
    }

    // Attempt to get texture coordinate property
    bool texCoordPropertySet = false;
    uint32_t texCoordPropertyOffset = 0;
    {
        std::optional<uint32_t> tOffset = vertexLayout.getPropertyOffset(
            SHADE_FLAG_TEXCOORD);

        if (tOffset.has_value())
        {
            texCoordPropertyOffset = tOffset.value();
            texCoordPropertySet = true;
        }
    }

	// Display warning if no properties were detected
	if (!(positionPropertySet | normalPropertySet | texCoordPropertySet))
	{
		std::cout << "Shade: (Warning) Mesh::loadFromOBJ call does nothing as none"
			" of the following properties were detected:" << std::endl <<
			"  - Position" << std::endl <<
			"  - Normal" << std::endl <<
			"  - Texture Coordinate" << std::endl <<
			"These properties should be set in the vertex layout using the" <<
			" following flags:" << std::endl <<
			"  SHADE_FLAG_POSITION, SHADE_FLAG_NORMAL, " <<
			" SHADE_FLAG_TEXCOORD" << std::endl;		
	}

    // Allocate vertex data
    void *vertices = malloc(vertexStride * vertexCount);
    memset(vertices, '\0', vertexStride * vertexCount); // Clear vertices

    std::vector<int> indices;

    uint32_t i = 0;
    for (const auto &index : shape.mesh.indices)
    {
        void *currentVertex = ((char*)vertices) + (vertexStride * i);

        if (positionPropertySet)
        {
            glm::vec3 *currentVertexProp = (glm::vec3 *) ((char*)currentVertex + positionPropertyOffset);
            *currentVertexProp = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]};
        }

        if (normalPropertySet)
        {
            glm::vec3 *currentVertexProp = (glm::vec3 *) ((char*)currentVertex + normalPropertyOffset);
            *currentVertexProp = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]};
        }

        if (texCoordPropertySet)
        {
            glm::vec2 *currentVertexProp = (glm::vec2*) ((char*)currentVertex + texCoordPropertyOffset);
            *currentVertexProp = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
        }

        indices.push_back(indices.size());
        i++;
    }

    return new Mesh(app, indices, vertices, vertexLayout, vertexCount);
}