#include "swapchain_builder.h"

#include <algorithm>
#include <limits>
#include <QDebug>

QDebug operator<<(QDebug debug, const vk::SurfaceFormatKHR& format)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "SurfaceFormatKHR("
                    << "format: " << vk::to_string(format.format).c_str()
                    << ", colorSpace: " << vk::to_string(format.colorSpace).c_str()
                    << ")";
    return debug;
}

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
    if (formats.empty()) {
        throw std::runtime_error("No surface formats available");
    }
    for (const auto& format : formats) {
        if (format.format == preferred && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return format;
        }
    }
    return formats[0];
}

vk::PresentModeKHR SwapchainBuilder::chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR> &modes, vk::PresentModeKHR preferred)
{
    if (std::ranges::any_of(modes, [preferred](auto presentMode) {
        return presentMode == preferred;
    })) {
        return preferred;
    }

    if (!std::ranges::any_of(modes, [](auto presentMode) {
        return presentMode == vk::PresentModeKHR::eFifo;
    })) {
        throw std::runtime_error("No FIFO present mode available");
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

void SwapchainBuilder::build(CoreCtx& ctx) {
    auto& sc = ctx.extension<SwapchainCtxMixin>();
    auto& graphics = ctx.extension<GraphicsCtxMixin>();

    auto swapchainSupport = querySwapchainSupport(ctx.physicalDevice, *sc.surface);

    qCInfo(logger()) << "Supported swapchain formats" << swapchainSupport.formats;

    auto surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats, m_preferredFormat);
    auto presentMode = chooseSwapPresentMode(swapchainSupport.presentModes, m_preferredPresentMode);
    auto extent = chooseSwapExtent(swapchainSupport.capabilities, m_width, m_height);

    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    // TODO: use MAX_FRAMES_IN_FLIGHT here too
    if (swapchainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapchainSupport.capabilities.maxImageCount) {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo{
        .flags = {},
        .surface = *sc.surface,
        .minImageCount = imageCount,
        // TODO: try hdr format?
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment
    };

    uint32_t queueFamilyIndices[] = {
        graphics.queueFamilyIndices.graphicsFamily.value(),
        graphics.queueFamilyIndices.presentFamily.value()
    };

    if (graphics.queueFamilyIndices.graphicsFamily != graphics.queueFamilyIndices.presentFamily) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = vk::True;

    sc.swapchain = vk::raii::SwapchainKHR(ctx.device, createInfo);
    sc.swapchainImages = sc.swapchain.getImages();
    sc.swapchainImageFormat = surfaceFormat.format;
    sc.swapchainExtent = extent;

    for (size_t i = 0; i < sc.swapchainImages.size(); i++) {
        vk::ImageViewCreateInfo viewInfo{
            .flags = {},
            .image = sc.swapchainImages[i],
            .viewType = vk::ImageViewType::e2D,
            .format = sc.swapchainImageFormat,
            .components = {},
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        sc.swapchainImageViews.emplace_back(ctx.device, viewInfo);
    }

    qCInfo(logger()) << "Swapchain created (" << extent.width << "x" << extent.height << ")";
}

} // namespace nuff::renderer

