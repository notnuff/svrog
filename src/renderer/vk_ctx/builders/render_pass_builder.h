#pragma once

#include "vk_ctx/builders/i_vk_builder.h"

namespace nuff::renderer {

// RenderPassBuilder - Creates render pass for triangle rendering
class RenderPassBuilder : public IVkBuilder {
public:
    void build(VkCtx& ctx) override;
};

} // namespace nuff::renderer

