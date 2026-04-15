#pragma once

#include "core/builders/i_vk_builder.h"

namespace nuff::renderer {

// FramebufferBuilder - Creates framebuffers for each swapchain image
class FramebufferBuilder : public IVkBuilder {
public:
    void build(CoreCtx& ctx) override;
};

} // namespace nuff::renderer

