#pragma once

#include "i_component.h"

#include "common/glm_common.h"

namespace nuff::engine {

class CameraComponent : public IComponent {
public:
    CameraComponent() = default;
    CameraComponent(float fovRadians, float aspectRatio, float nearPlane, float farPlane);

    void init() override;
    void update(float deltaTime) override;
    void render() override;

    void setFov(float fovRadians);
    float fov() const;

    void setAspectRatio(float aspectRatio);
    float aspectRatio() const;

    void setNearPlane(float nearPlane);
    float nearPlane() const;

    void setFarPlane(float farPlane);
    float farPlane() const;

    // Requires the owning entity to have a TransformComponent.
    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix() const;

private:
    float m_fov{glm::radians(45.0f)};
    float m_aspectRatio{16.0f / 9.0f};
    float m_nearPlane{0.1f};
    float m_farPlane{1000.0f};
};

} // namespace nuff::engine
