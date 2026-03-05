#include "vk_builder.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>

namespace svrog::renderer {

InstanceBuilder& InstanceBuilder::setAppName(const std::string& name) {
    appName_ = name;
    return *this;
}

InstanceBuilder& InstanceBuilder::setEngineName(const std::string& name) {
    engineName_ = name;
    return *this;
}

InstanceBuilder& InstanceBuilder::addExtensions(const std::vector<const char*>& extensions) {
    extensions_.insert(extensions_.end(), extensions.begin(), extensions.end());
    return *this;
}

InstanceBuilder& InstanceBuilder::addValidationLayers(const std::vector<const char*>& layers) {
    validationLayers_.insert(validationLayers_.end(), layers.begin(), layers.end());
    return *this;
}

InstanceBuilder& InstanceBuilder::enableValidation(bool enable) {
    enableValidation_ = enable;
    if (enable && validationLayers_.empty()) {
        validationLayers_.push_back("VK_LAYER_KHRONOS_validation");
    }
    return *this;
}

void InstanceBuilder::build(VkCtx& ctx) {
    vk::ApplicationInfo appInfo{
        appName_.c_str(),
        VK_MAKE_VERSION(1, 0, 0),
        engineName_.c_str(),
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_3
    };

    vk::InstanceCreateInfo createInfo{
        {},
        &appInfo,
        enableValidation_ ? static_cast<uint32_t>(validationLayers_.size()) : 0,
        enableValidation_ ? validationLayers_.data() : nullptr,
        static_cast<uint32_t>(extensions_.size()),
        extensions_.data()
    };

    ctx.instance = vk::createInstance(createInfo);
    std::cout << "[InstanceBuilder] Vulkan instance created\n";
}

SurfaceBuilder& SurfaceBuilder::setSurfaceCreator(SurfaceCreatorFn creator) {
    surfaceCreator_ = std::move(creator);
    return *this;
}

void SurfaceBuilder::build(VkCtx& ctx) {
    if (!surfaceCreator_) {
        throw std::runtime_error("SurfaceBuilder: No surface creator function set");
    }
    VkSurfaceKHR rawSurface = surfaceCreator_(static_cast<VkInstance>(ctx.instance));
    ctx.surface = vk::SurfaceKHR(rawSurface);
    std::cout << "[SurfaceBuilder] Surface created\n";
}

DeviceBuilder& DeviceBuilder::requireGraphicsQueue(bool require) {
    requireGraphics_ = require;
    return *this;
}

DeviceBuilder& DeviceBuilder::requirePresentQueue(bool require) {
    requirePresent_ = require;
    return *this;
}

DeviceBuilder& DeviceBuilder::addDeviceExtensions(const std::vector<const char*>& extensions) {
    deviceExtensions_.insert(deviceExtensions_.end(), extensions.begin(), extensions.end());
    return *this;
}

QueueFamilyIndices DeviceBuilder::findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
    QueueFamilyIndices indices;
    auto queueFamilies = device.getQueueFamilyProperties();

    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
        }

        if (device.getSurfaceSupportKHR(i, surface)) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) break;
        i++;
    }

    return indices;
}

