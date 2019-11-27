#pragma once

#include "./VulkanApplication.hpp"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace Shade
{
class UniformTextureLayout
{
private:
public:
	UniformTextureLayout() {}
	~UniformTextureLayout() {}
};

class UniformTexture
{
private:
	VulkanApplicationData* vulkanData;

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	uint32_t findMemoryType(uint32_t typeFilter,
		VkMemoryPropertyFlags properties);

	void createImageView(VkImage image, VkFormat format);
	void createTextureSampler();
public:
	UniformTexture(VulkanApplication* app, std::string path);
	~UniformTexture();

	VkImageView _getTextureImageView();
	VkSampler _getTextureSampler();
};
}