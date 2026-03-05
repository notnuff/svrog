#pragma once

#include "vk_ctx/vk_ctx.h"

#include <functional>
#include <string>
#include <vector>

namespace nuff::renderer {

// Base builder interface for Vulkan components.
class IVkBuilder {
public:
    virtual ~IVkBuilder() = default;
    virtual void build(VkCtx& ctx) = 0;
};

// InstanceBuilder - Creates Vulkan instance
class InstanceBuilder : public IVkBuilder {
public:
    InstanceBuilder& setAppName(const std::string& name);
    InstanceBuilder& setEngineName(const std::string& name);
    InstanceBuilder& addExtensions(const std::vector<const char*>& extensions);
    InstanceBuilder& addValidationLayers(const std::vector<const char*>& layers);
    InstanceBuilder& enableValidation(bool enable = true);

    void build(VkCtx& ctx) override;

private:
    std::string appName_ = "VkApp";
    std::string engineName_ = "svrog";
    std::vector<const char*> extensions_;
    std::vector<const char*> validationLayers_;
    bool enableValidation_ = false;
};

// SurfaceBuilder - Creates Vulkan surface (requires platform-specific callback)
class SurfaceBuilder : public IVkBuilder {
public:
    using SurfaceCreatorFn = std::function<VkSurfaceKHR(VkInstance)>;

    SurfaceBuilder& setSurfaceCreator(SurfaceCreatorFn creator);

    void build(VkCtx& ctx) override;

private:
    SurfaceCreatorFn surfaceCreator_;
};

// DeviceBuilder - Selects physical device and creates logical device
class DeviceBuilder : public IVkBuilder {
public:
    DeviceBuilder& requireGraphicsQueue(bool require = true);
    DeviceBuilder& requirePresentQueue(bool require = true);
    DeviceBuilder& addDeviceExtensions(const std::vector<const char*>& extensions);

    void build(VkCtx& ctx) override;

private:
    bool requireGraphics_ = true;
    bool requirePresent_ = true;
    std::vector<const char*> deviceExtensions_ = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    static QueueFamilyIndices findQueueFamilies(::vk::PhysicalDevice device, ::vk::SurfaceKHR surface);
    static bool isDeviceSuitable(::vk::PhysicalDevice device, ::vk::SurfaceKHR surface,
                                  const std::vector<const char*>& extensions);
    static bool checkDeviceExtensionSupport(::vk::PhysicalDevice device,
                                             const std::vector<const char*>& extensions);
};

// SwapchainBuilder - Creates swapchain and image views
class SwapchainBuilder : public IVkBuilder {
public:
    SwapchainBuilder& setPreferredFormat(::vk::Format format);
    SwapchainBuilder& setPreferredPresentMode(::vk::PresentModeKHR mode);
    SwapchainBuilder& setExtent(uint32_t width, uint32_t height);

    void build(VkCtx& ctx) override;

private:
    ::vk::Format preferredFormat_ = ::vk::Format::eB8G8R8A8Srgb;
    ::vk::PresentModeKHR preferredPresentMode_ = ::vk::PresentModeKHR::eMailbox;
    uint32_t width_ = 800;
    uint32_t height_ = 600;

    static SwapchainSupportDetails querySwapchainSupport(::vk::PhysicalDevice device, ::vk::SurfaceKHR surface);
    static ::vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<::vk::SurfaceFormatKHR>& formats,
                                                         ::vk::Format preferred);
    static ::vk::PresentModeKHR chooseSwapPresentMode(const std::vector<::vk::PresentModeKHR>& modes,
                                                     ::vk::PresentModeKHR preferred);
    static ::vk::Extent2D chooseSwapExtent(const ::vk::SurfaceCapabilitiesKHR& capabilities,
                                          uint32_t width, uint32_t height);
};

// RenderPassBuilder - Creates render pass for triangle rendering
class RenderPassBuilder : public IVkBuilder {
public:
    void build(VkCtx& ctx) override;
};

// PipelineBuilder - Creates graphics pipeline
class PipelineBuilder : public IVkBuilder {
public:
    PipelineBuilder& setVertexShaderPath(const std::string& path);
    PipelineBuilder& setFragmentShaderPath(const std::string& path);
    PipelineBuilder& setVertexShaderCode(const std::vector<uint32_t>& code);
    PipelineBuilder& setFragmentShaderCode(const std::vector<uint32_t>& code);

    void build(VkCtx& ctx) override;

private:
    std::string vertexShaderPath_;
    std::string fragmentShaderPath_;
    std::vector<uint32_t> vertexShaderCode_;
    std::vector<uint32_t> fragmentShaderCode_;

    static std::vector<uint32_t> readShaderFile(const std::string& path);
    static ::vk::ShaderModule createShaderModule(::vk::Device device, const std::vector<uint32_t>& code);
};

// FramebufferBuilder - Creates framebuffers for each swapchain image
class FramebufferBuilder : public IVkBuilder {
public:
    void build(VkCtx& ctx) override;
};

// CommandBuilder - Creates command pool and command buffers
class CommandBuilder : public IVkBuilder {
public:
    void build(VkCtx& ctx) override;
};

// SyncBuilder - Creates semaphores and fences
class SyncBuilder : public IVkBuilder {
public:
    void build(VkCtx& ctx) override;
};

} // namespace nuff::renderer

