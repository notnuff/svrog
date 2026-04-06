#pragma once

#include "vk_ctx/builders/i_vk_builder.h"

namespace nuff::renderer {

// FramebufferBuilder - Creates framebuffers for each swapchain image
class FramebufferBuilder : public IVkBuilder {
public:
    void build(VkCtx& ctx) override;
};

} // namespace nuff::renderer

