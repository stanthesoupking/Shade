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

	void createTextureSampler();
public:
	UniformTexture(VulkanApplication* app, std::string path);
	~UniformTexture();

	VkImageView _getTextureImageView();
	VkSampler _getTextureSampler();
};
}