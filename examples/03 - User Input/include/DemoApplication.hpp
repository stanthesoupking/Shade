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
    glm::vec3 inNormal;
};

struct UniformMVP
{
    glm::mat4 mvp;
};

struct UniformLighting
{
    glm::vec3 lightDirection;
    glm::vec3 lightColour;
};

class DemoApplication: public ShadeApplication
{
private:
    Mesh* mesh;
    UniformTexture* texture;
    StructuredUniformBuffer* uniformMVPBuffer;
    StructuredUniformBuffer* uniformLightingBuffer;
    Shader* basicShader;
	Material* basicMaterial;

    std::vector<Vertex> vertices;
    std::vector<int> indices;
	UniformMVP uniformMVP;
    UniformLighting uniformLighting;

    glm::vec2 cubeRotation;

public:
    ShadeApplicationInfo preInit();
    void init();
    void update(ShadeApplicationFrameData frameData);
    void render();
    void destroy();
};
