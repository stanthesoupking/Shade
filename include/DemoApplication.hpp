#pragma once

#define SHADE_ENABLE_VALIDATION_LAYERS
#include "shade/Shade.hpp"

#include "glm/glm.hpp"

using namespace Shade;

struct Vertex
{
    glm::vec2 pos;
};

class DemoApplication: public ShadeApplication
{
private:
    Buffer* vertexBuffer;
    Buffer* indexBuffer;
    Shader* basicShader;
    Material* basicMaterial;

    std::vector<Vertex> vertices;
    std::vector<int> indices;
public:
    ShadeApplicationInfo preInit();
    void init();
    void update();
    void render();
    void destroy();
};