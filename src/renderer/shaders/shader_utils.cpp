#include "shaders/shader_utils.h"

namespace nuff::renderer::shaders {

vk::raii::ShaderModule createShaderModule(const vk::raii::Device& device, const std::vector<char>& code) {
    vk::ShaderModuleCreateInfo createInfo{
        {},
        code.size(),
        reinterpret_cast<const uint32_t*>(code.data())
    };
    
    return vk::raii::ShaderModule(device, createInfo);
}

} // namespace nuff::renderer::shaders

