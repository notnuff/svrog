#pragma once

#include <QObject>

#include "engine/core/app.h"
#include "presentation/offscreen_render_target.h"

namespace nuff::editor {

class EditorApp : public QObject, public app::App {
    Q_OBJECT

public:
    explicit EditorApp(QObject* parent = nullptr);
    ~EditorApp() override;

    void initialize(uint32_t vendorId, uint32_t deviceId,
                    uint32_t width, uint32_t height);

    void renderFrame();
    void resize(uint32_t width, uint32_t height) override;

    int          exportMemoryFd();
    VkDeviceSize memorySize() const;
    vk::Format   imageFormat() const;
    vk::Extent2D imageExtent() const;
    uint32_t     imageWidth() const;
    uint32_t     imageHeight() const;

protected:
    std::unique_ptr<engine::PlatformConfig> makePlatformConfig() override;
    std::string                             defaultScenePath() const override;

private:
    renderer::OffscreenRenderTarget* offscreenTarget() const;

    uint32_t m_vendorId = 0;
    uint32_t m_deviceId = 0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
};

} // namespace nuff::editor
