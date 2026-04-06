#pragma once

#include "vk_ctx/builders/instance_builder.h"

namespace nuff::renderer {

// DebugInstanceBuilder - Creates Vulkan instance with debug features enabled
class DebugInstanceBuilder : public InstanceBuilder {
public:
    DebugInstanceBuilder();

    void build(VkCtx& ctx) override;

private:
    void setupDebugMessenger(VkCtx& ctx);

    std::vector<const char*> m_validationLayers = {"VK_LAYER_KHRONOS_validation"};

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        vk::DebugUtilsMessageTypeFlagsEXT messageType,
        const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};

} // namespace nuff::renderer

