#pragma once

#include "vk_ctx/builders/i_vk_builder.h"

namespace nuff::renderer {

// CommandBuilder - Creates command pool and command buffers
class CommandBuilder : public IVkBuilder {
public:
    void build(VkCtx& ctx) override;
};

} // namespace nuff::renderer

