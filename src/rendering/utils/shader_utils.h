#pragma once

#include "common/vk_common.h"
#include <vector>

namespace nuff::renderer::utils {

vk::raii::ShaderModule createShaderModule(const vk::raii::Device& device, const std::vector<char>& code);

} // namespace nuff::renderer::shaders

