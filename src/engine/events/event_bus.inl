#pragma once

#include <utility>

namespace nuff::engine::events {

template<typename EventType>
EventBus::EventBuffer<EventType>& EventBus::bufferFor() {
    const auto id = TypeIDSystem::getTypeID<EventType>();
    auto it = m_buffers.find(id);
    if (it == m_buffers.end()) {
        it = m_buffers.emplace(id, std::make_unique<EventBuffer<EventType>>()).first;
    }
    return static_cast<EventBuffer<EventType>&>(*it->second);
}

template<typename EventType>
requires std::is_base_of_v<IEvent, EventType>
EventSubscription EventBus::subscribe(IEventListener* listener) {
    assertOwnerThread();
    const auto id = TypeIDSystem::getTypeID<EventType>();
    m_listeners[id].push_back(listener);
    return EventSubscription{this, id, listener};
}

template<typename EventType>
requires std::is_base_of_v<IEvent, EventType>
void EventBus::unsubscribe(IEventListener* listener) {
    unsubscribeById(TypeIDSystem::getTypeID<EventType>(), listener);
}

template<typename EventType>
requires std::is_base_of_v<IEvent, EventType>
void EventBus::postEvent(std::unique_ptr<EventType> event) {
    assertOwnerThread();
    if (!event) return;
    bufferFor<EventType>().events.push_back(std::move(event));
}

template<typename EventType, typename... Args>
requires std::is_base_of_v<IEvent, EventType> && std::is_constructible_v<EventType, Args...>
void EventBus::emplaceEvent(Args&&... args) {
    assertOwnerThread();
    bufferFor<EventType>().events.push_back(std::make_unique<EventType>(std::forward<Args>(args)...));
}

} // namespace nuff::engine
