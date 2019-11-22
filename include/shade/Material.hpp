#pragma once

#include "./Shader.hpp"

namespace Shade
{
class Material
{
private:
    Shader* shader;
public:
    Material(Shader* shader);
    ~Material();

    Shader* getShader();
};
} // namespace Shade