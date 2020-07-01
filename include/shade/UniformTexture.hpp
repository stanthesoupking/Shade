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

struct UniformTexturePixelData
{
	void *pixels;
	int width;
	int height;
	int channels;
	int bpp; // Bits per pixel
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
	UniformTexture(VulkanApplication* app, UniformTexturePixelData pixelData);
	~UniformTexture();

	static UniformTexture* loadFromPath(VulkanApplication* app, std::string path);

	VkImageView _getTextureImageView();
	VkSampler _getTextureSampler();
};
}