#include "core/builders/physical_device_match_builder.h"

namespace nuff::renderer {

void PhysicalDeviceMatchBuilder::build(CoreCtx& ctx) {
    auto& pref = ctx.extension<PhysicalDevicePreferenceMixin>();
    pref.preferredVendorId = m_vendorId;
    pref.preferredDeviceId = m_deviceId;

    qCInfo(logger()) << "Physical device preference set: vendorId="
                     << m_vendorId << "deviceId=" << m_deviceId;
}

} // namespace nuff::renderer
