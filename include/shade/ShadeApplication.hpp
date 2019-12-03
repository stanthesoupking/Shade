#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

#include <vector>
#include <optional>

#include "./VulkanApplication.hpp"
#include "./Buffer.hpp"
#include "./IndexBuffer.hpp"
#include "./VertexBuffer.hpp"
#include "./Rect.hpp"
#include "./Colour.hpp"
#include "./Shader.hpp"
#include "./Mesh.hpp"
#include "./Shade.hpp"

namespace Shade
{

struct Mouse
{
    glm::vec2 position;
    glm::vec2 movement; // Mouse movement since last frame
    bool leftButtonPressed;
    bool rightButtonPressed;
    bool middleButtonPressed;
};

enum Key
{
    SHADE_KEY_A = GLFW_KEY_A,
    SHADE_KEY_B = GLFW_KEY_B,
    SHADE_KEY_C = GLFW_KEY_C,
    SHADE_KEY_D = GLFW_KEY_D,
    SHADE_KEY_E = GLFW_KEY_E,
    SHADE_KEY_F = GLFW_KEY_F,
    SHADE_KEY_G = GLFW_KEY_G,
    SHADE_KEY_H = GLFW_KEY_H,
    SHADE_KEY_I = GLFW_KEY_I,
    SHADE_KEY_J = GLFW_KEY_J,
    SHADE_KEY_K = GLFW_KEY_K,
    SHADE_KEY_L = GLFW_KEY_L,
    SHADE_KEY_M = GLFW_KEY_M,
    SHADE_KEY_N = GLFW_KEY_N,
    SHADE_KEY_O = GLFW_KEY_O,
    SHADE_KEY_P = GLFW_KEY_P,
    SHADE_KEY_Q = GLFW_KEY_Q,
    SHADE_KEY_R = GLFW_KEY_R,
    SHADE_KEY_S = GLFW_KEY_S,
    SHADE_KEY_T = GLFW_KEY_T,
    SHADE_KEY_U = GLFW_KEY_U,
    SHADE_KEY_V = GLFW_KEY_V,
    SHADE_KEY_W = GLFW_KEY_W,
    SHADE_KEY_X = GLFW_KEY_X,
    SHADE_KEY_Y = GLFW_KEY_Y,
    SHADE_KEY_Z = GLFW_KEY_Z,
    SHADE_KEY_0 = GLFW_KEY_0,
    SHADE_KEY_1 = GLFW_KEY_1,
    SHADE_KEY_2 = GLFW_KEY_2,
    SHADE_KEY_3 = GLFW_KEY_3,
    SHADE_KEY_4 = GLFW_KEY_4,
    SHADE_KEY_5 = GLFW_KEY_5,
    SHADE_KEY_6 = GLFW_KEY_6,
    SHADE_KEY_7 = GLFW_KEY_7,
    SHADE_KEY_8 = GLFW_KEY_8,
    SHADE_KEY_9 = GLFW_KEY_9,
    SHADE_KEY_UP = GLFW_KEY_UP,
    SHADE_KEY_DOWN = GLFW_KEY_DOWN,
    SHADE_KEY_LEFT = GLFW_KEY_LEFT,
    SHADE_KEY_RIGHT = GLFW_KEY_RIGHT,
    SHADE_KEY_SPACE = GLFW_KEY_SPACE,
    SHADE_KEY_APOSTROPHE = GLFW_KEY_APOSTROPHE,
    SHADE_KEY_BACKSLASH = GLFW_KEY_BACKSLASH,
    SHADE_KEY_BACKSPACE = GLFW_KEY_BACKSPACE,
    SHADE_KEY_CAPS_LOCK = GLFW_KEY_CAPS_LOCK,
    SHADE_KEY_COMMA = GLFW_KEY_COMMA,
    SHADE_KEY_DELETE = GLFW_KEY_DELETE,
    SHADE_KEY_END = GLFW_KEY_END,
    SHADE_KEY_ENTER = GLFW_KEY_ENTER,
    SHADE_KEY_EQUAL = GLFW_KEY_EQUAL,
    SHADE_KEY_ESCAPE = GLFW_KEY_ESCAPE,
    SHADE_KEY_F1 = GLFW_KEY_F1,
    SHADE_KEY_F2 = GLFW_KEY_F2,
    SHADE_KEY_F3 = GLFW_KEY_F3,
    SHADE_KEY_F4 = GLFW_KEY_F4,
    SHADE_KEY_F5 = GLFW_KEY_F5,
    SHADE_KEY_F6 = GLFW_KEY_F6,
    SHADE_KEY_F7 = GLFW_KEY_F7,
    SHADE_KEY_F8 = GLFW_KEY_F8,
    SHADE_KEY_F9 = GLFW_KEY_F9,
    SHADE_KEY_F10 = GLFW_KEY_F10,
    SHADE_KEY_F11 = GLFW_KEY_F11,
    SHADE_KEY_F12 = GLFW_KEY_F12,
    SHADE_KEY_GRAVE_ACCENT = GLFW_KEY_GRAVE_ACCENT,
    SHADE_KEY_HOME = GLFW_KEY_HOME,
    SHADE_KEY_INSERT = GLFW_KEY_INSERT,
    SHADE_KEY_LEFT_ALT = GLFW_KEY_LEFT_ALT,
    SHADE_KEY_LEFT_BRACKET = GLFW_KEY_LEFT_BRACKET,
    SHADE_KEY_LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL,
    SHADE_KEY_LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT,
    SHADE_KEY_LEFT_SUPER = GLFW_KEY_LEFT_SUPER,
    SHADE_KEY_MENU = GLFW_KEY_MENU,
    SHADE_KEY_MINUS = GLFW_KEY_MINUS,
    SHADE_KEY_NUM_LOCK = GLFW_KEY_NUM_LOCK,
    SHADE_KEY_PAGE_DOWN = GLFW_KEY_PAGE_DOWN,
    SHADE_KEY_PAGE_UP = GLFW_KEY_PAGE_UP,
    SHADE_KEY_PAUSE = GLFW_KEY_PAUSE,
    SHADE_KEY_PERIOD = GLFW_KEY_PERIOD,
    SHADE_KEY_PRINT_SCREEN = GLFW_KEY_PRINT_SCREEN,
    SHADE_KEY_RIGHT_ALT = GLFW_KEY_RIGHT_ALT,
    SHADE_KEY_RIGHT_BRACKET = GLFW_KEY_RIGHT_BRACKET,
    SHADE_KEY_RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL,
    SHADE_KEY_RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT,
    SHADE_KEY_RIGHT_SUPER = GLFW_KEY_RIGHT_SUPER,
    SHADE_KEY_SCROLL_LOCK = GLFW_KEY_SCROLL_LOCK,
    SHADE_KEY_SEMICOLON = GLFW_KEY_SEMICOLON,
    SHADE_KEY_SLASH = GLFW_KEY_SLASH,
    SHADE_KEY_TAB = GLFW_KEY_TAB
};

struct ShadeApplicationFrameData
{
    float timeSinceStartup; // Time in seconds since application initialised
    float deltaTime; // Delta time since last frame
};

struct ShadeApplicationInfo
{
    std::string windowTitle = "Shade Application";
    Rect windowSize = {0, 0, 640, 480};
	bool windowResizable = true;
    bool windowFullscreen = false;
    Colour clearColour = {0, 0, 0, 1};
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsQueue;
    std::optional<uint32_t> presentQueue;

