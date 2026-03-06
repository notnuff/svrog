#pragma once

#include "vk_ctx/builders/i_vk_builder.h"

namespace nuff::renderer {

// SyncBuilder - Creates semaphores and fences
class SyncBuilder : public IVkBuilder {
public:
    void build(VkCtx& ctx) override;
};

} // namespace nuff::renderer

