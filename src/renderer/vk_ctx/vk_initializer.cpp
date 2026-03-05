#include "vk_ctx/vk_initializer.h"

#include <iostream>

namespace nuff::renderer {

VkInitializer::VkInitializer() = default;

VkInitializer& VkInitializer::setAppName(const std::string& name) {
    instanceBuilder_.setAppName(name);
    return *this;
}

VkInitializer& VkInitializer::setEngineName(const std::string& name) {
    instanceBuilder_.setEngineName(name);
    return *this;
}

VkInitializer& VkInitializer::addInstanceExtensions(const std::vector<const char*>& extensions) {
    instanceBuilder_.addExtensions(extensions);
    return *this;
}

VkInitializer& VkInitializer::enableValidation(bool enable) {
    instanceBuilder_.enableValidation(enable);
    return *this;
}

VkInitializer& VkInitializer::setSurfaceCreator(SurfaceCreatorFn creator) {
    surfaceBuilder_.setSurfaceCreator(std::move(creator));
    return *this;
}

VkInitializer& VkInitializer::setExtent(uint32_t width, uint32_t height) {
    swapchainBuilder_.setExtent(width, height);
    return *this;
}

VkInitializer& VkInitializer::setVertexShaderPath(const std::string& path) {
    pipelineBuilder_.setVertexShaderPath(path);
    return *this;
}

VkInitializer& VkInitializer::setFragmentShaderPath(const std::string& path) {
    pipelineBuilder_.setFragmentShaderPath(path);
    return *this;
}

VkInitializer& VkInitializer::setVertexShaderCode(const std::vector<uint32_t>& code) {
    pipelineBuilder_.setVertexShaderCode(code);
    return *this;
}

VkInitializer& VkInitializer::setFragmentShaderCode(const std::vector<uint32_t>& code) {
    pipelineBuilder_.setFragmentShaderCode(code);
    return *this;
}

std::unique_ptr<VkCtx> VkInitializer::initialize() {
    auto ctx = std::make_unique<VkCtx>();

    std::cout << "[VkInitializer] Starting Vulkan initialization...\n";

    instanceBuilder_.build(*ctx);
    surfaceBuilder_.build(*ctx);
    deviceBuilder_.build(*ctx);
    swapchainBuilder_.build(*ctx);
    renderPassBuilder_.build(*ctx);
    pipelineBuilder_.build(*ctx);
    framebufferBuilder_.build(*ctx);
    commandBuilder_.build(*ctx);
    syncBuilder_.build(*ctx);

    std::cout << "[VkInitializer] Vulkan initialization complete!\n";

    return ctx;
}

} // namespace nuff::renderer

