#include "core/builders/debug_instance_builder.h"

namespace L {
Q_LOGGING_CATEGORY(vkValidationLayer, "nuff.renderer.vk.validation")
}

namespace nuff::renderer {

DebugInstanceBuilder::DebugInstanceBuilder() {
}

void DebugInstanceBuilder::build(CoreCtx& ctx) {
    qCInfo(logger()) << "Configuring debug validation layers and extensions";

    auto& extLayers = ctx.extension<InstanceLayersMixin>();
    extLayers.instanceLayers.push_back("VK_LAYER_KHRONOS_validation");

    auto& extExtensions = ctx.extension<InstanceExtensionsMixin>();
    extExtensions.instanceExtensions.push_back(vk::EXTDebugUtilsExtensionName);
}

void DebugMessengerBuilder::build(CoreCtx& ctx) {
    qCInfo(logger()) << "Setting up debug messenger";

    auto& extMessenger = ctx.extension<DebugMessengerCtxMixin>();

    vk::DebugUtilsMessengerCreateInfoEXT createInfo{
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = debugCallback,
    };

    extMessenger.debugMessenger = vk::raii::DebugUtilsMessengerEXT(ctx.instance, createInfo);
    qCInfo(logger()) << "Debug messenger created successfully";
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerBuilder::debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    vk::DebugUtilsMessageTypeFlagsEXT messageType,
    const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    (void)messageType;
    (void)pUserData;

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

