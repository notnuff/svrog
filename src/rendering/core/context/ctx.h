#pragma once

#include "common/vk_common.h"

#include <optional>
#include <unordered_map>
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

struct ICtxComponent {
    ICtxComponent() = default;
    virtual ~ICtxComponent() = default;
};

class CtxComponentTypeIDSystem {
private:
    static size_t nextTypeID;

public:
    template<typename T>
    static size_t getTypeID() {
        static size_t typeID = nextTypeID++;
        return typeID;
    }
};

struct CoreCtx {
    // RAII context must be initialized first and destroyed last
    vk::raii::Context context;

    vk::raii::Instance instance{nullptr};
    vk::raii::PhysicalDevice physicalDevice{nullptr};
    vk::raii::Device device{nullptr};

    template <typename CtxComponentT>
    CtxComponentT& component() {
        static_assert(std::is_base_of_v<ICtxComponent, CtxComponentT>,
              "extension must derive from ICtxExtension");

        auto id = CtxComponentTypeIDSystem::getTypeID<CtxComponentT>();
        if (m_ctxComponentsMap.contains(id)) {
            return *(static_cast<CtxComponentT*>(m_ctxComponentsMap[id]));
        }

        auto ptr = std::make_unique<CtxComponentT>();
        CtxComponentT& ref = *ptr;

        m_ctxComponents.emplace_back(std::move(ptr));
        m_ctxComponentsMap[id] = &ref;

        return ref;
    }

private:
    std::vector<std::unique_ptr<ICtxComponent>> m_ctxComponents;
    std::unordered_map<size_t, ICtxComponent*> m_ctxComponentsMap;
};

struct GraphicsCtxComponent : ICtxComponent {
    vk::Queue graphicsQueue;
    QueueFamilyIndices queueFamilyIndices;
};

struct InstanceExtensionsComponent : ICtxComponent {
    std::vector<const char*> instanceExtensions;
};

struct InstanceLayersComponent : ICtxComponent {
    std::vector<const char*> instanceLayers;
};

struct DebugMessengerCtxComponent : ICtxComponent {
    vk::raii::DebugUtilsMessengerEXT debugMessenger{nullptr};
};

struct PipelineCtxComponent : ICtxComponent {
    vk::raii::DescriptorSetLayout descriptorSetLayout{nullptr};
    vk::raii::PipelineLayout pipelineLayout{nullptr};
    vk::raii::Pipeline graphicsPipeline{nullptr};
};

struct PipelineConfigComponent : ICtxComponent {
    vk::Format colorAttachmentFormat = vk::Format::eB8G8R8A8Unorm;
};

struct DeviceRequirementsComponent : ICtxComponent {
    bool requirePresent = false;
    std::vector<const char*> additionalDeviceExtensions;
};

struct PhysicalDevicePreferenceComponent : ICtxComponent {
    std::optional<uint32_t> preferredVendorId;
    std::optional<uint32_t> preferredDeviceId;
};

struct PresentQueueComponent : ICtxComponent {
    vk::Queue presentQueue;
};

struct SwapchainCtxComponent : ICtxComponent {
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

struct RenderPassCtxComponent : ICtxComponent {
    vk::raii::RenderPass renderPass{nullptr};
    std::vector<vk::raii::Framebuffer> framebuffers;
};

} // namespace nuff::renderer

