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

enum UniformTextureFilterMode
{
	LINEAR,
	NEAREST
};

class UniformTexture
{
private:
	VulkanApplicationData* vulkanData;

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	void createTextureSampler(UniformTextureFilterMode filterMode);
public:
	UniformTexture(VulkanApplication* app, UniformTexturePixelData pixelData, UniformTextureFilterMode filterMode = UniformTextureFilterMode::LINEAR);
	~UniformTexture();

	static UniformTexture* loadFromPath(VulkanApplication* app, std::string path, UniformTextureFilterMode filterMode = UniformTextureFilterMode::LINEAR);

	VkImageView _getTextureImageView();
	VkSampler _getTextureSampler();
};
}