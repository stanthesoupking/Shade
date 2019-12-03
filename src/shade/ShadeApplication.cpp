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
    vkDestroyDescriptorPool(vulkanData.device, vulkanData.descriptorPool, nullptr);

    vkDestroySemaphore(vulkanData.device, vulkanData.imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(vulkanData.device, vulkanData.renderFinishedSemaphore, nullptr);

    cleanupDepthResources();

    vkDestroyRenderPass(vulkanData.device, vulkanData.renderPass, nullptr);

    vkDestroyCommandPool(vulkanData.device, vulkanData.commandPool, nullptr);

    for (auto framebuffer : vulkanData.swapChainFramebuffers)
    {
        vkDestroyFramebuffer(vulkanData.device, framebuffer, nullptr);
    }

    for (auto imageView : vulkanData.swapChainImageViews)
    {
        vkDestroyImageView(vulkanData.device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(vulkanData.device, vulkanData.swapChain, nullptr);

    vkDestroyDevice(vulkanData.device, nullptr);

    vkDestroySurfaceKHR(vulkanData.instance, vulkanData.surface, nullptr);

    vkDestroyInstance(vulkanData.instance, nullptr);

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

        this->update(
            getNextFrameData());

        this->renderStart();
        this->render();
        this->renderPresent();
    }

    // Wait until all operations on GPU are complete
    vkDeviceWaitIdle(vulkanData.device);

    this->destroy();
}

ShadeApplicationFrameData ShadeApplication::getNextFrameData()
{
    float timeSinceStartup = glfwGetTime();
    float deltaTime = timeSinceStartup - previouseFrameData.timeSinceStartup;

    return {
        timeSinceStartup,
        deltaTime};
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

    // Check if window is resizable
    if (this->info.windowResizable)
    {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    }
    else
    {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    }

    GLFWmonitor* monitor;
    // Check if window should be fullscreen
    if (this->info.windowFullscreen)
    {
        monitor = glfwGetPrimaryMonitor();

        // Match window properties with the monitor's video mode
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

        // Match window size with the full size of the monitor
        this->info.windowSize = {0, 0, (float)mode->width, (float)mode->height};
    }
    else
    {
        monitor = nullptr;
    }
    
    window = glfwCreateWindow(this->info.windowSize.width,
                              this->info.windowSize.height,
                              this->info.windowTitle.c_str(), monitor, nullptr);

    glfwSetWindowUserPointer(window, this);

    // Setup callbacks
    if (this->info.windowResizable)
    {
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }
}

void ShadeApplication::initVulkan()
{
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createImageViews();
    createCommandPool();
    createDepthResources();
    createRenderPass();
    createFramebuffers();
    createSemaphores();
    createCommandBuffers();
    createDescriptorPool();
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

    if (vkCreateInstance(&createInfo, nullptr, &vulkanData.instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to create instance!");
    }
}

void ShadeApplication::createSurface()
{
    if (glfwCreateWindowSurface(vulkanData.instance, window, nullptr, &vulkanData.surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to create window surface!");
    }
}

void ShadeApplication::pickPhysicalDevice()
{
    vulkanData.physicalDevice = VK_NULL_HANDLE;

    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(vulkanData.instance, &physicalDeviceCount, nullptr);

    if (physicalDeviceCount == 0)
    {
        throw std::runtime_error(
            "Shade: Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> physicalDevices;
    physicalDevices.resize(physicalDeviceCount);
    vkEnumeratePhysicalDevices(vulkanData.instance, &physicalDeviceCount, physicalDevices.data());

    for (const VkPhysicalDevice tDevice : physicalDevices)
    {
        if (isDeviceSuitable(tDevice))
        {
            vulkanData.physicalDevice = tDevice;
            break;
        }
    }

    if (vulkanData.physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Shade: Failed to find a suitable GPU!");
    }
}

bool ShadeApplication::isDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

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

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    ;
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
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vulkanData.surface, &queuePresentSupport);

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

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vulkanData.surface,
                                              &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanData.surface, &formatCount,
                                         nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanData.surface, &formatCount,
                                             details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkanData.surface,
                                              &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkanData.surface,
                                                  &presentModeCount, details.presentModes.data());
    }

    return details;
}

void ShadeApplication::createLogicalDevice()
{

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    // Get queue create infos
    QueueFamilyIndices indices = findQueueFamilies(vulkanData.physicalDevice);

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

    if (vkCreateDevice(vulkanData.physicalDevice, &createInfo, nullptr, &vulkanData.device) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to create logical device!");
    }

    vkGetDeviceQueue(vulkanData.device, indices.graphicsQueue.value(), 0, &vulkanData.graphicsQueue);
    vkGetDeviceQueue(vulkanData.device, indices.presentQueue.value(), 0, &vulkanData.presentQueue);
}

void ShadeApplication::createSwapchain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(vulkanData.physicalDevice);

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
    createInfo.surface = vulkanData.surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = imageExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(vulkanData.physicalDevice);
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

    if (vkCreateSwapchainKHR(vulkanData.device, &createInfo, nullptr, &vulkanData.swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to create swap chain!");
    }

    // Get swapchain images
    vkGetSwapchainImagesKHR(vulkanData.device, vulkanData.swapChain, &imageCount, nullptr);
    vulkanData.swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(vulkanData.device, vulkanData.swapChain, &imageCount, vulkanData.swapChainImages.data());

    vulkanData.swapChainImageFormat = surfaceFormat.format;
    vulkanData.swapChainExtent = imageExtent;
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
    vulkanData.swapChainImageViews.resize(vulkanData.swapChainImages.size());

    for (size_t i = 0; i < vulkanData.swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = vulkanData.swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = vulkanData.swapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vulkanData.device, &createInfo, nullptr,
                              &vulkanData.swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Shade: Failed to create image views!");
        }
    }
}

