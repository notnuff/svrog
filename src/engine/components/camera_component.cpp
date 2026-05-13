#include "camera_component.h"

#include "../entities/entity.h"
#include "transform_component.h"

#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>

namespace nuff::engine {

CameraComponent::CameraComponent(float fovRadians, float aspectRatio,
                                 float nearPlane, float farPlane)
    : m_fov(fovRadians)
    , m_aspectRatio(aspectRatio)
    , m_nearPlane(nearPlane)
    , m_farPlane(farPlane) {}

void CameraComponent::init() {}

void CameraComponent::update(float deltaTime) {
    auto* transform = m_owner->getComponent<TransformComponent>();
    transform->setPosition(transform->position() + transform->position() * deltaTime * 0.1f);
}

void CameraComponent::render() {}

void CameraComponent::setFov(float fovRadians) {
    m_fov = fovRadians;
}

float CameraComponent::fov() const {
    return m_fov;
}

void CameraComponent::setAspectRatio(float aspectRatio) {
    m_aspectRatio = aspectRatio;
}

float CameraComponent::aspectRatio() const {
    return m_aspectRatio;
}

void CameraComponent::setNearPlane(float nearPlane) {
    m_nearPlane = nearPlane;
}

float CameraComponent::nearPlane() const {
    return m_nearPlane;
}

void CameraComponent::setFarPlane(float farPlane) {
    m_farPlane = farPlane;
}

float CameraComponent::farPlane() const {
    return m_farPlane;
}

glm::mat4 CameraComponent::viewMatrix() const {
    auto* transform = m_owner->getComponent<TransformComponent>();
    if (!transform) {
        throw std::runtime_error("CameraComponent requires a TransformComponent on the same entity");
    }
    return glm::inverse(transform->worldMatrix());
}

glm::mat4 CameraComponent::projectionMatrix() const {
    glm::mat4 proj = glm::perspective(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
    proj[1][1] *= -1; // flip Y for Vulkan clip-space
    return proj;
}

} // namespace nuff::engine
