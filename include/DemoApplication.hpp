#pragma once

#define SHADE_ENABLE_VALIDATION_LAYERS
#include "shade/Shade.hpp"

using namespace Shade;

class DemoApplication: public ShadeApplication
{
private:
    Buffer* vertexBuffer;
    Buffer* indexBuffer;
    Shader* basicShader;
    Material* basicMaterial;
public:
    ShadeApplicationInfo preInit();
    void init();
    void update();
    void render();
    void destroy();
};