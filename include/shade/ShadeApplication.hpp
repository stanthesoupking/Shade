#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <optional>

#include "./Shade.hpp"

namespace Shade
{
struct ShadeApplicationInfo
{
    Rect windowSize = {0, 0, 640, 480};
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

class ShadeApplication
{
private:
    ShadeApplicationInfo info;

    GLFWwindow *window;

    const std::vector<const char *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};

    const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

    // Current image being rendered to
    uint32_t currentImageIndex;
    VkCommandBuffer currentCommandBuffer;

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

    void renderStart();
    void renderPresent();

public:
    ShadeApplication();
    ~ShadeApplication();

    // Start the application and enter main loop
    void start();

    // Overridable methods:
    virtual ShadeApplicationInfo preInit() = 0;
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void render() = 0;
    virtual void destroy() = 0;

    void setRenderClearColour(Colour c);
    
    void renderMesh(Buffer* indexBuffer, Buffer* vertexBuffer, Material* material);

    Buffer *createBuffer(void *data, uint32_t elementSize, uint32_t elementCount);
    Shader *createShaderFromSPIRVFile(ShaderLayout shaderLayout, const char *vertPath, const char *fragPath);
};
} // namespace Shade