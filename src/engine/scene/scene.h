#pragma once

#include "../entities/entity.h"
#include "scene_events.h"

#include <QtCore/qtclasshelpermacros.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace nuff::engine {

namespace events { class EventBus; }

class Scene {
public:
    explicit Scene(std::string name);
    ~Scene();

    Q_DISABLE_COPY(Scene)

    const std::string& name() const;
    void setName(std::string name);

    Entity* createEntity(std::string name, Entity* parent = nullptr);
    void    destroyEntity(EntityID id);
    Entity* find(EntityID id) const;

    void reparent(EntityID id, EntityID newParent);

    void setActiveCamera(EntityID id);
    Entity* activeCamera() const;

    void init();
    void update(float dt);

    std::vector<Entity*> rootEntities() const;
    const std::vector<std::unique_ptr<Entity>>& entities() const;

    events::EventBus& events();

private:
    void destroyRecursive(Entity* entity);

    std::string m_name;
    std::vector<std::unique_ptr<Entity>> m_entities;
    std::unordered_map<EntityID, Entity*> m_byId;
    EntityID m_nextId = 1;
    EntityID m_activeCameraId = 0;
    std::unique_ptr<events::EventBus> m_events;
};

} // namespace nuff::engine
