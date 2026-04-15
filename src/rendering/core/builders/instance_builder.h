#pragma once

#include "core/builders/i_vk_builder.h"

#include <string>
#include <vector>

namespace nuff::renderer {

// InstanceBuilder - Creates Vulkan instance
class InstanceBuilder : public IVkBuilder {
public:
    void build(CoreCtx& ctx) override;

protected:
    std::string m_appName = "VkApp";
    std::string m_engineName = "svrog";
    std::vector<const char*> m_extensions;
    std::vector<const char*> m_layers;
};

} // namespace nuff::renderer

