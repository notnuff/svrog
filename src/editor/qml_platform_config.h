#pragma once

#include "engine/core/platform_config.h"
#include "presentation/offscreen_render_target.h"

#include <cstdint>
#include <string>

namespace nuff::editor {

class QmlPlatformConfig : public engine::PlatformConfig {
public:
    QmlPlatformConfig(uint32_t vendorId, uint32_t deviceId,
                      uint32_t width, uint32_t height,
                      vk::Format format = vk::Format::eB8G8R8A8Unorm);

    void setShaderPath(std::string vertexPath, std::string fragmentPath);

protected:
    std::unique_ptr<renderer::CoreInitializer> makeInitializer() override;
    std::unique_ptr<renderer::IRenderTarget>
        makeRenderTarget(renderer::CoreCtx& ctx) override;

private:
    uint32_t m_vendorId;
    uint32_t m_deviceId;
    uint32_t m_width;
    uint32_t m_height;
    vk::Format m_format;
    std::string m_vertexShaderPath;
    std::string m_fragmentShaderPath;
};

} // namespace nuff::editor
