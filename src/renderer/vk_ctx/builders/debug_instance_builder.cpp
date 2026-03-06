#include "vk_ctx/builders/debug_instance_builder.h"

namespace nuff::renderer {

DebugInstanceBuilder::DebugInstanceBuilder() {
    // Add debug extensions
    addExtensions({VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
}

void DebugInstanceBuilder::build(VkCtx& ctx) {
    qCInfo(logger()) << "Building Vulkan instance with debug features enabled";

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

    ctx.instance = vk::createInstance(createInfo);
    qCInfo(logger()) << "Vulkan instance created with validation layers";

    // Set up debug messenger
    setupDebugMessenger(ctx);
}

void DebugInstanceBuilder::setupDebugMessenger(VkCtx& ctx) {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = this;  // Pass this pointer to access logger

    // Load the function pointer for creating debug messenger
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        static_cast<VkInstance>(ctx.instance), "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) {
        VkResult result = func(static_cast<VkInstance>(ctx.instance),
                              &createInfo, nullptr, &ctx.debugMessenger);
        if (result == VK_SUCCESS) {
            qCInfo(logger()) << "Debug messenger created successfully";
        } else {
            qCWarning(logger()) << "Failed to create debug messenger";
        }
    } else {
        qCWarning(logger()) << "Debug utils extension not available";
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugInstanceBuilder::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    (void)messageType;

    // Retrieve the logger from the builder instance
    auto* builder = static_cast<DebugInstanceBuilder*>(pUserData);
    const QLoggingCategory& log = builder->logger();

    // Map Vulkan severity to Qt logging levels
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

