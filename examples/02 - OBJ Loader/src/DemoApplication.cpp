#include "DemoApplication.hpp"

#include <iostream>

ShadeApplicationInfo DemoApplication::preInit()
{
    ShadeApplicationInfo appInfo;
    appInfo.windowTitle = "Shade Examples - OBJ Loader";
    appInfo.windowSize = Rect(860, 640);
    appInfo.windowFullscreen = false;
    appInfo.clearColour = Colour(0.15f, 0.15f, 0.15f);
	appInfo.windowResizable = true;

    return appInfo;
}

void DemoApplication::init()
{
    StructuredBufferLayout uniformDataLayout = StructuredBufferLayout(
        {{"mvp", MAT4}});

    StructuredBufferLayout vertexLayout = Mesh::baseLayout;
	
	ShaderLayout shaderLayout = {
		{
			{0, ShaderStage::VERTEX, uniformDataLayout},
			{1, ShaderStage::FRAGMENT, UniformTextureLayout()}
		},
		vertexLayout
	};

    std::cout << "Loading mesh..." << std::endl;

    mesh = Mesh::loadFromOBJ(this, "assets/models/cube.obj");

	std::cout << "Loading texture..." << std::endl;

	texture = new UniformTexture(this, "assets/textures/florence.png");

    std::cout << "Creating uniform buffer..." << std::endl;

	uniformData = {
        glm::mat4(1.0f)
    };

    uniformBuffer = new StructuredUniformBuffer(this, uniformDataLayout, &uniformData);

    std::cout << "Loading shader..." << std::endl;

    basicShader = Shader::loadFromSPIRV(this, shaderLayout, "assets/shaders/vert.spv", "assets/shaders/frag.spv");

	basicMaterial = new Material(this, basicShader, { uniformBuffer, texture });
}

void DemoApplication::destroy()
{
    // Cleanup
    delete basicMaterial;
    delete basicShader;

    delete texture;
    delete mesh;
    delete uniformBuffer;
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
    renderMesh(mesh, basicMaterial);
}