bool DeviceBuilder::checkDeviceExtensionSupport(vk::PhysicalDevice device,
                                                 const std::vector<const char*>& extensions) {
    auto availableExtensions = device.enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool DeviceBuilder::isDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface,
                                      const std::vector<const char*>& extensions) {
    auto indices = findQueueFamilies(device, surface);
    bool extensionsSupported = checkDeviceExtensionSupport(device, extensions);

    bool swapchainAdequate = false;
    if (extensionsSupported) {
        auto formats = device.getSurfaceFormatsKHR(surface);
        auto presentModes = device.getSurfacePresentModesKHR(surface);
        swapchainAdequate = !formats.empty() && !presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapchainAdequate;
}

void DeviceBuilder::build(VkCtx& ctx) {
    auto physicalDevices = ctx.instance.enumeratePhysicalDevices();
    if (physicalDevices.empty()) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support");
    }

    for (const auto& device : physicalDevices) {
        if (isDeviceSuitable(device, ctx.surface, deviceExtensions_)) {
            ctx.physicalDevice = device;
            break;
        }
    }

    if (!ctx.physicalDevice) {
        throw std::runtime_error("Failed to find a suitable GPU");
    }

    auto props = ctx.physicalDevice.getProperties();
    std::cout << "[DeviceBuilder] Selected GPU: " << props.deviceName << "\n";

    ctx.queueFamilyIndices = findQueueFamilies(ctx.physicalDevice, ctx.surface);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        ctx.queueFamilyIndices.graphicsFamily.value(),
        ctx.queueFamilyIndices.presentFamily.value()
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo{{}, queueFamily, 1, &queuePriority};
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};

    vk::DeviceCreateInfo createInfo{
        {},
        static_cast<uint32_t>(queueCreateInfos.size()),
        queueCreateInfos.data(),
        0, nullptr,
        static_cast<uint32_t>(deviceExtensions_.size()),
        deviceExtensions_.data(),
        &deviceFeatures
    };

    ctx.device = ctx.physicalDevice.createDevice(createInfo);
    ctx.graphicsQueue = ctx.device.getQueue(ctx.queueFamilyIndices.graphicsFamily.value(), 0);
    ctx.presentQueue = ctx.device.getQueue(ctx.queueFamilyIndices.presentFamily.value(), 0);

    std::cout << "[DeviceBuilder] Logical device created\n";
}

SwapchainBuilder& SwapchainBuilder::setPreferredFormat(vk::Format format) {
    preferredFormat_ = format;
    return *this;
}

SwapchainBuilder& SwapchainBuilder::setPreferredPresentMode(vk::PresentModeKHR mode) {
    preferredPresentMode_ = mode;
    return *this;
}

SwapchainBuilder& SwapchainBuilder::setExtent(uint32_t width, uint32_t height) {
    width_ = width;
    height_ = height;
    return *this;
}

SwapchainSupportDetails SwapchainBuilder::querySwapchainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
    SwapchainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
    details.formats = device.getSurfaceFormatsKHR(surface);
    details.presentModes = device.getSurfacePresentModesKHR(surface);
    return details;
}

vk::SurfaceFormatKHR SwapchainBuilder::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR>& formats, vk::Format preferred) {
    for (const auto& format : formats) {
        if (format.format == preferred && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return format;
        }
    }
    return formats[0];
}

