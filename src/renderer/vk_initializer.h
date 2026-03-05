#pragma once

#include "vk_builder.h"
#include "vk_ctx.h"

#include <functional>
#include <memory>
#include <vector>

namespace svrog::renderer {

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
    InstanceBuilder instanceBuilder_;
    SurfaceBuilder surfaceBuilder_;
    DeviceBuilder deviceBuilder_;
    SwapchainBuilder swapchainBuilder_;
    RenderPassBuilder renderPassBuilder_;
    PipelineBuilder pipelineBuilder_;
    FramebufferBuilder framebufferBuilder_;
    CommandBuilder commandBuilder_;
    SyncBuilder syncBuilder_;
};

} // namespace svrog::renderer

