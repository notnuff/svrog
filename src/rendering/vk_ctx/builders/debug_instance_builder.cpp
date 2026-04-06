#include "vk_ctx/builders/debug_instance_builder.h"

namespace L {
Q_LOGGING_CATEGORY(vkValidationLayer, "nuff.renderer.vk.validation")
}

namespace nuff::renderer {

DebugInstanceBuilder::DebugInstanceBuilder() {
    // Add debug extensions
    addExtensions({vk::EXTDebugUtilsExtensionName});
}

void DebugInstanceBuilder::build(VkCtx& ctx) {
    qCInfo(logger()) << "Building Vulkan instance with debug features enabled";

    m_layers.insert(m_layers.end(), m_validationLayers.begin(), m_validationLayers.end());
    InstanceBuilder::build(ctx);

    setupDebugMessenger(ctx);

    qCInfo(logger()) << "Vulkan instance created with validation layers";
}

void DebugInstanceBuilder::setupDebugMessenger(VkCtx& ctx) {
    vk::DebugUtilsMessengerCreateInfoEXT createInfo{
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = debugCallback,
        .pUserData = this
    };

    ctx.debugMessenger = vk::raii::DebugUtilsMessengerEXT(ctx.instance, createInfo);
    qCInfo(logger()) << "Debug messenger created successfully";
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugInstanceBuilder::debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    vk::DebugUtilsMessageTypeFlagsEXT messageType,
    const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    (void)messageType;

    const auto& log = L::vkValidationLayer;
    if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError) {
        qCCritical(log) << "[Vulkan Validation]" << pCallbackData->pMessage;
    } else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
        qCWarning(log) << "[Vulkan Validation]" << pCallbackData->pMessage;
    } else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo) {
        qCInfo(log) << "[Vulkan Validation]" << pCallbackData->pMessage;
    } else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose) {
        qCDebug(log) << "[Vulkan Validation]" << pCallbackData->pMessage;
    }

    return vk::False;
}

} // namespace nuff::renderer

