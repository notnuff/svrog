#pragma once

#include "../components/i_component.h"
#include "../systems/type_id_system.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace nuff::engine {

class Scene;

using EntityID = uint64_t;

class Entity {
public:
    Entity(EntityID id, std::string name);
    virtual ~Entity();

    EntityID id() const;

    const std::string& name() const;
    void setName(std::string name);

    Scene* scene() const;
    void   setScene(Scene* scene);

    void init();
    void update(float deltaTime);
    void render();

    void setActive(bool active);
    bool isActive() const;

    Entity* parent() const;
    const std::vector<Entity*>& children() const;
    void setParent(Entity* parent);
    bool isAncestorOf(const Entity* other) const;

    const std::vector<std::unique_ptr<IComponent>>& components() const;

    template <typename ComponentType, typename... Args>
    ComponentType* addComponent(Args&&... args) {
        static_assert(std::is_base_of_v<IComponent, ComponentType>,
            "ComponentType must derive from Component");

        const size_t typeID = TypeIDSystem::getTypeID<ComponentType>();
        auto it = m_componentsMap.find(typeID);
        if (it != m_componentsMap.end()) {
            return static_cast<ComponentType*>(it->second);
        }

        auto component = std::make_unique<ComponentType>(std::forward<Args>(args)...);
        component->setOwner(this);

        ComponentType* rawPtr = component.get();
        m_components.push_back(std::move(component));
        m_componentsMap[typeID] = rawPtr;

        return rawPtr;
    }

    template <typename ComponentType>
    ComponentType* getComponent() {
        static_assert(std::is_base_of_v<IComponent, ComponentType>,
            "ComponentType must derive from Component");

        const size_t typeID = TypeIDSystem::getTypeID<ComponentType>();
        const auto it = m_componentsMap.find(typeID);
        if (it == m_componentsMap.end()) {
            return nullptr;
        }
        return static_cast<ComponentType*>(it->second);
    }

    template <typename ComponentType>
    bool removeComponent() {
        static_assert(std::is_base_of_v<IComponent, ComponentType>,
            "ComponentType must derive from Component");

        const size_t typeID = TypeIDSystem::getTypeID<ComponentType>();
        const auto mapIt = m_componentsMap.find(typeID);
        if (mapIt == m_componentsMap.end()) {
            return false;
        }
        IComponent* rawPtr = mapIt->second;
        m_componentsMap.erase(mapIt);
        m_components.erase(
            std::remove_if(m_components.begin(), m_components.end(),
                [rawPtr](const std::unique_ptr<IComponent>& c) { return c.get() == rawPtr; }),
            m_components.end());
        return true;
    }

private:
    EntityID    m_id;
    std::string m_name;
    Scene*      m_scene = nullptr;

    Entity*              m_parent = nullptr;
    std::vector<Entity*> m_children;

    std::vector<std::unique_ptr<IComponent>> m_components;
    std::unordered_map<size_t, IComponent*>  m_componentsMap;

    bool m_active = true;
};

} // namespace nuff::engine
