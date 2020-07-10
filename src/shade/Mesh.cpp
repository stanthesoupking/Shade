#include "shade/Mesh.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "shade/vendor/tiny_obj_loader.hpp"

#include <iostream>
#include <fstream>
#include <regex>
#include <map>

using namespace Shade;

/**
 * Class constructor
 * 
 * @param app Vulkan application that the mesh object is valid in.
 * @param indices Indices of the mesh
 * @param vertices Vertex data
 * @param vertexLayout Layout of the supplied vertex data
 * @param vertexCount Number of vertices supplied
 */
Mesh::Mesh(VulkanApplication *app, std::vector<int> indices,
           void *vertices, StructuredBufferLayout vertexLayout, uint32_t vertexCount)
{
    this->indexBuffer = new IndexBuffer(app, indices);
    this->vertexBuffer = new VertexBuffer(app, vertexLayout, vertices,
                                          vertexCount);
}

/**
 * Class destructor
 * 
 * Cleans up allocated buffers used by the mesh.
 */
Mesh::~Mesh()
{
    // Free buffers
    delete indexBuffer;
    delete vertexBuffer;
}

/**
 * Return the index buffer of the mesh.
 * 
 * Warning: this buffer will become invalid when the mesh object's
 *  deconstructor is called.
 * 
 * @return a pointer to the index buffer used in the mesh
 */
IndexBuffer *Mesh::getIndexBuffer()
{
    return this->indexBuffer;
}

/**
 * Return the vertex buffer of the mesh.
 * 
 * Warning: this buffer will become invalid when the mesh object's
 *  deconstructor is called.
 * 
 * @return a pointer to the vertex buffer used in the mesh
 */
VertexBuffer *Mesh::getVertexBuffer()
{
    return this->vertexBuffer;
}

/**
 * Returns the vertex property info of the given vertex layout
 * 
 * Get vertex property(position, normal, tex-coord) info for constructing
 * vertex data from within class methods.
 * 
 * @return completed VertexPropertyInfo struct for the given vertex layout
 */
VertexPropertyInfo Mesh::getVertexPropertyInfo(
    StructuredBufferLayout vertexLayout)
{
    VertexPropertyInfo propertyinfo = {};

    // Attempt to get position property
    propertyinfo.positionProperty = {false, 0};
    {
        std::optional<uint32_t> tOffset = vertexLayout.getPropertyOffset(
            SHADE_FLAG_POSITION);

        if (tOffset.has_value())
        {
            propertyinfo.positionProperty = {true, tOffset.value()};
        }
    }

    // Attempt to get normal property
    propertyinfo.normalProperty = {false, 0};
    {
        std::optional<uint32_t> tOffset = vertexLayout.getPropertyOffset(
            SHADE_FLAG_NORMAL);

        if (tOffset.has_value())
        {
            propertyinfo.normalProperty = {true, tOffset.value()};
        }
    }

    // Attempt to get texture coordinate property
    propertyinfo.texCoordProperty = {false, 0};
    {
        std::optional<uint32_t> tOffset = vertexLayout.getPropertyOffset(
            SHADE_FLAG_TEXCOORD);

        if (tOffset.has_value())
        {
            propertyinfo.texCoordProperty = {true, tOffset.value()};
        }
    }

    return propertyinfo;
}

/**
 * Returns the next line in a .PLY file.
 * 
 * Gets the next non-comment line in the PLY file. Used in the loadFromPLY 
 * function.
 * 
 * @param file ASCII-encoded PLY file
 * @return next line
 */
