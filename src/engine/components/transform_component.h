#pragma once

#include "i_component.h"

#include "common/glm_common.h"
#include <glm/gtc/quaternion.hpp>

namespace nuff::engine {

class TransformComponent : public IComponent {
public:
    TransformComponent() = default;
    TransformComponent(glm::vec3 position, glm::quat rotation, glm::vec3 scale);

    void init() override;
    void update(float deltaTime) override;
    void render() override;

    void setPosition(const glm::vec3& position);
    const glm::vec3& position() const;

    void setRotation(const glm::quat& rotation);
    const glm::quat& rotation() const;

    void setScale(const glm::vec3& scale);
    const glm::vec3& scale() const;

    void setEulerAngles(const glm::vec3& eulerRadians);
    glm::vec3 eulerAngles() const;

    glm::mat4 localMatrix() const;
    glm::mat4 worldMatrix() const;

    glm::vec3 forward() const;
    glm::vec3 right() const;
    glm::vec3 up() const;

private:
    glm::vec3 m_position{0.0f};
    glm::quat m_rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 m_scale{1.0f};
};

} // namespace nuff::engine
