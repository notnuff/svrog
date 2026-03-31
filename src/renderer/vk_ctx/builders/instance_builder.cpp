#include <ranges>

#include "vk_ctx/builders/instance_builder.h"

namespace nuff::renderer {

InstanceBuilder& InstanceBuilder::setAppName(const std::string& name) {
    m_appName = name;
    return *this;
}

InstanceBuilder& InstanceBuilder::setEngineName(const std::string& name) {
    m_engineName = name;
    return *this;
}

InstanceBuilder& InstanceBuilder::addExtensions(const std::vector<const char*>& extensions) {
    m_extensions.insert(m_extensions.end(), extensions.begin(), extensions.end());
    return *this;
}

void InstanceBuilder::build(VkCtx& ctx) {
    vk::ApplicationInfo appInfo{
        .pApplicationName = m_appName.c_str(),
        .applicationVersion = vk::makeApiVersion(0, 1, 0, 0),
        .pEngineName = m_engineName.c_str(),
        .engineVersion = vk::makeApiVersion(0, 1, 0, 0),
        .apiVersion = vk::ApiVersion14
    };

    vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(m_layers.size()),
        .ppEnabledLayerNames = m_layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(m_extensions.size()),
        .ppEnabledExtensionNames = m_extensions.data()
    };

    const auto& supportedExtensions = ctx.context.enumerateInstanceExtensionProperties();
    for (const auto* extension : m_extensions) {
        const auto extensionPresent = std::ranges::any_of(supportedExtensions,
            [extension](const auto& extensionProperty)
            { return strcmp(extensionProperty.extensionName, extension) == 0; });

        if (extensionPresent) {
            continue;
        }
        throw std::runtime_error("Required extension not supported: " + std::string(extension));
    }

    const auto& supportedLayers = ctx.context.enumerateInstanceLayerProperties();
    for (const auto* layer : m_layers) {
        const auto layerPresent = std::ranges::any_of(supportedLayers,
            [layer](const auto& layerProperty)
            { return strcmp(layerProperty.layerName, layer) == 0; });

        if (layerPresent) {
            continue;
        }
        throw std::runtime_error("Required layer not supported: " + std::string(layer));
    }

    ctx.instance = vk::raii::Instance(ctx.context, createInfo);
    qCInfo(logger()) << "Vulkan instance created";
}

} // namespace nuff::renderer

