#include "editor_app.h"

#include "qml_platform_config.h"

#include <QCoreApplication>
#include <QLoggingCategory>

namespace L {
Q_LOGGING_CATEGORY(editorApp, "nuff.editor.app")
}

namespace nuff::editor {

EditorApp::EditorApp(QObject* parent)
    : QObject(parent) {}

EditorApp::~EditorApp() {
    if (isInitialized()) shutdown();
}

void EditorApp::initialize(uint32_t vendorId, uint32_t deviceId,
                           uint32_t width, uint32_t height) {
    if (isInitialized()) return;

    qCInfo(L::editorApp) << "Initializing: vendorId=" << vendorId
                         << "deviceId=" << deviceId
                         << "size=" << width << "x" << height;

    m_vendorId = vendorId;
    m_deviceId = deviceId;
    m_width = width;
    m_height = height;

    if (!app::App::initialize()) {
        qCCritical(L::editorApp) << "Failed to initialize engine";
        return;
    }
    qCInfo(L::editorApp) << "Engine initialized successfully";
}

std::unique_ptr<engine::PlatformConfig> EditorApp::makePlatformConfig() {
    auto config = std::make_unique<QmlPlatformConfig>(
        m_vendorId, m_deviceId, m_width, m_height);

    std::string shaderPath = (QCoreApplication::applicationDirPath()
        + "/shaders/triangle_shader.spv").toStdString();
    config->setShaderPath(shaderPath, shaderPath);

    return config;
}

std::string EditorApp::defaultScenePath() const {
    return (QCoreApplication::applicationDirPath()
        + "/models/mv_spartan/scene.gltf").toStdString();
}

void EditorApp::renderFrame() {
    tick();
}

void EditorApp::resize(uint32_t width, uint32_t height) {
    if (!isInitialized()) return;
    if (width == 0 || height == 0) return;

    auto* target = offscreenTarget();
    if (!target) return;

    auto currentExtent = target->extent();
    if (currentExtent.width == width && currentExtent.height == height) return;

    target->resize(width, height);
    app::App::resize(width, height);

    qCInfo(L::editorApp) << "Resized to" << width << "x" << height;
}

renderer::OffscreenRenderTarget* EditorApp::offscreenTarget() const {
    return engineSystem().renderTargetAs<renderer::OffscreenRenderTarget>();
}

int EditorApp::exportMemoryFd() {
    if (!isInitialized()) return -1;
    return offscreenTarget()->getMemoryFd();
}

VkDeviceSize EditorApp::memorySize() const {
    if (!isInitialized()) return 0;
    return offscreenTarget()->memorySize();
}

vk::Format EditorApp::imageFormat() const {
    if (!isInitialized()) return vk::Format::eUndefined;
    return offscreenTarget()->format();
}

vk::Extent2D EditorApp::imageExtent() const {
    if (!isInitialized()) return {0, 0};
    return offscreenTarget()->extent();
}

uint32_t EditorApp::imageWidth() const {
    return imageExtent().width;
}

uint32_t EditorApp::imageHeight() const {
    return imageExtent().height;
}

} // namespace nuff::editor
