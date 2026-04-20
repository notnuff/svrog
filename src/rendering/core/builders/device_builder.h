#pragma once

#include "core/builders/i_vk_builder.h"

#include <vector>

namespace nuff::renderer {

// Surface/present support is optional, configured via DeviceRequirementsMixin
class DeviceBuilder : public IVkBuilder {
public:
    void build(CoreCtx& ctx) override;

private:
    static QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device,
                                                 vk::SurfaceKHR surface = nullptr);
    static bool isDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface,
                                  const std::vector<const char*>& extensions,
                                  bool requirePresent);

    template <typename... Features>
    static bool deviceHasRequiredFeatures(vk::PhysicalDevice device, const vk::StructureChain<Features...>& req);

    static bool checkDeviceExtensionSupport(vk::PhysicalDevice device,
                                            const std::vector<const char*>& reqExtensions);
};

} // namespace nuff::renderer

#include "core/builders/device_builder.inl"
