#pragma once

#include "context/initialization/build_type_initializer.h"
#include "context/builders/physical_device_match_builder.h"
#include "context/builders/external_memory_device_builder.h"

namespace nuff::editor {

// Own Vulkan instance, same physical device as Qt, external memory for image sharing
class QmlInitializer : public CoreInitializerT {
public:
    QmlInitializer() = default;
    ~QmlInitializer() override = default;

    void setPhysicalDevicePreference(uint32_t vendorId, uint32_t deviceId) {
        m_vendorId = vendorId;
        m_deviceId = deviceId;
    }

protected:
    void prepareBuilders() override {
        CoreInitializerT::prepareBuilders();

        if (auto link = m_builders.get<renderer::DeviceBuilder>(); link.has_value()) {
            link->insert_before<renderer::PhysicalDeviceMatchBuilder>(m_vendorId, m_deviceId);
            link->insert_before<renderer::ExternalMemoryDeviceBuilder>();
        }
    }

private:
    uint32_t m_vendorId = 0;
    uint32_t m_deviceId = 0;
};

} // namespace nuff::editor