void ShadeApplication::createRenderPass()
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = vulkanData.swapChainImageFormat;
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

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = vulkanData.depthImageFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

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

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(vulkanData.device, &renderPassInfo, nullptr,
                           &vulkanData.renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to create render pass!");
    }
}

void ShadeApplication::createFramebuffers()
{
    vulkanData.swapChainFramebuffers.resize(vulkanData.swapChainImageViews.size());

    for (size_t i = 0; i < vulkanData.swapChainImageViews.size(); i++)
    {
        std::array<VkImageView, 2> attachments = {
            vulkanData.swapChainImageViews[i],
            vulkanData.depthImageView};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = vulkanData.renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = vulkanData.swapChainExtent.width;
        framebufferInfo.height = vulkanData.swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vulkanData.device, &framebufferInfo, nullptr,
                                &vulkanData.swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Shade: Failed to create framebuffer!");
        }
    }
}

void ShadeApplication::createSemaphores()
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(vulkanData.device, &semaphoreInfo,
                          nullptr, &vulkanData.imageAvailableSemaphore) != VK_SUCCESS |
        vkCreateSemaphore(vulkanData.device, &semaphoreInfo,
                          nullptr, &vulkanData.renderFinishedSemaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create semaphores!");
    }
}

void ShadeApplication::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices =
        findQueueFamilies(vulkanData.physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex =
        queueFamilyIndices.graphicsQueue.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(vulkanData.device, &poolInfo, nullptr,
                            &vulkanData.commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void ShadeApplication::createCommandBuffers()
{
    vulkanData.commandBuffers.resize(vulkanData.swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vulkanData.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)vulkanData.commandBuffers.size();

    if (vkAllocateCommandBuffers(vulkanData.device, &allocInfo,
                                 vulkanData.commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to allocate command buffers!");
    }
}

void ShadeApplication::createDescriptorPool()
{
    VkDescriptorPoolSize poolSizes[] = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024}};

    VkDescriptorPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.maxSets = 1024; // PLACEHOLDER VALUE, research this
    createInfo.poolSizeCount = 2;
    createInfo.pPoolSizes = poolSizes;

    vkCreateDescriptorPool(vulkanData.device, &createInfo, nullptr, &vulkanData.descriptorPool);
}

void ShadeApplication::createDepthResources()
{
    // Create depth image
    vulkanData.depthImageFormat = VK_FORMAT_D32_SFLOAT; // Widely supported format

    _createImage(vulkanData.swapChainExtent.width,
                 vulkanData.swapChainExtent.height, vulkanData.depthImageFormat,
                 VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 vulkanData.depthImage, vulkanData.depthImageMemory);

    _createImageView(vulkanData.depthImage, vulkanData.depthImageFormat,
                     VK_IMAGE_ASPECT_DEPTH_BIT, vulkanData.depthImageView);

    _transitionImageLayout(vulkanData.depthImage, vulkanData.depthImageFormat,
                           VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void ShadeApplication::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    ShadeApplication *app = (ShadeApplication *)glfwGetWindowUserPointer(window);
    app->setWindowSize({0, 0, (float)width, (float)height});
}

void ShadeApplication::recreateSwapchain()
{
    vkDeviceWaitIdle(vulkanData.device);

    cleanupSwapchain();

    createSwapchain();
    createImageViews();
    createRenderPass();

    // Update shaders
    for (auto shader : shaderRegistry)
    {
        shader->_recreateGraphicsPipeline();
    }

    cleanupDepthResources();
    createDepthResources();

    createFramebuffers();
    createCommandBuffers();
}

void ShadeApplication::cleanupSwapchain()
{
    for (size_t i = 0; i < vulkanData.swapChainFramebuffers.size(); i++)
    {
        vkDestroyFramebuffer(vulkanData.device, vulkanData.swapChainFramebuffers[i], nullptr);
    }

    vkFreeCommandBuffers(vulkanData.device, vulkanData.commandPool, static_cast<uint32_t>(vulkanData.commandBuffers.size()), vulkanData.commandBuffers.data());

    vkDestroyRenderPass(vulkanData.device, vulkanData.renderPass, nullptr);

    for (size_t i = 0; i < vulkanData.swapChainImageViews.size(); i++)
    {
        vkDestroyImageView(vulkanData.device, vulkanData.swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(vulkanData.device, vulkanData.swapChain, nullptr);
}

void ShadeApplication::cleanupDepthResources()
{
    vkDestroyImage(vulkanData.device, vulkanData.depthImage, nullptr);
    vkDestroyImageView(vulkanData.device, vulkanData.depthImageView, nullptr);
    vkFreeMemory(vulkanData.device, vulkanData.depthImageMemory, nullptr);
}

void ShadeApplication::renderStart()
{
    // Get current image index
    vkAcquireNextImageKHR(vulkanData.device, vulkanData.swapChain, UINT64_MAX, vulkanData.imageAvailableSemaphore,
                          nullptr, &vulkanData.currentImageIndex);

    // Reset command buffer
    vkResetCommandBuffer(vulkanData.commandBuffers[vulkanData.currentImageIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

    // Begin recording command buffer
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(vulkanData.commandBuffers[vulkanData.currentImageIndex],
                             &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Shade: Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vulkanData.renderPass;
    renderPassInfo.framebuffer = vulkanData.swapChainFramebuffers[vulkanData.currentImageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = vulkanData.swapChainExtent;

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {info.clearColour.r, info.clearColour.g, info.clearColour.b, info.clearColour.a};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    // Begin render pass
    vkCmdBeginRenderPass(vulkanData.commandBuffers[vulkanData.currentImageIndex], &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
}

void ShadeApplication::renderPresent()
{
    // End render pass
    vkCmdEndRenderPass(vulkanData.commandBuffers[vulkanData.currentImageIndex]);

    if (vkEndCommandBuffer(vulkanData.commandBuffers[vulkanData.currentImageIndex]) != VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to record command buffer!");
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {vulkanData.imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] =
        {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vulkanData.commandBuffers[vulkanData.currentImageIndex];

    VkSemaphore signalSemaphores[] = {vulkanData.renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(vulkanData.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("Shade: Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {vulkanData.swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &vulkanData.currentImageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(vulkanData.presentQueue, &presentInfo);

    // Prevent CPU from submitting work faster than GPU
    //  can keep up with.
    // Wait for present operations to be finished.
    vkQueueWaitIdle(vulkanData.presentQueue);
}

void ShadeApplication::setRenderClearColour(Colour c)
{
    this->info.clearColour = c;
}

void ShadeApplication::setWindowSize(Rect windowSize)
{
    if ((windowSize.height > 1) | (windowSize.width > 1))
    {
        info.windowSize = windowSize;
        this->recreateSwapchain();
    }
}

Rect ShadeApplication::getWindowSize()
{
    return this->info.windowSize;
}

void ShadeApplication::setWindowTitle(std::string windowTitle)
{
    this->info.windowTitle = windowTitle;

    glfwSetWindowTitle(window, windowTitle.c_str());
}

std::string ShadeApplication::getWindowTitle()
{
    return this->info.windowTitle;
}

void ShadeApplication::renderMesh(Mesh *mesh, Material *material)
{
    IndexBuffer *indexBuffer = mesh->getIndexBuffer();
    VertexBuffer *vertexBuffer = mesh->getVertexBuffer();

    // Bind shader graphics pipeline
    vkCmdBindPipeline(vulkanData.commandBuffers[vulkanData.currentImageIndex],
                      VK_PIPELINE_BIND_POINT_GRAPHICS, material->getShader()->_getGraphicsPipeline());

    vkCmdBindIndexBuffer(vulkanData.commandBuffers[vulkanData.currentImageIndex], indexBuffer->_getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

    VkBuffer vertexBuffers[] = {vertexBuffer->_getVkBuffer()};
    VkDeviceSize vertexBufferOffsets[] = {0};
    vkCmdBindVertexBuffers(vulkanData.commandBuffers[vulkanData.currentImageIndex], 0, 1, vertexBuffers, vertexBufferOffsets);

    VkDescriptorSet descriptorSet = material->_getDescriptorSet();

    vkCmdBindDescriptorSets(vulkanData.commandBuffers[vulkanData.currentImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
                            material->getShader()->_getGraphicsPipelineLayout(),
                            0, 1, &descriptorSet, 0, nullptr);

    vkCmdDrawIndexed(vulkanData.commandBuffers[vulkanData.currentImageIndex], indexBuffer->getElementCount(), 1, 0, 0, 0);
}

ShadeApplicationInfo *ShadeApplication::_getApplicationInfo()
{
    return &this->info;
}

void ShadeApplication::_registerShader(Shader *shader)
{
    shaderRegistry.push_back(shader);
}

void ShadeApplication::_unregisterShader(Shader *shader)
{
    int registrySize = shaderRegistry.size();
    for (int i = 0; i < registrySize; i++)
    {
        if (shaderRegistry[i] == shader)
        {
            shaderRegistry.erase(shaderRegistry.begin() + i);
            return;
        }
    }

    throw std::runtime_error("Shade: Failed to unregister shader!");
}

std::vector<Shader *> ShadeApplication::_getShaders()
{
    return this->shaderRegistry;
}
