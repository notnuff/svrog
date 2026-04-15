#pragma once

#include "i_vk_builder.h"

namespace nuff::renderer {

// CommandBuilder - Creates command pool and command buffers
class CommandBuilder : public IVkBuilder {
public:
    void build(CoreCtx& ctx) override;
};

} // namespace nuff::renderer