std::string Mesh::plyGetNextLine(std::fstream *file)
{
    std::string line;

    // Get line
    while (true)
    {
        std::getline(*file, line);

        // Ignore comment lines
        if (line.length() < 7 || line.substr(0, 7).compare("comment"))
            break;
    }

    return line;
}

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
Mesh *Mesh::loadFromPLY(VulkanApplication *app, std::string path,
                        StructuredBufferLayout vertexLayout,
                        bool swapZYAxis)
{
    std::fstream file;
    file.open(path, std::ios::in);

    // Check if file was opened successfully
    if (!file.is_open())
    {
        throw std::runtime_error("Shade: Failed to load from PLY file.");
    }

    // Check if file is a valid PLY file
    std::string lineBuffer;
    lineBuffer = plyGetNextLine(&file);
    if (lineBuffer.compare("ply") != 0)
    {
        throw std::runtime_error("Shade: Failed to load from PLY file due to invalid file format.");
    }

    // Check file format
    std::string sFormat, sFormatType, sFormatVersion;
    file >> sFormat >> sFormatType >> sFormatVersion;

    // Fail if format is not 'ASCII'
    if (sFormatType.compare("ascii") != 0)
    {
        throw std::runtime_error(
            "Shade Failed to load from PLY file due to file not being in ASCII format.");
    }

    // Fail if format version is not '1.0'
    if (sFormatVersion.compare("1.0") != 0)
    {
        throw std::runtime_error(
            "Shade Failed to load from PLY file due to file not being in ASCII 1.0 format.");
    }

    // Get vertex property info
    VertexPropertyInfo vertexPropertyInfo = getVertexPropertyInfo(vertexLayout);

    enum class ElementType
    {
        UNKNOWN,
        VERTEX,
        FACE
    };

    enum class ElementProperty
    {
        UNKNOWN, // Unknown (ignored) property
        X,
        Y,
        Z, // Position vector propreties
        NX,
        NY,
        NZ, // Normal vector properties
        S,
        T // TexCoord vector properties
    };

    std::map<std::string, ElementProperty> elementPropertyMap = {
        {"x", ElementProperty::X},
        {"y", ElementProperty::Y},
        {"z", ElementProperty::Z},
        {"nx", ElementProperty::NX},
        {"ny", ElementProperty::NY},
        {"nz", ElementProperty::NZ},
        {"s", ElementProperty::S},
        {"t", ElementProperty::T}};

    struct Element
    {
        ElementType type;
        uint32_t count;
        std::vector<ElementProperty> properties;
    };

    // Declare regex variables
    std::regex elementDeclaration = std::regex("^element (.*) (\\d+)$");
    std::regex propertyDecleration = std::regex("^property (.*) (.*)$");
    std::smatch sm;

    std::vector<Element> elements;

    // Read rest of header
    lineBuffer = plyGetNextLine(&file);
    while (lineBuffer.compare("end_header") && !file.eof())
    {
        // Check for element declaration line
        if (std::regex_match(lineBuffer, sm, elementDeclaration))
        {
            Element element = {};
            element.type = ElementType::UNKNOWN;
            element.count = std::stoi(sm[2]);

            if (!sm[1].compare("vertex"))
            {
                element.type = ElementType::VERTEX;
            }
            else if (!sm[1].compare("face"))
            {
                element.type = ElementType::FACE;
            }
            elements.push_back(element);
        }
        else if (std::regex_match(lineBuffer, sm, propertyDecleration))
        {
            // Attempt to get property enum mapping
            std::map<std::string, ElementProperty>::iterator it =
                elementPropertyMap.find(sm[2]);

            if (it != elementPropertyMap.end())
            {
                // Mapping was found
                elements.back().properties.push_back(it->second);
            }
        }

        lineBuffer = plyGetNextLine(&file);
    }

    // Initialise temporary buffers
    void *vertices;
    uint32_t vertexCount;
    std::vector<int> indices;

    // Read element data
    for (const auto element : elements)
    {
        if (element.type == ElementType::VERTEX)
        {
            uint32_t vertexStride = vertexLayout.getStride(app, VERTEX);
            vertexCount = element.count;

            vertices = malloc(vertexStride * vertexCount);
            memset(vertices, '\0', vertexStride * vertexCount); // Clear vertices

            glm::vec3 *currentVertexPosition = nullptr;
            glm::vec3 *currentVertexNormal = nullptr;
            glm::vec3 *currentVertexTexCoord = nullptr;

            // Record vertices
            for (int i = 0; i < element.count; i++)
            {
                void *currentVertex = ((char *)vertices) + (vertexStride * i);

                if (vertexPropertyInfo.positionProperty.set)
                {
                    currentVertexPosition =
                        (glm::vec3 *)((char *)currentVertex +
                                      vertexPropertyInfo.positionProperty.offset);
                }

                if (vertexPropertyInfo.normalProperty.set)
                {
                    currentVertexNormal =
                        (glm::vec3 *)((char *)currentVertex +
                                      vertexPropertyInfo.normalProperty.offset);
                }

                if (vertexPropertyInfo.texCoordProperty.set)
                {
                    currentVertexTexCoord =
                        (glm::vec3 *)((char *)currentVertex +
                                      vertexPropertyInfo.texCoordProperty.offset);
                }

                for (const auto property : element.properties)
                {
                    float floatBuffer;
                    file >> floatBuffer;

                    switch (property)
                    {
                    // Position
                    case (ElementProperty::X):
                        if (currentVertexPosition != nullptr)
                            currentVertexPosition->x = floatBuffer;
                        break;
                    case (ElementProperty::Y):
                        if (currentVertexPosition != nullptr)
                        {
                            if (swapZYAxis)
                            {
                                currentVertexPosition->z = floatBuffer;
                            }
                            else
                            {
                                currentVertexPosition->y = floatBuffer;
                            }
                        }
                        break;
                    case (ElementProperty::Z):
                        if (currentVertexPosition != nullptr)
                        {
                            if (swapZYAxis)
                            {
                                currentVertexPosition->y = floatBuffer;
                            }
                            else
                            {
                                currentVertexPosition->z = floatBuffer;
                            }
                        }
                        break;

                    // Normal
                    case (ElementProperty::NX):
                        if (currentVertexNormal != nullptr)
                            currentVertexNormal->x = floatBuffer;
                        break;
                    case (ElementProperty::NY):
                        if (currentVertexNormal != nullptr)
                        {
                            if (swapZYAxis)
                            {
                                currentVertexNormal->z = floatBuffer;
                            }
                            else
                            {
                                currentVertexNormal->y = floatBuffer;
                            }
                        }
                        break;
                    case (ElementProperty::NZ):
                        if (currentVertexNormal != nullptr)
                        {
                            if (swapZYAxis)
                            {
                                currentVertexNormal->y = floatBuffer;
                            }
                            else
                            {
                                currentVertexNormal->z = floatBuffer;
                            }
                        }
                        break;

                    // Texture Coordinate
                    case (ElementProperty::S):
                        if (currentVertexTexCoord != nullptr)
                            currentVertexTexCoord->x = floatBuffer;
                        break;
                    case (ElementProperty::T):
                        if (currentVertexTexCoord != nullptr)
                            currentVertexTexCoord->y = floatBuffer;
                        break;
                    default:
                        break;
                    }
                }
            }
        }
        else if (element.type == ElementType::FACE)
        {
            if (swapZYAxis) // Push indices in reverse order
            {
                for (int i = 0; i < element.count; i++)
                {
                    int c, i0, i1, i2;
                    file >> c >> i0 >> i1 >> i2;

                    indices.push_back(i2);
                    indices.push_back(i1);
                    indices.push_back(i0);
                }
            }
            else // Push indices in original order
            {
                for (int i = 0; i < element.count; i++)
                {
                    int c, i0, i1, i2;
                    file >> c >> i0 >> i1 >> i2;

                    indices.push_back(i0);
                    indices.push_back(i1);
                    indices.push_back(i2);
                }
            }
        }
    }

    // Return finished mesh
    return new Mesh(app, indices, vertices, vertexLayout, vertexCount);
}

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
    VertexPropertyInfo vertexPropertyInfo = getVertexPropertyInfo(vertexLayout);

    // Display warning if no properties were detected
    if (!(vertexPropertyInfo.positionProperty.set |
          vertexPropertyInfo.normalProperty.set |
          vertexPropertyInfo.texCoordProperty.set))
    {
        std::cout << "Shade: (Warning) Mesh::loadFromOBJ call does nothing as none"
                     " of the following properties were detected:"
                  << std::endl
                  << "  - Position" << std::endl
                  << "  - Normal" << std::endl
                  << "  - Texture Coordinate" << std::endl
                  << "These properties should be set in the vertex layout using the"
                  << " following flags:" << std::endl
                  << "  SHADE_FLAG_POSITION, SHADE_FLAG_NORMAL, "
                  << " SHADE_FLAG_TEXCOORD" << std::endl;
    }

    // Allocate vertex data
    void *vertices = malloc(vertexStride * vertexCount);
    memset(vertices, '\0', vertexStride * vertexCount); // Clear vertices

    std::vector<int> indices;

    uint32_t i = 0;
    for (const auto &index : shape.mesh.indices)
    {
        void *currentVertex = ((char *)vertices) + (vertexStride * i);

        if (vertexPropertyInfo.positionProperty.set)
        {
            glm::vec3 *currentVertexProp =
                (glm::vec3 *)((char *)currentVertex +
                              vertexPropertyInfo.positionProperty.offset);

            *currentVertexProp = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]};
        }

        if (vertexPropertyInfo.normalProperty.set)
        {
            glm::vec3 *currentVertexProp =
                (glm::vec3 *)((char *)currentVertex +
                              vertexPropertyInfo.normalProperty.offset);

            *currentVertexProp = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]};
        }

        if (vertexPropertyInfo.texCoordProperty.set)
        {
            glm::vec2 *currentVertexProp =
                (glm::vec2 *)((char *)currentVertex +
                              vertexPropertyInfo.texCoordProperty.offset);

            *currentVertexProp = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
        }

        indices.push_back(indices.size());
        i++;
    }

    return new Mesh(app, indices, vertices, vertexLayout, vertexCount);
}