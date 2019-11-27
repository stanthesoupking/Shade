#include "DemoApplication.hpp"

#include <iostream>

ShadeApplicationInfo DemoApplication::preInit()
{
    ShadeApplicationInfo appInfo;
    appInfo.windowSize = Rect(860, 640);
    appInfo.clearColour = Colour(0.15f, 0.15f, 0.15f);
	appInfo.windowResizable = true;

    return appInfo;
}

void DemoApplication::init()
{
    StructuredBufferLayout uniformDataLayout = StructuredBufferLayout(
        {{"mvp", MAT4}});

    StructuredBufferLayout vertexLayout = StructuredBufferLayout(
		{{"inPosition", VEC2},
		{"inTexCoord", VEC2}});

	std::cout << "Loading Florence texture..." << std::endl;

	florenceTexture = new UniformTexture(this, "../assets/textures/florence.png");
	
	ShaderLayout shaderLayout = {
		{
			{0, ShaderStage::VERTEX, uniformDataLayout},
			{1, ShaderStage::FRAGMENT, UniformTextureLayout()}
		},
		vertexLayout
	};

    std::cout << "Creating vertex buffer..." << std::endl;

	vertices = {
		{{0.5f, -0.5f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f}},
		{{-0.5f, 0.5f}, {1.0f, 1.0f}},
		{{-0.5f, -0.5f}, {1.0f, 0.0f}} };

    vertexBuffer = new VertexBuffer(this, vertexLayout, vertices.data(), vertices.size());

    std::cout << "Creating index buffer..." << std::endl;

    indices = {
        2, 1, 0, 3, 2, 0};

    indexBuffer = new IndexBuffer(this, indices);

    std::cout << "Creating uniform buffer..." << std::endl;

	uniformData = {
        glm::mat4(1.0f)
    };

    uniformBuffer = new StructuredUniformBuffer(this, uniformDataLayout, &uniformData);

    std::cout << "Loading shader..." << std::endl;

    basicShader = Shader::FromSPIRVFile(this, shaderLayout, "../assets/shaders/vert.spv", "../assets/shaders/frag.spv");

	basicMaterial = new Material(this, basicShader, { uniformBuffer, florenceTexture });

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
	delete florenceTexture;
}

void DemoApplication::update(ShadeApplicationFrameData frameData)
{
	// Get current window size
	Rect windowSize = getWindowSize();

	// Calculate mvp
	glm::mat4 model = glm::rotate(glm::mat4(1.0f), (frameData.timeSinceStartup / 10.0f) * glm::radians(360.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), windowSize.width / windowSize.height, 0.1f, 10.0f);
	uniformData.mvp = projection * view * model;

    uniformBuffer->setData(&uniformData);
}

void DemoApplication::render()
{
    renderMesh(indexBuffer, vertexBuffer, basicMaterial);
}
