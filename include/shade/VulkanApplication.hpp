/**
 * VulkanApplication specification:
 *  Sole purpose is to allow for transfer of Vulkan data throughout Shade
 *  objects.
 */

#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "./vendor/vk_mem_alloc.hpp"

namespace Shade
{
// Forward declaration of shader
class Shader;

struct VulkanApplicationData
{
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    
    uint32_t graphicsQueueFamilyIndex;
    VkQueue graphicsQueue;

    uint32_t presentQueueFamilyIndex;
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

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
    VkFormat depthImageFormat;

    VmaAllocator allocator;
    const VkAllocationCallbacks* allocationCallbacks;
};

class VulkanApplication
{
private:
protected:
    VulkanApplicationData vulkanData;

public:
    VulkanApplication(){};
    ~VulkanApplication(){};

    VulkanApplicationData *_getVulkanData()
    {
        return &this->vulkanData;
    };

    virtual void _registerShader(Shader *shader) = 0;
    virtual void _unregisterShader(Shader *shader) = 0;
    virtual std::vector<Shader *> _getShaders() = 0;

    // Various utility functions
    uint32_t _findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void _createImage(uint32_t width, uint32_t height,
                      VkFormat format, VkImageTiling tiling,
                      VkImageUsageFlags usage,
                      VkMemoryPropertyFlags properties,
                      VkImage &image, VkDeviceMemory &imageMemory);

    void _createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView);

    VkCommandBuffer _beginSingleTimeCommands();
    void _endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void _copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void _copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void _transitionImageLayout(VkImage image, VkFormat format,
                                VkImageLayout oldLayout,
                                VkImageLayout newLayout);
};
} // namespace Shade