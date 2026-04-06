#pragma once

#include "vk_ctx/builders/i_vk_builder.h"

#include <vector>

namespace nuff::renderer {

// SwapchainBuilder - Creates swapchain and image views
class SwapchainBuilder : public IVkBuilder {
public:
    SwapchainBuilder& setPreferredFormat(vk::Format format);
    SwapchainBuilder& setPreferredPresentMode(vk::PresentModeKHR mode);
    SwapchainBuilder& setExtent(uint32_t width, uint32_t height);

    void build(VkCtx& ctx) override;

private:
    vk::Format m_preferredFormat = vk::Format::eB8G8R8A8Srgb;
    vk::PresentModeKHR m_preferredPresentMode = vk::PresentModeKHR::eMailbox;
    uint32_t m_width = 800;
    uint32_t m_height = 600;

    static SwapchainSupportDetails querySwapchainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface);
    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats,
                                                         vk::Format preferred);
    static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& modes,
                                                     vk::PresentModeKHR preferred);
    static vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                                          uint32_t width, uint32_t height);
};

} // namespace nuff::renderer

