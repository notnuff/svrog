#pragma once

#include <QObject>
#include <memory>

#include "core/context/ctx.h"
#include "core/renderer/renderer.h"
#include "presentation/offscreen_render_target.h"

namespace nuff::bridge {

class RenderEngine : public QObject {
    Q_OBJECT

public:
    explicit RenderEngine(QObject* parent = nullptr);
    ~RenderEngine() override;

    void initialize(uint32_t vendorId, uint32_t deviceId,
                    uint32_t width, uint32_t height);

    bool isInitialized() const { return m_initialized; }

    void renderFrame();
    void resize(uint32_t width, uint32_t height);

    int exportMemoryFd();
    VkDeviceSize memorySize() const;
    vk::Format imageFormat() const;
    vk::Extent2D imageExtent() const;
    uint32_t imageWidth() const;
    uint32_t imageHeight() const;

private:
    bool m_initialized = false;
    std::unique_ptr<renderer::CoreCtx> m_ctx;
    renderer::Renderer m_renderer;
    std::unique_ptr<renderer::OffscreenRenderTarget> m_renderTarget;
};

} // namespace nuff::bridge
