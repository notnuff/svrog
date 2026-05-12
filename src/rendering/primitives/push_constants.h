#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace nuff::renderer {

struct ObjectPushConstantData {
    glm::mat4 model{1.0f};
    float time = 0.0f;
};

} // namespace nuff::renderer
