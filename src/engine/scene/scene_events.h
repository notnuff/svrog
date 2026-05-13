#pragma once

#include <cstddef>

#include "../entities/entity.h"
#include "../events/i_event.h"

namespace nuff::engine {

struct EntityAddedEvent : public events::Event<EntityAddedEvent> {
    EntityID id;
    explicit EntityAddedEvent(EntityID id) : id(id) {}
};

struct EntityRemovedEvent : public events::Event<EntityRemovedEvent> {
    EntityID id;
    explicit EntityRemovedEvent(EntityID id) : id(id) {}
};

struct EntityReparentedEvent : public events::Event<EntityReparentedEvent> {
    EntityID id;
    EntityID newParent;
    EntityReparentedEvent(EntityID id, EntityID newParent) : id(id), newParent(newParent) {}
};

struct ComponentAddedEvent : public events::Event<ComponentAddedEvent> {
    EntityID id;
    size_t componentTypeId;
    ComponentAddedEvent(EntityID id, size_t componentTypeId) : id(id), componentTypeId(componentTypeId) {}
};

struct ComponentRemovedEvent : public events::Event<ComponentRemovedEvent> {
    EntityID id;
    size_t componentTypeId;
    ComponentRemovedEvent(EntityID id, size_t componentTypeId) : id(id), componentTypeId(componentTypeId) {}
};

struct ActiveCameraChangedEvent : public events::Event<ActiveCameraChangedEvent> {
    EntityID id;
    explicit ActiveCameraChangedEvent(EntityID id) : id(id) {}
};

} // namespace nuff::engine
