#pragma once

#include "common/vk_common.h"
#include "primitives/mesh.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <vector>

namespace nuff::renderer {

struct DrawItem {
    const Mesh* mesh = nullptr;
    vk::DescriptorSet materialSet{};
    glm::mat4 model{1.0f};
};

struct FrameView {
    glm::mat4 view{1.0f};
    glm::mat4 proj{1.0f};
    float time = 0.0f;
    std::vector<DrawItem> items;
};

} // namespace nuff::renderer
