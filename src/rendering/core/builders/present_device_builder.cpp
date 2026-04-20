#include "core/builders/present_device_builder.h"

namespace nuff::renderer {

void PresentDeviceBuilder::build(CoreCtx& ctx) {
    auto& reqs = ctx.extension<DeviceRequirementsMixin>();
    reqs.requirePresent = true;
    reqs.additionalDeviceExtensions.push_back(vk::KHRSwapchainExtensionName);
}

} // namespace nuff::renderer
