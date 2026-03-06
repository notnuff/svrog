#pragma once

#include "vk_ctx/builders/i_vk_builder.h"

#include <string>
#include <vector>

namespace nuff::renderer {

// InstanceBuilder - Creates Vulkan instance
class InstanceBuilder : public IVkBuilder {
public:
    InstanceBuilder& setAppName(const std::string& name);
    InstanceBuilder& setEngineName(const std::string& name);
    InstanceBuilder& addExtensions(const std::vector<const char*>& extensions);
    InstanceBuilder& addValidationLayers(const std::vector<const char*>& layers);
    InstanceBuilder& enableValidation(bool enable = true);

    void build(VkCtx& ctx) override;

private:
    std::string m_appName = "VkApp";
    std::string m_engineName = "svrog";
    std::vector<const char*> m_extensions;
    std::vector<const char*> m_validationLayers;
    bool m_enableValidation = false;
};

} // namespace nuff::renderer

