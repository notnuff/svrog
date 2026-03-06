#include "vk_ctx/builders/debug_instance_builder.h"

namespace nuff::renderer {

DebugInstanceBuilder::DebugInstanceBuilder() {
    // Add debug extensions
    addExtensions({VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
}

void DebugInstanceBuilder::build(VkCtx& ctx) {
    qCInfo(logger()) << "Building Vulkan instance with debug features enabled";

    // TODO: use inheritance to build debug instance, not copy-paste
    // Build instance with validation layers
    vk::ApplicationInfo appInfo{
        m_appName.c_str(),
        VK_MAKE_VERSION(1, 0, 0),
        m_engineName.c_str(),
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_3
    };

    vk::InstanceCreateInfo createInfo{
        {},
        &appInfo,
        static_cast<uint32_t>(m_validationLayers.size()),
        m_validationLayers.data(),
        static_cast<uint32_t>(m_extensions.size()),
        m_extensions.data()
    };

    ctx.instance = vk::raii::Instance(ctx.context, createInfo);
    qCInfo(logger()) << "Vulkan instance created with validation layers";

    setupDebugMessenger(ctx);
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
        this  // Pass this pointer to access logger
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

