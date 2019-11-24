#pragma once

#define SHADE_ENABLE_VALIDATION_LAYERS
#include "shade/Shade.hpp"

#include <glm/glm.hpp>

using namespace Shade;

struct Vertex
{
    glm::vec2 pos;
};

struct Uniforms
{
    glm::vec3 colour;
};

class DemoApplication: public ShadeApplication
{
private:
    VertexBuffer* vertexBuffer;
    IndexBuffer* indexBuffer;
    StructuredUniformBuffer* uniformBuffer;
    Shader* basicShader;
	Material* basicMaterial;

    std::vector<Vertex> vertices;
    std::vector<int> indices;
    Uniforms uniforms;

    int time;
public:
    ShadeApplicationInfo preInit();
    void init();
    void update();
    void render();
    void destroy();
};