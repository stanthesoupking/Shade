#pragma once

#define SHADE_ENABLE_VALIDATION_LAYERS
#include "shade/Shade.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Shade;

struct Vertex
{
    glm::vec3 inPosition;
	glm::vec2 inTexCoord;
};

struct UniformData
{
    glm::mat4 mvp;
};

class DemoApplication: public ShadeApplication
{
private:
    Mesh* mesh;
    UniformTexture* texture;
    StructuredUniformBuffer* uniformBuffer;
    Shader* basicShader;
	Material* basicMaterial;

    std::vector<Vertex> vertices;
    std::vector<int> indices;
	UniformData uniformData;

    glm::vec2 cubeRotation;

public:
    ShadeApplicationInfo preInit();
    void init();
    void update(ShadeApplicationFrameData frameData);
    void render();
    void destroy();
};
