#pragma once

namespace nuff::engine::events {

inline EventSubscription& EventSubscription::operator=(EventSubscription&& other) noexcept {
    if (this != &other) {
        release();
        m_bus = other.m_bus;
        m_typeId = other.m_typeId;
        m_listener = other.m_listener;
        other.m_bus = nullptr;
        other.m_listener = nullptr;
    }
    return *this;
}

inline EventSubscription::~EventSubscription() { release(); }

inline void EventSubscription::release() {
    if (m_bus) {
        m_bus->unsubscribeById(m_typeId, m_listener);
        m_bus = nullptr;
        m_listener = nullptr;
    }
}

} // namespace nuff::engine
