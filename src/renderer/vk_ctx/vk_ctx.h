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

struct SwapchainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct VkCtx {
    // RAII context must be first member to be initialized first and destroyed last
    vk::raii::Context context;

    vk::raii::Instance instance{nullptr};
    vk::raii::SurfaceKHR surface{nullptr};
    vk::raii::PhysicalDevice physicalDevice{nullptr};
    vk::raii::Device device{nullptr};

#ifndef NDEBUG
    vk::raii::DebugUtilsMessengerEXT debugMessenger{nullptr};
#endif

    vk::Queue graphicsQueue;  // Non-owning handle, no RAII wrapper
    vk::Queue presentQueue;   // Non-owning handle, no RAII wrapper
    QueueFamilyIndices queueFamilyIndices;

    vk::raii::SwapchainKHR swapchain{nullptr};
    std::vector<vk::Image> swapchainImages;  // Non-owning handles, no RAII wrapper
    std::vector<vk::raii::ImageView> swapchainImageViews;
    vk::Format swapchainImageFormat;
    vk::Extent2D swapchainExtent;

    vk::raii::RenderPass renderPass{nullptr};

    vk::raii::PipelineLayout pipelineLayout{nullptr};
    vk::raii::Pipeline graphicsPipeline{nullptr};

    std::vector<vk::raii::Framebuffer> framebuffers;

    vk::raii::CommandPool commandPool{nullptr};
    vk::raii::CommandBuffers commandBuffers{nullptr};

    std::vector<vk::raii::Semaphore> imageAvailableSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence> inFlightFences;

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
};

} // namespace nuff::renderer

