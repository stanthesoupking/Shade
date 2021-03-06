#pragma once 

#define SHADE_VERSION_MAJOR 0
#define SHADE_VERSION_MINOR 1
#define SHADE_VERSION_PATCH 0

#define SHADE_ENABLE_VALIDATION_LAYERS

#ifdef SHADE_ENABLE_VALIDATION_LAYERS
    #define _SHADE_ENABLE_VALIDATION_LAYERS 1
#else
    #define _SHADE_ENABLE_VALIDATION_LAYERS 0
#endif

#include "./Colour.hpp"
#include "./Rect.hpp"
#include "./Buffer.hpp"
#include "./StructuredBuffer.hpp"
#include "./StructuredUniformBuffer.hpp"
#include "./IndexBuffer.hpp"
#include "./VertexBuffer.hpp"
#include "./UniformTexture.hpp"
#include "./Shader.hpp"
#include "./Material.hpp"
#include "./Mesh.hpp"
#include "./ShadeApplication.hpp"