/**
 * VulkanApplication specification:
 *  Sole purpose is to allow for transfer of Vulkan data throughout Shade
 *  objects.
 */

#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace Shade
{
// Forward declaration of shader
class Shader;

struct VulkanApplicationData
{
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

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

    VkDescriptorPool descriptorPool;

    // Current image being rendered to
    uint32_t currentImageIndex;
    VkCommandBuffer currentCommandBuffer;
};

class VulkanApplication
{
private:
protected:
    VulkanApplicationData vulkanData;
public:
    VulkanApplication() {};
    ~VulkanApplication() {};

    VulkanApplicationData* _getVulkanData()
    {
        return &this->vulkanData;
    };

	virtual void _registerShader(Shader* shader) = 0;
	virtual void _unregisterShader(Shader* shader) = 0;
	virtual std::vector<Shader*> _getShaders() = 0;
};
} // namespace Shade