#include "default_camera.h"

#include <glm/gtc/quaternion.hpp>

#include "../components/transform_component.h"
#include "../entities/entity.h"
#include "../events/event_bus.h"
#include "../scene/scene.h"
#include "camera_component.h"
#include "fly_camera_controller.h"

namespace nuff::engine {

Entity* createDefaultCameraEntity(Scene& scene, const DefaultCameraConfig& cfg) {
    Entity* entity = scene.createEntity(cfg.name);

    auto* xf = entity->addComponent<TransformComponent>();
    xf->setPosition(cfg.position);
    const glm::vec3 dir = glm::normalize(cfg.lookAt - cfg.position);
    xf->setRotation(glm::quatLookAt(dir, cfg.worldUp));

    entity->addComponent<CameraComponent>(cfg.fovRadians, cfg.aspectRatio,
                                          cfg.nearPlane, cfg.farPlane);

    if (cfg.attachFlyController) {
        entity->addComponent<FlyCameraController>();
    }

    if (cfg.setAsActive) {
        scene.setActiveCamera(entity->id());
    }

    return entity;
}

} // namespace nuff::engine
