#include "vk_ctx/builders/swapchain_builder.h"

#include <algorithm>
#include <limits>

namespace nuff::renderer {

SwapchainBuilder& SwapchainBuilder::setPreferredFormat(vk::Format format) {
    m_preferredFormat = format;
    return *this;
}

SwapchainBuilder& SwapchainBuilder::setPreferredPresentMode(vk::PresentModeKHR mode) {
    m_preferredPresentMode = mode;
    return *this;
}

SwapchainBuilder& SwapchainBuilder::setExtent(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
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
    auto swapchainSupport = querySwapchainSupport(ctx.physicalDevice, *ctx.surface);

    auto surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats, m_preferredFormat);
    auto presentMode = chooseSwapPresentMode(swapchainSupport.presentModes, m_preferredPresentMode);
    auto extent = chooseSwapExtent(swapchainSupport.capabilities, m_width, m_height);

    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapchainSupport.capabilities.maxImageCount) {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo{
        {},
        *ctx.surface,
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

    ctx.swapchain = vk::raii::SwapchainKHR(ctx.device, createInfo);
    ctx.swapchainImages = ctx.swapchain.getImages();
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
        ctx.swapchainImageViews.emplace_back(ctx.device, viewInfo);
    }

    qCInfo(logger()) << "Swapchain created (" << extent.width << "x" << extent.height << ")";
}

} // namespace nuff::renderer

