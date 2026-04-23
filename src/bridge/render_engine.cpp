#include "render_engine.h"
#include "qml_initializer.h"

#include <QCoreApplication>
#include <QLoggingCategory>

namespace L {
Q_LOGGING_CATEGORY(renderEngine, "nuff.bridge.render_engine")
}

namespace nuff::bridge {

RenderEngine::RenderEngine(QObject* parent)
    : QObject(parent) {}

RenderEngine::~RenderEngine() {
    if (m_initialized) {
        m_renderer.cleanup();
    }
}

void RenderEngine::initialize(uint32_t vendorId, uint32_t deviceId,
                              uint32_t width, uint32_t height) {
    if (m_initialized) return;

    qCInfo(L::renderEngine) << "Initializing engine: vendorId=" << vendorId
                            << "deviceId=" << deviceId
                            << "size=" << width << "x" << height;

    QmlInitializer initializer;
    initializer.setPhysicalDevicePreference(vendorId, deviceId);

    std::string shaderPath = (QCoreApplication::applicationDirPath()
        + "/shaders/triangle_shader.spv").toStdString();
    initializer
        .setVertexShaderPath(shaderPath)
        .setFragmentShaderPath(shaderPath);

    m_ctx = initializer.buildCtx();
    if (!m_ctx) {
        qCCritical(L::renderEngine) << "Failed to create Vulkan context";
        return;
    }

    m_renderTarget = std::make_unique<renderer::OffscreenRenderTarget>(
        m_ctx.get(), width, height,
        vk::Format::eB8G8R8A8Unorm,
        true /* exportable */);

    std::string texturePath = (QCoreApplication::applicationDirPath()
        + "/textures/test.jpeg").toStdString();

    m_renderer.setContext(m_ctx.get());
    m_renderer.setRenderTarget(m_renderTarget.get());
    m_renderer.setTexturePath(texturePath);
    m_renderer.initialize();

    m_initialized = true;
    qCInfo(L::renderEngine) << "Engine initialized successfully";
}

void RenderEngine::renderFrame() {
    if (!m_initialized) return;
    m_renderer.drawFrame();
}

void RenderEngine::resize(uint32_t width, uint32_t height) {
    if (!m_initialized) return;
    if (width == 0 || height == 0) return;

    auto currentExtent = m_renderTarget->extent();
    if (currentExtent.width == width && currentExtent.height == height) return;

    m_renderTarget->resize(width, height);
    qCInfo(L::renderEngine) << "Resized to" << width << "x" << height;
}

int RenderEngine::exportMemoryFd() {
    if (!m_initialized) return -1;
    return m_renderTarget->getMemoryFd();
}

VkDeviceSize RenderEngine::memorySize() const {
    if (!m_initialized) return 0;
    return m_renderTarget->memorySize();
}

vk::Format RenderEngine::imageFormat() const {
    if (!m_initialized) return vk::Format::eUndefined;
    return m_renderTarget->format();
}

vk::Extent2D RenderEngine::imageExtent() const {
    if (!m_initialized) return {0, 0};
    return m_renderTarget->extent();
}

uint32_t RenderEngine::imageWidth() const {
    return imageExtent().width;
}

uint32_t RenderEngine::imageHeight() const {
    return imageExtent().height;
}

} // namespace nuff::bridge
