#pragma once

namespace nuff::engine {

class Entity;

class IComponent {
public:
    virtual ~IComponent() = default;

    virtual void init() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;

    void setOwner(Entity* owner);
    Entity* owner() const;

protected:
    Entity* m_owner{nullptr};

};

} // namespace nuff::engine
