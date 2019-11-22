#include "shade/ShadeApplication.hpp"

#include <iostream>
#include <set>
#include <algorithm>

using namespace Shade;

ShadeApplication::ShadeApplication()
{
}

ShadeApplication::~ShadeApplication()
{
    // Clean up internal variables
    vkDestroyCommandPool(device, commandPool, nullptr);

    for (auto framebuffer : swapChainFramebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    for (auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);

    vkDestroyDevice(device, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}

void ShadeApplication::start()
{
    // Get application info
    this->info = this->preInit();

    // Initialise system
    this->initSystem();

    // Initialise user-variables
    this->init();

    // Enter main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        this->update();

        this->renderStart();
        this->render();
        this->renderPresent();
    }

    // Wait until all operations on GPU are complete
    vkDeviceWaitIdle(device);

    this->destroy();
}

void ShadeApplication::initSystem()
{
    initWindow();
    initVulkan();
}

void ShadeApplication::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(this->info.windowSize.width, this->info.windowSize.height, "Shade Application", nullptr, nullptr);
}

void ShadeApplication::initVulkan()
{
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createImageViews();
    createRenderPass();
    createFramebuffers();
    createSemaphores();
    createCommandPool();
    createCommandBuffers();
}

void ShadeApplication::createInstance()
{
    VkApplicationInfo applicationInfo;
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = nullptr;
    applicationInfo.pEngineName = "Shade";
    applicationInfo.engineVersion = VK_MAKE_VERSION(SHADE_VERSION_MAJOR, SHADE_VERSION_MINOR, SHADE_VERSION_PATCH);

    // TODO: Get application info from user
    applicationInfo.pApplicationName = "Shade Application";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &applicationInfo;

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    if (_SHADE_ENABLE_VALIDATION_LAYERS)
    {
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(validationLayers.size());

        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to create instance!");
    }
}

void ShadeApplication::createSurface()
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to create window surface!");
    }
}

void ShadeApplication::pickPhysicalDevice()
{
    physicalDevice = VK_NULL_HANDLE;

    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

    if (physicalDeviceCount == 0)
    {
        throw std::runtime_error(
            "Shade: Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> physicalDevices;
    physicalDevices.resize(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

    for (const VkPhysicalDevice tDevice : physicalDevices)
    {
        if (isDeviceSuitable(tDevice))
        {
            this->physicalDevice = tDevice;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Shade: Failed to find a suitable GPU!");
    }
}

bool ShadeApplication::isDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionsSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport =
            querySwapChainSupport(device);

        swapChainAdequate = !swapChainSupport.formats.empty() &&
                            !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

QueueFamilyIndices ShadeApplication::findQueueFamilies(VkPhysicalDevice device)
{
    uint32_t queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies;
    queueFamilies.resize(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, queueFamilies.data());

    QueueFamilyIndices indices;

    int i = 0;
    for (const auto queueFamily : queueFamilies)
    {
        if (indices.isComplete())
            break;

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsQueue = i;
        }

        VkBool32 queuePresentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &queuePresentSupport);

        if (queuePresentSupport)
        {
            indices.presentQueue = i;
        }

        i++;
    }

    return indices;
}

bool ShadeApplication::checkDeviceExtensionsSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         availableExtensions.data());

    // Create set from device extensions
    std::set<std::string>
        requiredExtensions(deviceExtensions.begin(),
                           deviceExtensions.end());

    // Erase extensions that are available
    for (const auto &extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails ShadeApplication::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                              &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                             details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                              &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                                  &presentModeCount, details.presentModes.data());
    }

    return details;
}

void ShadeApplication::createLogicalDevice()
{

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    // Get queue create infos
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsQueue.value(),
        indices.presentQueue.value()};

    float priority = 1.0f;
    for (uint32_t index : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext = nullptr;
        queueCreateInfo.flags = 0;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.queueFamilyIndex = index;
        queueCreateInfo.pQueuePriorities = &priority; // Use default priority

        queueCreateInfos.push_back(queueCreateInfo);
    }

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (_SHADE_ENABLE_VALIDATION_LAYERS)
    {
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsQueue.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentQueue.value(), 0, &presentQueue);
}

void ShadeApplication::createSwapchain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    // Check that image count is not exceeding the maximum supported value
    if ((swapChainSupport.capabilities.maxImageCount > 0) && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // Choose optimal image extent
    VkExtent2D imageExtent = getOptimalSwapExtent(swapChainSupport.capabilities);

    // Choose optimal surface format
    VkSurfaceFormatKHR surfaceFormat = getOptimalSwapSurfaceFormat(swapChainSupport.formats);

    // Choose optimal present mode
    VkPresentModeKHR presentMode = getOptimalSwapPresentMode(swapChainSupport.presentModes);

    VkSwapchainCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = imageExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsQueue.value(),
                                     indices.presentQueue.value()};

    if (indices.graphicsQueue != indices.presentQueue)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to create swap chain!");
    }

    // Get swapchain images
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = imageExtent;
}

