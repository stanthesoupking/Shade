#pragma once

#include "./VulkanApplication.hpp"
#include "./IndexBuffer.hpp"
#include "./VertexBuffer.hpp"
#include "./StructuredBuffer.hpp"

#include <vector>

#include <glm/glm.hpp>

namespace Shade
{
struct VertexPropertyInfoEntry
{
    bool set;
    uint32_t offset;
};

struct VertexPropertyInfo
{
    VertexPropertyInfoEntry positionProperty;
    VertexPropertyInfoEntry normalProperty;
    VertexPropertyInfoEntry texCoordProperty;
};
/**
 * Mesh implementation for the Shade engine.
 * 
 * Stores indices and vertices that make up a mesh.
 */
class Mesh
{
private:
    Shader *shader;
    IndexBuffer *indexBuffer;
    VertexBuffer *vertexBuffer;

    /**
     * Returns the vertex property info of the given vertex layout
     * 
     * Get vertex property(position, normal, tex-coord) info for constructing
     * vertex data from within class methods.
     * 
     * @return completed VertexPropertyInfo struct for the given vertex layout
     */
    static VertexPropertyInfo getVertexPropertyInfo(
        StructuredBufferLayout vertexLayout);

    /**
     * Returns the next line in a .PLY file.
     * 
     * Gets the next non-comment line in the PLY file. Used in the loadFromPLY 
     * function.
     * 
     * @param file ASCII-encoded PLY file
     * @return next line
     */
    static std::string plyGetNextLine(std::fstream *file);

public:
    /**
     * Class constructor
     * 
     * @param app Vulkan application that the mesh object is valid in.
     * @param indices Indices of the mesh
     * @param vertices Vertex data
     * @param vertexLayout Layout of the supplied vertex data
     * @param vertexCount Number of vertices supplied
     */
    Mesh(VulkanApplication *app, std::vector<int> indices,
         void *vertices, StructuredBufferLayout vertexLayout,
         uint32_t vertexCount);

    /**
     * Class destructor
     * 
     * Cleans up allocated buffers used by the mesh.
     */
    ~Mesh();

    /**
     * (WIP) Loads mesh from the PLY file at the given path.
     * 
     * Loads a given PLY file with the given file format:
     *  ASCII 1.0
     * 
     * By default, this function swaps the Z and Y axis to conform with Shade's
     *  coordinate system. This can be disabled by setting the 'swapZYAxis' 
     *  parameter to false.
     * 
     * Warning: This method is WIP and is likely to not be compatible with every
     *  PLY file.
     * 
     * @param app Vulkan application that the returned mesh object is valid in.
     * @param path Path to the PLY file
     * @param vertexLayout Vertex layout to load vertices into
     * @param swapZYAxis Swap the z and y axis of the mesh to conform with Shade
     *  standards; Y = Up/Down, Z = Forward/Backward
     * @return mesh loaded from PLY file
     */
    static Mesh *loadFromPLY(VulkanApplication *app, std::string path,
                             StructuredBufferLayout vertexLayout,
                             bool swapZYAxis = true);

    /**
     * Loads mesh from the Wavefront OBJ file at the given path.
     * 
     * Loads a given Wavefront OBJ file at the given path. Uses syoyo's tinyobjloader
     *  under the hood (https://github.com/syoyo/tinyobjloader).
     * 
     * @param app Vulkan application that the returned mesh object is valid in.
     * @param path Path to the PLY file
     * @param vertexLayout Vertex layout to load vertices into
     * @return mesh loaded from PLY file
     */
    static Mesh *loadFromOBJ(VulkanApplication *app, std::string path,
                             StructuredBufferLayout vertexLayout);

    /**
     * Return the index buffer of the mesh.
     * 
     * Warning: this buffer will become invalid when the mesh object's
     *  deconstructor is called.
     * 
     * @return a pointer to the index buffer used in the mesh
     */
    IndexBuffer *getIndexBuffer();

    /**
     * Return the vertex buffer of the mesh.
     * 
     * Warning: this buffer will become invalid when the mesh object's
     *  deconstructor is called.
     * 
     * @return a pointer to the vertex buffer used in the mesh
     */
    VertexBuffer *getVertexBuffer();
};

} // namespace Shade