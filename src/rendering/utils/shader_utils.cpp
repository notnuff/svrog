#include "shader_utils.h"

namespace nuff::renderer::utils {

vk::raii::ShaderModule createShaderModule(const vk::raii::Device& device, const std::vector<char>& code) {
    vk::ShaderModuleCreateInfo createInfo{
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data())
    };
    
    return vk::raii::ShaderModule(device, createInfo);
}

} // namespace nuff::renderer::utils