    bool isComplete()
    {
        return graphicsQueue.has_value() && presentQueue.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class ShadeApplication: public VulkanApplication
{
private:
    ShadeApplicationInfo info;

    GLFWwindow *window;

    ShadeApplicationFrameData previouseFrameData {
        0.0f, // Time since startup
        0.0f // Placeholder delta time
    };

    Mouse mouseData;
    Mouse previousMouseData;

    const std::vector<const char *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};

    const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    void initSystem();
    void initWindow();
    void initVulkan();

    // Vulkan init methods:
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool checkDeviceExtensionsSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    void createLogicalDevice();
    void createSwapchain();
    VkExtent2D getOptimalSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    VkPresentModeKHR getOptimalSwapPresentMode(
        const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkSurfaceFormatKHR getOptimalSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &availableFormats);
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();
    void createSemaphores();
    void createCommandPool();
    void createCommandBuffers();
    void createDescriptorPool();
    void createDepthResources();

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	void recreateSwapchain();
	void cleanupSwapchain();
    void cleanupDepthResources();

    ShadeApplicationFrameData getNextFrameData();

    void renderStart();
    void renderPresent();

    void updateMouseData();

	/**
	 * Keep track of loaded shaders for updating on window resize events.
	 */
	std::vector<Shader*> shaderRegistry;
public:
    ShadeApplication();
    ~ShadeApplication();

    // Start the application and enter main loop
    void start();

    // Overridable methods:
    virtual ShadeApplicationInfo preInit() = 0;
    virtual void init() = 0;
    virtual void update(ShadeApplicationFrameData frameData) = 0;
    virtual void render() = 0;
    virtual void destroy() = 0;

    void setRenderClearColour(Colour c);
	
	void setWindowSize(Rect windowSize);
	Rect getWindowSize();

    void setWindowTitle(std::string windowTitle);
    std::string getWindowTitle();

    void renderMesh(Mesh* mesh, Material *material);

	ShadeApplicationInfo* _getApplicationInfo();
	void _registerShader(Shader* shader);
	void _unregisterShader(Shader* shader);
	std::vector<Shader*> _getShaders();

    bool getKeyPressed(Key key);
    bool getKeyReleased(Key key);

    Mouse getMouse();
};
} // namespace Shade