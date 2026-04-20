#pragma once

#include "core/builders/i_vk_builder.h"

namespace nuff::renderer {

// Must run BEFORE DeviceBuilder
class ExternalMemoryDeviceBuilder : public IVkBuilder {
public:
    void build(CoreCtx& ctx) override;
};

} // namespace nuff::renderer
