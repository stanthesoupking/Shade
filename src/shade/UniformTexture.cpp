#include "shade/UniformTexture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "shade/vendor/stb_image.hpp"

#include "shade/Buffer.hpp"

#include <iostream>

using namespace Shade;

UniformTexture::UniformTexture(VulkanApplication *app, std::string path)
{
	this->vulkanData = app->_getVulkanData();

	int textureWidth, textureHeight, textureChannels;

	// Load image at path
	stbi_uc *pixels = stbi_load(path.c_str(), &textureWidth, &textureHeight,
								&textureChannels, STBI_rgb_alpha);

	if (!pixels)
	{
		throw std::runtime_error("Shade: Failed to load uniform texture!");
	}

	uint32_t stride = textureWidth * textureHeight * 4;

	// Create staging buffer
	Buffer stagingBuffer = Buffer(app, pixels, stride, 1, TRANSFER);

	// Free original image
	stbi_image_free(pixels);


	app->_createImage(textureWidth, textureHeight,
					  VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
					  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	app->_transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	app->_copyBufferToImage(stagingBuffer._getVkBuffer(), textureImage, static_cast<uint32_t>(textureWidth), static_cast<uint32_t>(textureHeight));
	app->_transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// Create image view
	app->_createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, textureImageView);

	// Create texture sampler
	createTextureSampler();
}

UniformTexture::~UniformTexture()
{
	vkDestroySampler(vulkanData->device, textureSampler, nullptr);
	vkDestroyImageView(vulkanData->device, textureImageView, nullptr);
	vkDestroyImage(vulkanData->device, textureImage, nullptr);
	vkFreeMemory(vulkanData->device, textureImageMemory, nullptr);
}

void UniformTexture::createTextureSampler()
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.flags = 0;
	samplerInfo.pNext = nullptr;

	// TODO: Allow for these properties to be specified
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
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
	samplerInfo.maxLod = 0.0f;

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