vk::PresentModeKHR SwapchainBuilder::chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR>& modes, vk::PresentModeKHR preferred) {
    for (const auto& mode : modes) {
        if (mode == preferred) return mode;
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D SwapchainBuilder::chooseSwapExtent(
    const vk::SurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    vk::Extent2D actualExtent = {width, height};
    actualExtent.width = std::clamp(actualExtent.width,
        capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height,
        capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actualExtent;
}

void SwapchainBuilder::build(VkCtx& ctx) {
    auto swapchainSupport = querySwapchainSupport(ctx.physicalDevice, ctx.surface);

    auto surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats, preferredFormat_);
    auto presentMode = chooseSwapPresentMode(swapchainSupport.presentModes, preferredPresentMode_);
    auto extent = chooseSwapExtent(swapchainSupport.capabilities, width_, height_);

    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapchainSupport.capabilities.maxImageCount) {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo{
        {},
        ctx.surface,
        imageCount,
        surfaceFormat.format,
        surfaceFormat.colorSpace,
        extent,
        1,
        vk::ImageUsageFlagBits::eColorAttachment
    };

    uint32_t queueFamilyIndices[] = {
        ctx.queueFamilyIndices.graphicsFamily.value(),
        ctx.queueFamilyIndices.presentFamily.value()
    };

    if (ctx.queueFamilyIndices.graphicsFamily != ctx.queueFamilyIndices.presentFamily) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    ctx.swapchain = ctx.device.createSwapchainKHR(createInfo);
    ctx.swapchainImages = ctx.device.getSwapchainImagesKHR(ctx.swapchain);
    ctx.swapchainImageFormat = surfaceFormat.format;
    ctx.swapchainExtent = extent;

    ctx.swapchainImageViews.resize(ctx.swapchainImages.size());
    for (size_t i = 0; i < ctx.swapchainImages.size(); i++) {
        vk::ImageViewCreateInfo viewInfo{
            {},
            ctx.swapchainImages[i],
            vk::ImageViewType::e2D,
            ctx.swapchainImageFormat,
            {},
            {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
        };
        ctx.swapchainImageViews[i] = ctx.device.createImageView(viewInfo);
    }

    std::cout << "[SwapchainBuilder] Swapchain created (" << extent.width << "x" << extent.height << ")\n";
}

void RenderPassBuilder::build(VkCtx& ctx) {
    vk::AttachmentDescription colorAttachment{
        {},
        ctx.swapchainImageFormat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR
    };

    vk::AttachmentReference colorAttachmentRef{0, vk::ImageLayout::eColorAttachmentOptimal};

    vk::SubpassDescription subpass{
        {},
        vk::PipelineBindPoint::eGraphics,
        0, nullptr,
        1, &colorAttachmentRef
    };

    vk::SubpassDependency dependency{
        VK_SUBPASS_EXTERNAL, 0,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        {},
        vk::AccessFlagBits::eColorAttachmentWrite
    };

    vk::RenderPassCreateInfo renderPassInfo{
        {},
        1, &colorAttachment,
        1, &subpass,
        1, &dependency
    };

    ctx.renderPass = ctx.device.createRenderPass(renderPassInfo);
    std::cout << "[RenderPassBuilder] Render pass created\n";
}

PipelineBuilder& PipelineBuilder::setVertexShaderPath(const std::string& path) {
    vertexShaderPath_ = path;
    return *this;
}

PipelineBuilder& PipelineBuilder::setFragmentShaderPath(const std::string& path) {
    fragmentShaderPath_ = path;
    return *this;
}

PipelineBuilder& PipelineBuilder::setVertexShaderCode(const std::vector<uint32_t>& code) {
    vertexShaderCode_ = code;
    return *this;
}

PipelineBuilder& PipelineBuilder::setFragmentShaderCode(const std::vector<uint32_t>& code) {
    fragmentShaderCode_ = code;
    return *this;
}

std::vector<uint32_t> PipelineBuilder::readShaderFile(const std::string& path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + path);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(fileSize));
    file.close();

    return buffer;
}

vk::ShaderModule PipelineBuilder::createShaderModule(vk::Device device, const std::vector<uint32_t>& code) {
    vk::ShaderModuleCreateInfo createInfo{{}, code.size() * sizeof(uint32_t), code.data()};
    return device.createShaderModule(createInfo);
}


void PipelineBuilder::build(VkCtx& ctx) {
    std::vector<uint32_t> vertCode = vertexShaderCode_.empty()
        ? readShaderFile(vertexShaderPath_) : vertexShaderCode_;
    std::vector<uint32_t> fragCode = fragmentShaderCode_.empty()
        ? readShaderFile(fragmentShaderPath_) : fragmentShaderCode_;

    vk::ShaderModule vertShaderModule = createShaderModule(ctx.device, vertCode);
    vk::ShaderModule fragShaderModule = createShaderModule(ctx.device, fragCode);

    vk::PipelineShaderStageCreateInfo shaderStages[] = {
        {{}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main"},
        {{}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main"}
    };

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
        {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE
    };

    vk::Viewport viewport{
        0.0f, 0.0f,
        static_cast<float>(ctx.swapchainExtent.width),
        static_cast<float>(ctx.swapchainExtent.height),
        0.0f, 1.0f
    };

    vk::Rect2D scissor{{0, 0}, ctx.swapchainExtent};

    vk::PipelineViewportStateCreateInfo viewportState{{}, 1, &viewport, 1, &scissor};

    vk::PipelineRasterizationStateCreateInfo rasterizer{
        {}, VK_FALSE, VK_FALSE,
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack,
        vk::FrontFace::eClockwise,
        VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f
    };

    vk::PipelineMultisampleStateCreateInfo multisampling{
        {}, vk::SampleCountFlagBits::e1, VK_FALSE
    };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        VK_FALSE,
        vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending{
        {}, VK_FALSE, vk::LogicOp::eCopy,
        1, &colorBlendAttachment
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    ctx.pipelineLayout = ctx.device.createPipelineLayout(pipelineLayoutInfo);

    vk::GraphicsPipelineCreateInfo pipelineInfo{
        {},
        2, shaderStages,
        &vertexInputInfo,
        &inputAssembly,
        nullptr,
        &viewportState,
        &rasterizer,
        &multisampling,
        nullptr,
        &colorBlending,
        nullptr,
        ctx.pipelineLayout,
        ctx.renderPass,
        0
    };

    auto result = ctx.device.createGraphicsPipeline(nullptr, pipelineInfo);
    if (result.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }
    ctx.graphicsPipeline = result.value;

    ctx.device.destroyShaderModule(fragShaderModule);
    ctx.device.destroyShaderModule(vertShaderModule);

    std::cout << "[PipelineBuilder] Graphics pipeline created\n";
}

void FramebufferBuilder::build(VkCtx& ctx) {
    ctx.framebuffers.resize(ctx.swapchainImageViews.size());

    for (size_t i = 0; i < ctx.swapchainImageViews.size(); i++) {
        vk::ImageView attachments[] = {ctx.swapchainImageViews[i]};

        vk::FramebufferCreateInfo framebufferInfo{
            {},
            ctx.renderPass,
            1, attachments,
            ctx.swapchainExtent.width,
            ctx.swapchainExtent.height,
            1
        };

        ctx.framebuffers[i] = ctx.device.createFramebuffer(framebufferInfo);
    }

    std::cout << "[FramebufferBuilder] " << ctx.framebuffers.size() << " framebuffers created\n";
}

// ============================================================================
// CommandBuilder
// ============================================================================

void CommandBuilder::build(VkCtx& ctx) {
    vk::CommandPoolCreateInfo poolInfo{
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        ctx.queueFamilyIndices.graphicsFamily.value()
    };

    ctx.commandPool = ctx.device.createCommandPool(poolInfo);

    ctx.commandBuffers.resize(VkCtx::MAX_FRAMES_IN_FLIGHT);
    vk::CommandBufferAllocateInfo allocInfo{
        ctx.commandPool,
        vk::CommandBufferLevel::ePrimary,
        static_cast<uint32_t>(ctx.commandBuffers.size())
    };

    ctx.commandBuffers = ctx.device.allocateCommandBuffers(allocInfo);

    std::cout << "[CommandBuilder] Command pool and " << ctx.commandBuffers.size() << " command buffers created\n";
}

// ============================================================================
// SyncBuilder
// ============================================================================

void SyncBuilder::build(VkCtx& ctx) {
    ctx.imageAvailableSemaphores.resize(VkCtx::MAX_FRAMES_IN_FLIGHT);
    ctx.renderFinishedSemaphores.resize(VkCtx::MAX_FRAMES_IN_FLIGHT);
    ctx.inFlightFences.resize(VkCtx::MAX_FRAMES_IN_FLIGHT);

    vk::SemaphoreCreateInfo semaphoreInfo{};
    vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};

    for (size_t i = 0; i < VkCtx::MAX_FRAMES_IN_FLIGHT; i++) {
        ctx.imageAvailableSemaphores[i] = ctx.device.createSemaphore(semaphoreInfo);
        ctx.renderFinishedSemaphores[i] = ctx.device.createSemaphore(semaphoreInfo);
        ctx.inFlightFences[i] = ctx.device.createFence(fenceInfo);
    }

    std::cout << "[SyncBuilder] Sync objects created\n";
}

} // namespace svrog::renderer
