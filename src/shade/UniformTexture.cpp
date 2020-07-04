#include "shade/UniformTexture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "shade/vendor/stb_image.hpp"

#include "shade/Buffer.hpp"

#include <iostream>
#include <cmath>
#include <algorithm>

using namespace Shade;

UniformTexture::UniformTexture(VulkanApplication *app, UniformTexturePixelData pixelData, UniformTextureFilterMode filterMode, bool enableMipmaps)
{
	this->vulkanData = app->_getVulkanData();

	uint32_t stride = pixelData.width * pixelData.height * 4;

	// Create staging buffer
	Buffer stagingBuffer = Buffer(app, pixelData.pixels, stride, 1, TRANSFER);

	uint32_t mipLevels = 1;
	VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	if (enableMipmaps)
	{
		// Calculate mip levels
		mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(pixelData.width, pixelData.height)))) + 1;

		usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	app->_createImage(pixelData.width, pixelData.height,
					  VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
					  usageFlags,
					  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, mipLevels);

	app->_transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
	app->_copyBufferToImage(stagingBuffer._getVkBuffer(), textureImage, static_cast<uint32_t>(pixelData.width), static_cast<uint32_t>(pixelData.height));
	if (enableMipmaps)
	{
		app->_generateMipmaps(textureImage, pixelData.width, pixelData.height, mipLevels);
	}
	else
	{
		app->_transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	// Create image view
	app->_createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, textureImageView, mipLevels);

	// Create texture sampler
	createTextureSampler(filterMode, mipLevels);
}

UniformTexture::~UniformTexture()
{
	vkDestroySampler(vulkanData->device, textureSampler, nullptr);
	vkDestroyImageView(vulkanData->device, textureImageView, nullptr);
	vkDestroyImage(vulkanData->device, textureImage, nullptr);
	vkFreeMemory(vulkanData->device, textureImageMemory, nullptr);
}

UniformTexture *UniformTexture::loadFromPath(VulkanApplication *app, std::string path, UniformTextureFilterMode filterMode, bool enableMipmaps)
{
	UniformTexturePixelData pixelData = {};

	// Load image at path
	pixelData.pixels = stbi_load(path.c_str(), &pixelData.width, &pixelData.height,
								 &pixelData.channels, STBI_rgb_alpha);

	if (!pixelData.pixels)
	{
		throw std::runtime_error("Shade: Failed to load uniform texture!");
	}

	// Create texture
	UniformTexture *texture = new UniformTexture(app, pixelData, filterMode, enableMipmaps);

	// Free original image
	stbi_image_free(pixelData.pixels);

	return texture;
}

void UniformTexture::createTextureSampler(UniformTextureFilterMode filterMode, uint32_t mipLevels)
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.flags = 0;
	samplerInfo.pNext = nullptr;

	switch (filterMode)
	{
	case LINEAR:
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		break;
	case NEAREST:
		samplerInfo.magFilter = VK_FILTER_NEAREST;
		samplerInfo.minFilter = VK_FILTER_NEAREST;
		break;
	}

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;

	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(mipLevels);

	if (vkCreateSampler(vulkanData->device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Shade: Failed to create texture sampler!");
	}
}

VkImageView UniformTexture::_getTextureImageView()
{
	return this->textureImageView;
}

VkSampler UniformTexture::_getTextureSampler()
{
	return this->textureSampler;
}