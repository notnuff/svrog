#pragma once

#include <vulkan/vulkan.hpp>

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
    ::vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<::vk::SurfaceFormatKHR> formats;
    std::vector<::vk::PresentModeKHR> presentModes;
};

struct VkCtx {
    ::vk::Instance instance;
    ::vk::SurfaceKHR surface;
    ::vk::PhysicalDevice physicalDevice;
    ::vk::Device device;

#ifndef NDEBUG
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
#endif

    ::vk::Queue graphicsQueue;
    ::vk::Queue presentQueue;
    QueueFamilyIndices queueFamilyIndices;

    ::vk::SwapchainKHR swapchain;
    std::vector<::vk::Image> swapchainImages;
    std::vector<::vk::ImageView> swapchainImageViews;
    ::vk::Format swapchainImageFormat;
    ::vk::Extent2D swapchainExtent;

    ::vk::RenderPass renderPass;

    ::vk::PipelineLayout pipelineLayout;
    ::vk::Pipeline graphicsPipeline;

    std::vector<::vk::Framebuffer> framebuffers;

    ::vk::CommandPool commandPool;
    std::vector<::vk::CommandBuffer> commandBuffers;

    std::vector<::vk::Semaphore> imageAvailableSemaphores;
    std::vector<::vk::Semaphore> renderFinishedSemaphores;
    std::vector<::vk::Fence> inFlightFences;

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    void cleanup();
};

} // namespace nuff::renderer

