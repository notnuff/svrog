#pragma once

#include "core/builders/instance_builder.h"

namespace nuff::renderer {

// DebugInstanceBuilder - Configures validation layers and debug extensions
// Must run BEFORE InstanceBuilder
class DebugInstanceBuilder : public IVkBuilder {
public:
    DebugInstanceBuilder();

    void build(CoreCtx& ctx) override;
};

// DebugMessengerBuilder - Sets up debug messenger after instance creation
// Must run AFTER InstanceBuilder
class DebugMessengerBuilder : public IVkBuilder {
public:
    void build(CoreCtx& ctx) override;

private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        vk::DebugUtilsMessageTypeFlagsEXT messageType,
        const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};

} // namespace nuff::renderer

