#pragma once

#define SHADE_ENABLE_VALIDATION_LAYERS
#include "shade/Shade.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Shade;

struct Vertex
{
	glm::vec2 inTexCoord;
    glm::vec3 inPosition;
};

struct UniformData
{
    glm::mat4 mvp;
    glm::vec2 repeat;
};

class DemoApplication: public ShadeApplication
{
private:
    Mesh* mesh;
    StructuredUniformBuffer* uniformBuffer;
    UniformTexture* florenceTexture;
    Shader* basicShader;
	Material* basicMaterial;

    std::vector<Vertex> vertices;
    std::vector<int> indices;
	UniformData uniformData;

public:
    ShadeApplicationInfo preInit();
    void init();
    void update();
    void render();
    void destroy();
};
