#pragma once

namespace nuff::engine::events {

template<typename EventType>
requires std::is_base_of_v<IEvent, EventType>
void IEventListener::subscribeTo(EventBus& bus) {
    m_subscriptions.emplace_back(bus.subscribe<EventType>(this));
}

} // namespace nuff::engine
