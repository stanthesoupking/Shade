#include "shade/UniformTexture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "shade/vendor/stb_image.hpp"

#include "shade/Buffer.hpp"

#include <iostream>

using namespace Shade;

UniformTexture::UniformTexture(VulkanApplication* app, std::string path)
{
	this->vulkanData = app->_getVulkanData();

	int textureWidth, textureHeight, textureChannels;

	// Load image at path
	stbi_uc* pixels = stbi_load(path.c_str(), &textureWidth, &textureHeight,
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

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.flags = 0;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = textureWidth;
	imageInfo.extent.height = textureHeight;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	if (vkCreateImage(vulkanData->device, &imageInfo, nullptr, &textureImage) != VK_SUCCESS)
	{
		throw std::runtime_error("Shade: Failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(vulkanData->device, textureImage, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(vulkanData->device, &allocInfo, nullptr, &textureImageMemory)
		!= VK_SUCCESS)
	{
		throw std::runtime_error("Shade: Failed to allocate image memory!");
	}

	vkBindImageMemory(vulkanData->device, textureImage, textureImageMemory, 0);

	// Copy buffer data

	// Create temporary command buffer
	VkCommandBufferAllocateInfo commandAllocInfo = {};
	commandAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandAllocInfo.commandPool = vulkanData->commandPool;
	commandAllocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(vulkanData->device, &commandAllocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	// Transition image layout
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = textureImage;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	// Copy buffer to image
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		static_cast<uint32_t>(textureWidth),
		static_cast<uint32_t>(textureHeight),
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		stagingBuffer._getVkBuffer(),
		textureImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	// Transition image layout for read only sampling
	barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = textureImage;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	// Destroy temporary command buffer
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(vulkanData->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(vulkanData->graphicsQueue);

	vkFreeCommandBuffers(vulkanData->device, vulkanData->commandPool, 1, &commandBuffer);

	// Create image view
	createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM);

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

uint32_t UniformTexture::findMemoryType(uint32_t typeFilter,
	VkMemoryPropertyFlags properties)
{
	// Get available memory types
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(vulkanData->physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("Shade: Failed to find suitable memory type!");
}

void UniformTexture::createImageView(VkImage image, VkFormat format)
{
	// Create image view
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = textureImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(vulkanData->device, &viewInfo, nullptr, &textureImageView) != VK_SUCCESS) {
		throw std::runtime_error("Shade: Failed to create texture image view!");
	}
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

	if (vkCreateSampler(vulkanData->device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
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