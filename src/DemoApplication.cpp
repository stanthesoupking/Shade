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
    StructuredBufferLayout uniformLayout = StructuredBufferLayout({{"colour", VEC3}});

    StructuredBufferLayout vertexLayout = StructuredBufferLayout({{"pos", VEC2}});

    std::cout << "Creating vertex buffer..." << std::endl;

    vertices = {
        {{0.0f, -0.5f}},
        {{0.5f, 0.5f}},
        {{-0.5f, 0.5f}}};

    vertexBuffer = new VertexBuffer(this, vertexLayout, vertices.data(), vertices.size());

    std::cout << "Creating index buffer..." << std::endl;

    indices = {
        0, 1, 2};

    indexBuffer = new IndexBuffer(this, indices);

    std::cout << "Creating uniform buffer..." << std::endl;

    uniforms = {
        {1.0f, 0.0f, 0.0f}};

    uniformBuffer = new StructuredUniformBuffer(this, uniformLayout, &uniforms);

    std::cout << "Loading shader..." << std::endl;

    basicShader = Shader::FromSPIRVFile(this, &uniformLayout, &vertexLayout, "../shaders/vert.spv", "../shaders/frag.spv");

    basicMaterial = new Material(this, basicShader, uniformBuffer);

    time = 0;
}

void DemoApplication::destroy()
{
    // Cleanup
    delete basicMaterial;
    delete basicShader;

    delete vertexBuffer;
    delete indexBuffer;
    delete uniformBuffer;
}

void DemoApplication::update()
{
    // Demo colour shift
    // TODO: Add more colours
    time++;

    if (time > 100)
    {
        time = 0;
    }

    uniforms = {
        {1.0f - (time / 100.0f), 0.0f, 0.0f}
    };

    uniformBuffer->setData(&uniforms);
}

void DemoApplication::render()
{
    renderMesh(indexBuffer, vertexBuffer, basicMaterial);
}