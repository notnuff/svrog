#pragma once

#include "vk_ctx/builders/i_vk_builder.h"

#include <vector>

namespace nuff::renderer {

// DeviceBuilder - Selects physical device and creates logical device
class DeviceBuilder : public IVkBuilder {
public:
    DeviceBuilder& requireGraphicsQueue(bool require = true);
    DeviceBuilder& requirePresentQueue(bool require = true);
    DeviceBuilder& addDeviceExtensions(const std::vector<const char*>& extensions);

    void build(VkCtx& ctx) override;

private:
    bool m_requireGraphics = true;
    bool m_requirePresent = true;
    std::vector<const char*> m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    static QueueFamilyIndices findQueueFamilies(::vk::PhysicalDevice device, ::vk::SurfaceKHR surface);
    static bool isDeviceSuitable(::vk::PhysicalDevice device, ::vk::SurfaceKHR surface,
                                  const std::vector<const char*>& extensions);
    static bool checkDeviceExtensionSupport(::vk::PhysicalDevice device,
                                             const std::vector<const char*>& reqExtensions);
};

} // namespace nuff::renderer

