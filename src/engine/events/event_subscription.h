#pragma once

#include <QtCore/qtclasshelpermacros.h>

#include <cstddef>

namespace nuff::engine::events {

class EventBus;
class IEventListener;

class EventSubscription {
    EventBus*       m_bus = nullptr;
    size_t          m_typeId = 0;
    IEventListener* m_listener = nullptr;

public:
    EventSubscription() = default;
    EventSubscription(EventBus* bus, size_t typeId, IEventListener* listener)
        : m_bus(bus), m_typeId(typeId), m_listener(listener) {}

    Q_DISABLE_COPY(EventSubscription)

    EventSubscription(EventSubscription&& other) noexcept
        : m_bus(other.m_bus), m_typeId(other.m_typeId), m_listener(other.m_listener) {
        other.m_bus = nullptr;
        other.m_listener = nullptr;
    }

    EventSubscription& operator=(EventSubscription&& other) noexcept;
    ~EventSubscription();

    void release();
    bool active() const { return m_bus != nullptr; }
};

} // namespace nuff::engine
