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

	void createTextureSampler(UniformTextureFilterMode filterMode, uint32_t mipLevels = 1);
public:
	UniformTexture(VulkanApplication* app, UniformTexturePixelData pixelData, UniformTextureFilterMode filterMode = UniformTextureFilterMode::LINEAR, bool enableMipmaps = true);
	~UniformTexture();

	static UniformTexture* loadFromPath(VulkanApplication* app, std::string path, UniformTextureFilterMode filterMode = UniformTextureFilterMode::LINEAR, bool enableMipmaps = true);

	VkImageView _getTextureImageView();
	VkSampler _getTextureSampler();
};
}