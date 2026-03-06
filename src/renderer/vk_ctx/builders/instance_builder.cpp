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
        m_appName.c_str(),
        VK_MAKE_VERSION(1, 0, 0),
        m_engineName.c_str(),
        VK_MAKE_VERSION(1, 0, 0),
        vk::ApiVersion14
    };

    vk::InstanceCreateInfo createInfo{
        {},
        &appInfo,
        0,
        nullptr,
        static_cast<uint32_t>(m_extensions.size()),
        m_extensions.data()
    };

    ctx.instance = vk::raii::Instance(ctx.context, createInfo);
    qCInfo(logger()) << "Vulkan instance created";
}

} // namespace nuff::renderer

