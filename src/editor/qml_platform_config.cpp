#include "qml_platform_config.h"
#include "qml_initializer.h"

namespace nuff::editor {

QmlPlatformConfig::QmlPlatformConfig(uint32_t vendorId, uint32_t deviceId,
                                     uint32_t width, uint32_t height,
                                     vk::Format format)
    : m_vendorId(vendorId), m_deviceId(deviceId),
      m_width(width), m_height(height), m_format(format) {}

void QmlPlatformConfig::setShaderPath(std::string vertexPath, std::string fragmentPath) {
    m_vertexShaderPath = std::move(vertexPath);
    m_fragmentShaderPath = std::move(fragmentPath);
}

std::unique_ptr<renderer::CoreInitializer> QmlPlatformConfig::makeInitializer() {
    auto initializer = std::make_unique<QmlInitializer>();
    initializer->setPhysicalDevicePreference(m_vendorId, m_deviceId);
    initializer->setVertexShaderPath(m_vertexShaderPath)
                .setFragmentShaderPath(m_fragmentShaderPath);
    return initializer;
}

std::unique_ptr<renderer::IRenderTarget>
QmlPlatformConfig::makeRenderTarget(renderer::CoreCtx& ctx) {
    return std::make_unique<renderer::OffscreenRenderTarget>(
        &ctx, m_width, m_height, m_format, /*exportable=*/true);
}

} // namespace nuff::editor
