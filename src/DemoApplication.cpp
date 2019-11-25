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
    StructuredBufferLayout uniformLayout = StructuredBufferLayout(
        {{"colour", VEC3},
         {"mvp", MAT4}});

    StructuredBufferLayout vertexLayout = StructuredBufferLayout(
		{{"pos", VEC2},
		{"inColour", VEC3}});

    std::cout << "Creating vertex buffer..." << std::endl;

	vertices = {
		{{0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}} };

    vertexBuffer = new VertexBuffer(this, vertexLayout, vertices.data(), vertices.size());

    std::cout << "Creating index buffer..." << std::endl;

    indices = {
        2, 1, 0, 3, 2, 0};

    indexBuffer = new IndexBuffer(this, indices);

    std::cout << "Creating uniform buffer..." << std::endl;

    uniforms = {
        {1.0f, 0.0f, 0.0f},
        glm::aligned_mat4(1.0f)
    };

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

    if (time > 1000)
    {
        time = 0;
    }

	// Calculate mvp
	glm::aligned_mat4 model = glm::rotate(glm::aligned_mat4(1.0f), (time / 1000.0f) * glm::radians(180.0f), glm::aligned_vec3(0.0f, 0.0f, 1.0f));
	glm::aligned_mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::aligned_mat4 projection = glm::perspective(glm::radians(45.0f), 860.0f / 480.0f, 0.1f, 10.0f);
	uniforms.mvp = projection * view * model;
	uniforms.mvp[1][1] *= -1;

    uniformBuffer->setData(&uniforms);
}

void DemoApplication::render()
{
    renderMesh(indexBuffer, vertexBuffer, basicMaterial);
}