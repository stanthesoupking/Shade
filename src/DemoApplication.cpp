#include "DemoApplication.hpp"

#include <iostream>

ShadeApplicationInfo DemoApplication::preInit()
{
    ShadeApplicationInfo appInfo;
    appInfo.windowSize = Rect(860, 640);
    appInfo.clearColour = Colour(0.15f, 0.15f, 0.15f);

    return appInfo;
}

void DemoApplication::init()
{
    std::cout << "Creating vertex buffer..." << std::endl;

    vertices = {
        {{0.0f, -0.5f}},
        {{0.5f, 0.5f}},
        {{-0.5f, 0.5f}}};

    vertexBuffer = createBuffer(vertices.data(), sizeof(Vertex), vertices.size());

    std::cout << "Creating index buffer..." << std::endl;

    indices = {
        0, 1, 2};

    indexBuffer = createBuffer(indices.data(), sizeof(int), indices.size());

    std::cout << "Loading shader..." << std::endl;

    ShaderLayout basicShaderLayout = ShaderLayout({VEC2});
    basicShader = createShaderFromSPIRVFile(basicShaderLayout, "../shaders/vert.spv", "../shaders/frag.spv");

    basicMaterial = new Material(basicShader);
}

void DemoApplication::destroy()
{
}

void DemoApplication::update()
{
}

void DemoApplication::render()
{
    renderMesh(indexBuffer, vertexBuffer, basicMaterial);
}