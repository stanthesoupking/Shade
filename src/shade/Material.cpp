#include "shade/Material.hpp"

using namespace Shade;

Material::Material(Shader* shader)
{
    this->shader = shader;
}

Material::~Material()
{

}

Shader* Material::getShader()
{
    return this->shader;
}