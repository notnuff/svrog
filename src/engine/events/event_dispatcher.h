#pragma once
#include <type_traits>

#include "i_event.h"
#include "../systems/type_id_system.h"

namespace nuff::engine::events {

class EventDispatcher {
private:
    const IEvent& m_event;

public:
    explicit EventDispatcher(const IEvent& event) : m_event(event) {}
    ~EventDispatcher() = default;

    template<typename EventType, typename FuncType>
    requires std::is_base_of_v<IEvent, EventType>
    bool dispatch(const FuncType& handler) {
        if (m_event.typeId() == TypeIDSystem::getTypeID<EventType>()) {
            handler(static_cast<const EventType&>(m_event));
            return true;
        }
        return false;
    }
};

} // namespace nuff::engine
