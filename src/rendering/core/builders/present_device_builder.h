#pragma once

#include "core/builders/i_vk_builder.h"

namespace nuff::renderer {

// Must run BEFORE DeviceBuilder
class PresentDeviceBuilder : public IVkBuilder {
public:
    void build(CoreCtx& ctx) override;
};

} // namespace nuff::renderer
