#include "entity.h"

#include <utility>

namespace nuff::engine {

Entity::Entity(EntityID id, std::string name)
    : m_id(id), m_name(std::move(name)) {}

Entity::~Entity() {
    if (m_parent) {
        auto& siblings = m_parent->m_children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this),
                       siblings.end());
        m_parent = nullptr;
    }
    for (Entity* child : m_children) {
        child->m_parent = nullptr;
    }
    m_children.clear();
}

EntityID Entity::id() const { return m_id; }

const std::string& Entity::name() const { return m_name; }
void Entity::setName(std::string name) { m_name = std::move(name); }

Scene* Entity::scene() const { return m_scene; }
void   Entity::setScene(Scene* scene) { m_scene = scene; }

void Entity::init() {
    for (auto& component : m_components) {
        component->init();
    }
}

void Entity::update(float deltaTime) {
    if (!m_active) return;
    for (auto& component : m_components) {
        component->update(deltaTime);
    }
}

void Entity::render() {
    if (!m_active) return;
    for (auto& component : m_components) {
        component->render();
    }
}

void Entity::setActive(bool active) { m_active = active; }
bool Entity::isActive() const { return m_active; }

Entity* Entity::parent() const { return m_parent; }
const std::vector<Entity*>& Entity::children() const { return m_children; }

bool Entity::isAncestorOf(const Entity* other) const {
    for (const Entity* n = other ? other->m_parent : nullptr; n; n = n->m_parent) {
        if (n == this) return true;
    }
    return false;
}

void Entity::setParent(Entity* parent) {
    if (parent == m_parent) return;
    if (parent == this) return;
    if (parent && isAncestorOf(parent)) return;

    if (m_parent) {
        auto& siblings = m_parent->m_children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this),
                       siblings.end());
    }
    m_parent = parent;
    if (m_parent) {
        m_parent->m_children.push_back(this);
    }
}

const std::vector<std::unique_ptr<IComponent>>& Entity::components() const {
    return m_components;
}

} // namespace nuff::engine
