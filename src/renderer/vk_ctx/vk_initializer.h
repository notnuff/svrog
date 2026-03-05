#pragma once

#include "vk_ctx/vk_builder.h"
#include "vk_ctx/vk_ctx.h"

#include <functional>
#include <memory>
#include <vector>

namespace nuff::renderer {

class VkInitializer {
public:
    using SurfaceCreatorFn = std::function<VkSurfaceKHR(VkInstance)>;

    VkInitializer();

    VkInitializer& setAppName(const std::string& name);
    VkInitializer& setEngineName(const std::string& name);
    VkInitializer& addInstanceExtensions(const std::vector<const char*>& extensions);
    VkInitializer& enableValidation(bool enable = true);
    VkInitializer& setSurfaceCreator(SurfaceCreatorFn creator);
    VkInitializer& setExtent(uint32_t width, uint32_t height);
    VkInitializer& setVertexShaderPath(const std::string& path);
    VkInitializer& setFragmentShaderPath(const std::string& path);
    VkInitializer& setVertexShaderCode(const std::vector<uint32_t>& code);
    VkInitializer& setFragmentShaderCode(const std::vector<uint32_t>& code);

    std::unique_ptr<VkCtx> initialize();

private:
    InstanceBuilder m_instanceBuilder;
    SurfaceBuilder m_surfaceBuilder;
    DeviceBuilder m_deviceBuilder;
    SwapchainBuilder m_swapchainBuilder;
    RenderPassBuilder m_renderPassBuilder;
    PipelineBuilder m_pipelineBuilder;
    FramebufferBuilder m_framebufferBuilder;
    CommandBuilder m_commandBuilder;
    SyncBuilder m_syncBuilder;
};

} // namespace nuff::renderer

