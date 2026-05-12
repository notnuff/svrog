#include "transform_component.h"

#include "../entities/entity.h"

#include <glm/gtc/matrix_transform.hpp>

namespace nuff::engine {

TransformComponent::TransformComponent(glm::vec3 position, glm::quat rotation, glm::vec3 scale)
    : m_position(position), m_rotation(rotation), m_scale(scale) {}

void TransformComponent::init() {}

void TransformComponent::update(float /*deltaTime*/) {}

void TransformComponent::render() {}

void TransformComponent::setPosition(const glm::vec3& position) {
    m_position = position;
}

const glm::vec3& TransformComponent::position() const {
    return m_position;
}

void TransformComponent::setRotation(const glm::quat& rotation) {
    m_rotation = rotation;
}

const glm::quat& TransformComponent::rotation() const {
    return m_rotation;
}

void TransformComponent::setScale(const glm::vec3& scale) {
    m_scale = scale;
}

const glm::vec3& TransformComponent::scale() const {
    return m_scale;
}

void TransformComponent::setEulerAngles(const glm::vec3& eulerRadians) {
    m_rotation = glm::quat(eulerRadians);
}

glm::vec3 TransformComponent::eulerAngles() const {
    return glm::eulerAngles(m_rotation);
}

// TODO use caching for matrix calculations
glm::mat4 TransformComponent::localMatrix() const {
    const glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_position);
    const glm::mat4 rotation    = glm::mat4_cast(m_rotation);
    const glm::mat4 scaling     = glm::scale(glm::mat4(1.0f), m_scale);
    return translation * rotation * scaling;
}

glm::mat4 TransformComponent::worldMatrix() const {
    glm::mat4 parentWorld(1.0f);
    if (Entity* p = m_owner ? m_owner->parent() : nullptr) {
        if (auto* parentXf = p->getComponent<TransformComponent>()) {
            parentWorld = parentXf->worldMatrix();
        }
    }
    return parentWorld * localMatrix();
}

glm::vec3 TransformComponent::forward() const {
    return glm::normalize(m_rotation * glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::vec3 TransformComponent::right() const {
    return glm::normalize(m_rotation * glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 TransformComponent::up() const {
    return glm::normalize(m_rotation * glm::vec3(0.0f, 1.0f, 0.0f));
}

} // namespace nuff::engine