VkExtent2D ShadeApplication::getOptimalSwapExtent(const VkSurfaceCapabilitiesKHR &
                                                      capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        VkExtent2D actualExtent = {static_cast<uint32_t>(this->info.windowSize.width),
                                   static_cast<uint32_t>(this->info.windowSize.height)};

        // Get most optimal dimensions for extent
        actualExtent.width =
            std::max(capabilities.minImageExtent.width,
                     std::min(capabilities.maxImageExtent.width,
                              actualExtent.width));

        actualExtent.height =
            std::max(capabilities.minImageExtent.height,
                     std::min(capabilities.maxImageExtent.height,
                              actualExtent.height));

        return actualExtent;
    }
}

VkPresentModeKHR ShadeApplication::getOptimalSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR ShadeApplication::getOptimalSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace ==
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    // Just settle for first format specified if no optimal format exists
    return availableFormats[0];
}

void ShadeApplication::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr,
                              &swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Shade: Failed to create image views!");
        }
    }
}

void ShadeApplication::createRenderPass()
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr,
                           &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to create render pass!");
    }
}

void ShadeApplication::createFramebuffers()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        VkImageView attachments[] = {swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType =
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr,
                                &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Shade: Failed to create framebuffer!");
        }
    }
}

void ShadeApplication::createSemaphores()
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(device, &semaphoreInfo,
                          nullptr, &imageAvailableSemaphore) != VK_SUCCESS |
        vkCreateSemaphore(device, &semaphoreInfo,
                          nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create semaphores!");
    }
}

void ShadeApplication::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices =
        findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex =
        queueFamilyIndices.graphicsQueue.value();
    poolInfo.flags = 0;

    if (vkCreateCommandPool(device, &poolInfo, nullptr,
                            &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void ShadeApplication::createCommandBuffers()
{
    commandBuffers.resize(swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo,
                                 commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to allocate command buffers!");
    }
}

void ShadeApplication::renderStart()
{
    // Get current image index
    vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore,
                          nullptr, &currentImageIndex);

    // Reset command buffer
    vkResetCommandBuffer(commandBuffers[currentImageIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

    // Begin recording command buffer
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffers[currentImageIndex],
                             &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Shade: Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[currentImageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor = {info.clearColour.r, info.clearColour.g, info.clearColour.b, info.clearColour.a};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    // Begin render pass
    vkCmdBeginRenderPass(commandBuffers[currentImageIndex], &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
}

void ShadeApplication::renderPresent()
{
    // End render pass
    vkCmdEndRenderPass(commandBuffers[currentImageIndex]);

    if (vkEndCommandBuffer(commandBuffers[currentImageIndex]) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to record command buffer!");
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] =
        {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentImageIndex];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &currentImageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(presentQueue, &presentInfo);

    // Prevent CPU from submitting work faster than GPU
    //  can keep up with.
    // Wait for present operations to be finished.
    vkQueueWaitIdle(presentQueue);
}

void ShadeApplication::setRenderClearColour(Colour c)
{
    this->info.clearColour = c;
}

void ShadeApplication::renderMesh(Buffer *indexBuffer, Buffer *vertexBuffer, Material *material)
{
    // Bind shader graphics pipeline
    vkCmdBindPipeline(commandBuffers[currentImageIndex],
                      VK_PIPELINE_BIND_POINT_GRAPHICS, material->getShader()->_getGraphicsPipeline());

    vkCmdBindIndexBuffer(commandBuffers[currentImageIndex], indexBuffer->_getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

    VkBuffer vertexBuffers[] = {vertexBuffer->_getVkBuffer()};
    VkDeviceSize vertexBufferOffsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffers[currentImageIndex], 0, 1, vertexBuffers, vertexBufferOffsets);

    // NOTE: placeholder values
    vkCmdDrawIndexed(commandBuffers[currentImageIndex], 3, 1, 0, 0, 0);
    //vkCmdDraw(commandBuffers[currentImageIndex], 3, 1, 0, 0);
}

Buffer *ShadeApplication::createBuffer(void *data, uint32_t size)
{
    return new Buffer(physicalDevice, device, data, size);
}

Shader *ShadeApplication::createShaderFromSPIRVFile(ShaderLayout shaderLayout, const char *vertPath, const char *fragPath)
{
    return Shader::FromSPIRVFile(
        device, swapChainExtent, renderPass, shaderLayout, vertPath, fragPath);
}