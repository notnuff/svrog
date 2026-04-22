#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace nuff::renderer {

struct UniformBufferObject {
    glm::mat4 model{1.0f};
    glm::mat4 view{1.0f};
    glm::mat4 proj{1.0f};
};

} // namespace nuff::renderer
