#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vector>

namespace nuff::renderer::shaders {

vk::raii::ShaderModule createShaderModule(const vk::raii::Device& device, const std::vector<char>& code);

} // namespace nuff::renderer::shaders

