#pragma once

#include "core/builders/i_vk_builder.h"

namespace nuff::renderer {

// Must run BEFORE DeviceBuilder
class PhysicalDeviceMatchBuilder : public IVkBuilder {
public:
    PhysicalDeviceMatchBuilder(uint32_t vendorId, uint32_t deviceId)
        : m_vendorId(vendorId), m_deviceId(deviceId) {}

    void build(CoreCtx& ctx) override;

private:
    uint32_t m_vendorId;
    uint32_t m_deviceId;
};

} // namespace nuff::renderer
