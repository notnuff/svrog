#pragma once

#include "common/vk_common.h"

#include <optional>
#include <vector>

namespace nuff::renderer {

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct ICtxExtension {
    ICtxExtension() = default;
    virtual ~ICtxExtension() = default;
};

struct CoreCtx {
    // RAII context must be first member to be initialized first and destroyed last
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
    vk::Queue graphicsQueue;  // Non-owning handle, no RAII wrapper
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

struct CommandCtxMixin : ICtxExtension {
    vk::raii::CommandPool commandPool{nullptr};
    vk::raii::CommandBuffers commandBuffers{nullptr};
};

struct PresentCtxMixin : ICtxExtension {
    vk::Queue presentQueue;   // Non-owning handle, no RAII wrapper

    std::vector<vk::raii::Semaphore> imageAvailableSemaphores;  // Per frame-in-flight
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores; // Per swapchain image
    std::vector<vk::raii::Fence> inFlightFences;               // Per frame-in-flight

    uint32_t maxFramesInFlight{2};
};

struct SwapchainCtxMixin : ICtxExtension {
    vk::raii::SurfaceKHR surface{nullptr};
    vk::raii::SwapchainKHR swapchain{nullptr};
    std::vector<vk::Image> swapchainImages;
    std::vector<vk::raii::ImageView> swapchainImageViews;
    vk::Format swapchainImageFormat{};
    vk::Extent2D swapchainExtent{};
};

struct PipelineCtxMixin : ICtxExtension {
    vk::raii::PipelineLayout pipelineLayout{nullptr};
    vk::raii::Pipeline graphicsPipeline{nullptr};
};

struct RenderPassCtxMixin : ICtxExtension {
    vk::raii::RenderPass renderPass{nullptr};
    std::vector<vk::raii::Framebuffer> framebuffers;
};

struct SwapchainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

} // namespace nuff::renderer

