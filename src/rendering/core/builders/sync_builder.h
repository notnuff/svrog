#pragma once

#include "core/builders/i_vk_builder.h"

namespace nuff::renderer {

// SyncBuilder - Creates semaphores and fences
class SyncBuilder : public IVkBuilder {
public:
    void build(CoreCtx& ctx) override;
};

} // namespace nuff::renderer

