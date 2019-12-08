#include "DemoApplication.hpp"

#include <iostream>

ShadeApplicationInfo DemoApplication::preInit()
{
    ShadeApplicationInfo appInfo;
    appInfo.windowTitle = "Shade Examples - User Input";
    appInfo.windowSize = Rect(860, 640);
    appInfo.windowFullscreen = false;
    appInfo.clearColour = Colour(0.15f, 0.15f, 0.15f);
	appInfo.windowResizable = true;
    appInfo.mouseLock = true;

    return appInfo;
}

void DemoApplication::init()
{
    StructuredBufferLayout uniformMVPLayout = {
        {
            {"mvp", MAT4},
            {"mv", MAT4},
            {"m", MAT4}
        }
    };

    StructuredBufferLayout uniformLightingLayout = {
        {
            {"lightDirection", VEC3},
            {"lightColour", VEC3},
            {"ambientLighting", VEC3}
        }
    };

    StructuredBufferLayout vertexLayout = {
        {
            {"inPosition", VEC3, SHADE_FLAG_POSITION},
            {"inTexCoord", VEC2, SHADE_FLAG_TEXCOORD},
            {"inNormal", VEC3, SHADE_FLAG_NORMAL}
        }
    };
    
	ShaderLayout shaderLayout = {
		{
			{0, ShaderStage::VERTEX, uniformMVPLayout},
			{1, ShaderStage::FRAGMENT, UniformTextureLayout()},
            {2, ShaderStage::FRAGMENT, uniformLightingLayout}
		},
		vertexLayout
	};

    std::cout << "Loading mesh..." << std::endl;

    mesh = Mesh::loadFromOBJ(this, "assets/models/cube.obj", vertexLayout);

	std::cout << "Loading texture..." << std::endl;

	texture = new UniformTexture(this, "assets/textures/florence.png");

    std::cout << "Creating uniform buffers..." << std::endl;

    // Create MVP buffer
	uniformMVP = {
        glm::mat4(1.0f),    // mvp
        glm::mat4(1.0f),    // mv
        glm::mat4(1.0f)     // m
    };

    uniformMVPBuffer = new StructuredUniformBuffer(this, uniformMVPLayout, &uniformMVP);

    // Create lighting buffer
    uniformLighting = {
        glm::vec3(0.0f, 1.0f, 0.0f),    // lightingDirection
        glm::vec3(1.0f, 1.0f, 1.0f),    // lightingColour
        glm::vec3(0.4f, 0.4f, 0.4f)     // ambientLighting
    };

    uniformLightingBuffer = new StructuredUniformBuffer(this, uniformLightingLayout, &uniformLighting);

    std::cout << "Loading shader..." << std::endl;

    basicShader = Shader::loadFromSPIRV(this, shaderLayout, "assets/shaders/vert.spv", "assets/shaders/frag.spv");

	basicMaterial = new Material(this, basicShader, { uniformMVPBuffer, texture, uniformLightingBuffer });

    cubeRotation = {0.0f, 0.0f};
}

void DemoApplication::destroy()
{
    // Cleanup
    delete basicMaterial;
    delete basicShader;

    delete texture;
    delete mesh;
    delete uniformMVPBuffer;
    delete uniformLightingBuffer;
}

void DemoApplication::update(ShadeApplicationFrameData frameData)
{
    // Check if escape button has been pressed
    if (getKeyPressed(SHADE_KEY_ESCAPE))
        exit();

	// Get current window size
	Rect windowSize = getWindowSize();

    // Update cube rotation
    Mouse mouse = getMouse();
    cubeRotation.x += mouse.movement.x;
    cubeRotation.y += mouse.movement.y;

	// Calculate mvp
	glm::mat4 model = 
        glm::rotate(glm::mat4(1.0f), cubeRotation.x * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::rotate(glm::mat4(1.0f), cubeRotation.y * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), windowSize.width / windowSize.height, 0.1f, 100.0f);
    uniformMVP.m = model;
    uniformMVP.mv = view * model;
	uniformMVP.mvp = projection * uniformMVP.mv;

    uniformMVPBuffer->setData(&uniformMVP);
}

void DemoApplication::render()
{
    renderMesh(mesh, basicMaterial);
}
