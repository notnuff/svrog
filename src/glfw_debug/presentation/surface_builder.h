#pragma once

#include "core/builders/i_vk_builder.h"

#include <functional>

namespace nuff::renderer {

// SurfaceBuilder - Creates Vulkan surface (requires platform-specific callback)
class SurfaceBuilder final : public IVkBuilder {
public:
    using SurfaceCreatorFn = std::function<VkSurfaceKHR(VkInstance)>;

    SurfaceBuilder& setSurfaceCreator(SurfaceCreatorFn creator);

    void build(CoreCtx& ctx) override;

private:
    SurfaceCreatorFn m_surfaceCreator;
};

} // namespace nuff::renderer

