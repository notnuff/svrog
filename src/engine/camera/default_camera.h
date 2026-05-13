#pragma once

#include <string>

#include "../common/glm_common.h"

namespace nuff::engine {

class Scene;
class Entity;

struct DefaultCameraConfig {
    std::string name      = "Camera";
    glm::vec3 position    = {3.0f, 3.0f, 3.0f};
    glm::vec3 lookAt      = {0.0f, 0.0f, 0.0f};
    glm::vec3 worldUp     = {0.0f, 0.0f, 1.0f};
    float     fovRadians  = glm::radians(45.0f);
    float     aspectRatio = 16.0f / 9.0f;
    float     nearPlane   = 0.1f;
    float     farPlane    = 1000.0f;

    bool attachFlyController = true;
    bool setAsActive         = true;
};

Entity* createDefaultCameraEntity(Scene& scene, const DefaultCameraConfig& cfg = {});

} // namespace nuff::engine
