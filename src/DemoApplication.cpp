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
	StructuredBufferLayout uniformLayout = StructuredBufferLayout({
		{"colour", VEC3}
	});

	StructuredBufferLayout vertexLayout = StructuredBufferLayout({
		{"pos", VEC2}
	});

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
        {1.0f, 0.0f, 0.0f}
    };

    uniformBuffer = new Buffer(this, &uniforms, sizeof(Uniforms), 1, UNIFORM);

    std::cout << "Loading shader..." << std::endl;

    basicShader = Shader::FromSPIRVFile(this, &uniformLayout, &vertexLayout, "../shaders/vert.spv", "../shaders/frag.spv");

    basicMaterial = new Material(this, basicShader, uniformBuffer);
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

}

void DemoApplication::render()
{
    renderMesh(indexBuffer, vertexBuffer, basicMaterial);
}