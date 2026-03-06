#include "vk_ctx/vk_initializer.h"
#include "vk_ctx/builders/instance_builder.h"

#ifndef NDEBUG
#include "vk_ctx/builders/debug_instance_builder.h"
#endif

#include <QLoggingCategory>

namespace L {
Q_LOGGING_CATEGORY(vkInitializer, "nuff.renderer.vk.initializer")
}

namespace nuff::renderer {

VkInitializer::VkInitializer() {
#ifndef NDEBUG
    m_instanceBuilder = std::make_unique<DebugInstanceBuilder>();
    qCInfo(L::vkInitializer) << "Using DebugInstanceBuilder (debug build)";
#else
    m_instanceBuilder = std::make_unique<InstanceBuilder>();
    qCInfo(L::vkInitializer) << "Using InstanceBuilder (release build)";
#endif
}

VkInitializer& VkInitializer::setAppName(const std::string& name) {
    m_instanceBuilder->setAppName(name);
    return *this;
}

VkInitializer& VkInitializer::setEngineName(const std::string& name) {
    m_instanceBuilder->setEngineName(name);
    return *this;
}

VkInitializer& VkInitializer::addInstanceExtensions(const std::vector<const char*>& extensions) {
    m_instanceBuilder->addExtensions(extensions);
    return *this;
}

VkInitializer& VkInitializer::setSurfaceCreator(SurfaceCreatorFn creator) {
    m_surfaceBuilder.setSurfaceCreator(std::move(creator));
    return *this;
}

VkInitializer& VkInitializer::setExtent(uint32_t width, uint32_t height) {
    m_swapchainBuilder.setExtent(width, height);
    return *this;
}

VkInitializer& VkInitializer::setVertexShaderPath(const std::string& path) {
    m_pipelineBuilder.setVertexShaderPath(path);
    return *this;
}

VkInitializer& VkInitializer::setFragmentShaderPath(const std::string& path) {
    m_pipelineBuilder.setFragmentShaderPath(path);
    return *this;
}

VkInitializer& VkInitializer::setVertexShaderCode(const std::vector<uint32_t>& code) {
    m_pipelineBuilder.setVertexShaderCode(code);
    return *this;
}

VkInitializer& VkInitializer::setFragmentShaderCode(const std::vector<uint32_t>& code) {
    m_pipelineBuilder.setFragmentShaderCode(code);
    return *this;
}

std::unique_ptr<VkCtx> VkInitializer::initialize() {
    auto ctx = std::make_unique<VkCtx>();

    qCInfo(L::vkInitializer) << "Starting Vulkan initialization...";

    m_instanceBuilder->build(*ctx);
    m_surfaceBuilder.build(*ctx);
    m_deviceBuilder.build(*ctx);
    m_swapchainBuilder.build(*ctx);
    m_renderPassBuilder.build(*ctx);
    m_pipelineBuilder.build(*ctx);
    m_framebufferBuilder.build(*ctx);
    m_commandBuilder.build(*ctx);
    m_syncBuilder.build(*ctx);

    qCInfo(L::vkInitializer) << "Vulkan initialization complete!";

    return ctx;
}

} // namespace nuff::renderer

