#pragma once

#include "common/vk_common.h"

#include <optional>
#include <vector>

namespace nuff::renderer {

// TODO use separate queue for TRANSFER.

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool isComplete() const {
        return graphicsFamily.has_value();
    }

    [[nodiscard]] bool hasPresentSupport() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct ICtxExtension {
    ICtxExtension() = default;
    virtual ~ICtxExtension() = default;
};

struct CoreCtx {
    // RAII context must be initialized first and destroyed last
    vk::raii::Context context;

    vk::raii::Instance instance{nullptr};
    vk::raii::PhysicalDevice physicalDevice{nullptr};
    vk::raii::Device device{nullptr};

    template <typename CtxExtension>
    CtxExtension& extension() {
        static_assert(std::is_base_of_v<ICtxExtension, CtxExtension>,
              "extension must derive from ICtxExtension");

        for (const auto& extension : ctxExtensions) {
            if (auto ext = dynamic_cast<CtxExtension*>(extension.get())) {
                return *ext;
            }
        }

        auto ptr = std::make_unique<CtxExtension>();
        CtxExtension& ref = *ptr;

        ctxExtensions.emplace_back(std::move(ptr));
        return ref;
    }

private:
    std::vector<std::unique_ptr<ICtxExtension>> ctxExtensions;
};

struct GraphicsCtxMixin : ICtxExtension {
    vk::Queue graphicsQueue;
    QueueFamilyIndices queueFamilyIndices;
};

struct InstanceExtensionsMixin : ICtxExtension {
    std::vector<const char*> instanceExtensions;
};

struct InstanceLayersMixin : ICtxExtension {
    std::vector<const char*> instanceLayers;
};

struct DebugMessengerCtxMixin : ICtxExtension {
    vk::raii::DebugUtilsMessengerEXT debugMessenger{nullptr};
};

struct PipelineCtxMixin : ICtxExtension {
    vk::raii::DescriptorSetLayout descriptorSetLayout{nullptr};
    vk::raii::PipelineLayout pipelineLayout{nullptr};
    vk::raii::Pipeline graphicsPipeline{nullptr};
};

struct PipelineConfigMixin : ICtxExtension {
    vk::Format colorAttachmentFormat = vk::Format::eB8G8R8A8Unorm;
};

struct DeviceRequirementsMixin : ICtxExtension {
    bool requirePresent = false;
    std::vector<const char*> additionalDeviceExtensions;
};

struct PhysicalDevicePreferenceMixin : ICtxExtension {
    std::optional<uint32_t> preferredVendorId;
    std::optional<uint32_t> preferredDeviceId;
};

struct PresentQueueMixin : ICtxExtension {
    vk::Queue presentQueue;
};

struct SwapchainCtxMixin : ICtxExtension {
    vk::raii::SurfaceKHR surface{nullptr};
    vk::raii::SwapchainKHR swapchain{nullptr};
    std::vector<vk::Image> swapchainImages;
    std::vector<vk::raii::ImageView> swapchainImageViews;
    vk::Format swapchainImageFormat{};
    vk::Extent2D swapchainExtent{};
};

struct SwapchainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct RenderPassCtxMixin : ICtxExtension {
    vk::raii::RenderPass renderPass{nullptr};
    std::vector<vk::raii::Framebuffer> framebuffers;
};

} // namespace nuff::renderer

