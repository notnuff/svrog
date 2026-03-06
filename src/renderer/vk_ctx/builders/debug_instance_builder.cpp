#include "vk_ctx/builders/debug_instance_builder.h"

namespace nuff::renderer {

DebugInstanceBuilder::DebugInstanceBuilder() {
    // Add debug extensions
    addExtensions({VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
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
        {},
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        debugCallback,
        this
    };

    ctx.debugMessenger = vk::raii::DebugUtilsMessengerEXT(ctx.instance, createInfo);
    qCInfo(logger()) << "Debug messenger created successfully";
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugInstanceBuilder::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    (void)messageType;

    auto* builder = static_cast<DebugInstanceBuilder*>(pUserData);
    const QLoggingCategory& log = builder->logger();

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        qCCritical(log) << "[Vulkan Validation]" << pCallbackData->pMessage;
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        qCWarning(log) << "[Vulkan Validation]" << pCallbackData->pMessage;
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        qCInfo(log) << "[Vulkan Validation]" << pCallbackData->pMessage;
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        qCDebug(log) << "[Vulkan Validation]" << pCallbackData->pMessage;
    }

    return VK_FALSE;
}

} // namespace nuff::renderer

