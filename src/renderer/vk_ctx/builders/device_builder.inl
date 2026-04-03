#pragma once

namespace nuff::renderer::detail {

inline bool is_a_implies_b(const vk::Bool32* req, const vk::Bool32* sup, std::size_t count) {
    for (std::size_t i = 0; i < count; ++i) {
        if (req[i] && !sup[i]) {
            return false;
        }
    }
    return true;
}

template <typename FeatureStruct>
inline bool hasAllFlags(const FeatureStruct& required, const FeatureStruct& supported) {
    static_assert(requires { required.pNext; }, "FeatureStruct must be a chainable Vulkan struct!");

    constexpr std::size_t usefulDataOffset = offsetof(FeatureStruct, pNext) + sizeof(FeatureStruct::pNext);
    constexpr std::size_t boolCount  = (sizeof(FeatureStruct) - usefulDataOffset) / sizeof(vk::Bool32);

    return is_a_implies_b(
        reinterpret_cast<const vk::Bool32*>(reinterpret_cast<const char*>(&required) + usefulDataOffset),
        reinterpret_cast<const vk::Bool32*>(reinterpret_cast<const char*>(&supported) + usefulDataOffset),
        boolCount);
}

} // namespace nuff::renderer::detail

namespace nuff::renderer {

template <typename... Features>
bool DeviceBuilder::deviceHasRequiredFeatures(const vk::PhysicalDevice device,
                                              const vk::StructureChain<Features...>& req) {
    auto supported = device.getFeatures2<Features...>();
    return (detail::hasAllFlags(req.template get<Features>(),
                                supported.template get<Features>()) && ...);
}

} // namespace nuff::renderer

