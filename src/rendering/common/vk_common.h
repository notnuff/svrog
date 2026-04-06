#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

// it would be nice to activate reflection
// so we could get rid of vk::Bool32 comparison nightmare in device_builder.inl,
// but that shit must be compiled on some supercomputer, as it requires enormous amount of memory
// #define VULKAN_HPP_USE_REFLECT
#include <vulkan/vulkan_raii.hpp>

