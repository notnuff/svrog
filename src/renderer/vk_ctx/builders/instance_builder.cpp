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

InstanceBuilder& InstanceBuilder::addValidationLayers(const std::vector<const char*>& layers) {
    m_validationLayers.insert(m_validationLayers.end(), layers.begin(), layers.end());
    return *this;
}

InstanceBuilder& InstanceBuilder::enableValidation(bool enable) {
    m_enableValidation = enable;
    if (enable && m_validationLayers.empty()) {
        m_validationLayers.push_back("VK_LAYER_KHRONOS_validation");
    }
    return *this;
}

void InstanceBuilder::build(VkCtx& ctx) {
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
        m_enableValidation ? static_cast<uint32_t>(m_validationLayers.size()) : 0,
        m_enableValidation ? m_validationLayers.data() : nullptr,
        static_cast<uint32_t>(m_extensions.size()),
        m_extensions.data()
    };

    ctx.instance = vk::createInstance(createInfo);
    qCInfo(logger()) << "Vulkan instance created";
}

} // namespace nuff::renderer

