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
};
} // namespace Shade