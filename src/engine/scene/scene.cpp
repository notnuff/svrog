#include "scene.h"

#include "../events/event_bus.h"

#include <algorithm>
#include <utility>

namespace nuff::engine {

Scene::Scene(std::string name)
    : m_name(std::move(name)),
      m_events(std::make_unique<events::EventBus>()) {}
Scene::~Scene() = default;

const std::string& Scene::name() const { return m_name; }
void Scene::setName(std::string name) { m_name = std::move(name); }

Entity* Scene::createEntity(std::string name, Entity* parent) {
    auto entity = std::make_unique<Entity>(m_nextId++, std::move(name));
    Entity* raw = entity.get();
    raw->setScene(this);
    m_byId[raw->id()] = raw;
    m_entities.push_back(std::move(entity));

    if (parent) raw->setParent(parent);

    m_events->sendEvent(EntityAddedEvent{raw->id()});
    return raw;
}

void Scene::destroyRecursive(Entity* entity) {
    const auto children = entity->children();
    for (Entity* child : children) {
        destroyRecursive(child);
    }
    const EntityID id = entity->id();
    if (id == m_activeCameraId) m_activeCameraId = 0;
    m_byId.erase(id);
    const auto it = std::find_if(m_entities.begin(), m_entities.end(),
        [entity](const std::unique_ptr<Entity>& e){ return e.get() == entity; });
    if (it != m_entities.end()) {
        m_entities.erase(it);
    }
    m_events->sendEvent(EntityRemovedEvent{id});
}

void Scene::destroyEntity(EntityID id) {
    const auto it = m_byId.find(id);
    if (it == m_byId.end()) return;
    destroyRecursive(it->second);
}

Entity* Scene::find(EntityID id) const {
    const auto it = m_byId.find(id);
    return it == m_byId.end() ? nullptr : it->second;
}

void Scene::reparent(EntityID id, EntityID newParent) {
    Entity* e = find(id);
    if (!e) return;
    Entity* p = newParent ? find(newParent) : nullptr;
    if (newParent && !p) return;
    e->setParent(p);
    m_events->sendEvent(EntityReparentedEvent{id, newParent});
}

void Scene::setActiveCamera(EntityID id) {
    if (id == m_activeCameraId) return;
    m_activeCameraId = id;
    m_events->sendEvent(ActiveCameraChangedEvent{id});
}

Entity* Scene::activeCamera() const {
    return find(m_activeCameraId);
}

void Scene::init() {
    for (auto& e : m_entities) e->init();
}

void Scene::update(float dt) {
    for (auto& e : m_entities) e->update(dt);
}

std::vector<Entity*> Scene::rootEntities() const {
    std::vector<Entity*> roots;
    roots.reserve(m_entities.size());
    for (const auto& e : m_entities) {
        if (!e->parent()) roots.push_back(e.get());
    }
    return roots;
}

const std::vector<std::unique_ptr<Entity>>& Scene::entities() const {
    return m_entities;
}

events::EventBus& Scene::events() { return *m_events; }

} // namespace nuff::engine
