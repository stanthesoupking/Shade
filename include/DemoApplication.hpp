#pragma once

#define SHADE_ENABLE_VALIDATION_LAYERS
#include "shade/Shade.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtx/type_aligned.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Shade;

struct Vertex
{
    glm::vec2 pos;
};

struct Uniforms
{
    glm::vec3 colour;
    glm::aligned_mat4 transformMatrix;
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