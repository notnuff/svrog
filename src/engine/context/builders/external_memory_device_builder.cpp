#include "context/builders/external_memory_device_builder.h"

namespace nuff::renderer {

void ExternalMemoryDeviceBuilder::build(CoreCtx& ctx) {
    auto& reqs = ctx.component<DeviceRequirementsComponent>();
    reqs.additionalDeviceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
}

} // namespace nuff::renderer
