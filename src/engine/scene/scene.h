#pragma once

#include "../entities/entity.h"
#include "../events/event_bus.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace nuff::engine {

struct EntityAddedEvent         { EntityID id; };
struct EntityRemovedEvent       { EntityID id; };
struct EntityReparentedEvent    { EntityID id; EntityID newParent; };
struct ComponentAddedEvent      { EntityID id; size_t componentTypeId; };
struct ComponentRemovedEvent    { EntityID id; size_t componentTypeId; };
struct ActiveCameraChangedEvent { EntityID id; };

class Scene {
public:
    explicit Scene(std::string name);
    ~Scene();

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

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

    EventBus& events();

private:
    void destroyRecursive(Entity* entity);

    std::string m_name;
    std::vector<std::unique_ptr<Entity>> m_entities;
    std::unordered_map<EntityID, Entity*> m_byId;
    EntityID m_nextId = 1;
    EntityID m_activeCameraId = 0;
    EventBus m_events;
};

} // namespace nuff::engine
