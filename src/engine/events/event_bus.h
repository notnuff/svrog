#pragma once

#include <QtCore/qtclasshelpermacros.h>

#include <cassert>
#include <cstddef>
#include <memory>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "event_subscription.h"
#include "i_event.h"
#include "i_event_listener.h"
#include "../systems/type_id_system.h"

namespace nuff::engine::events {

class EventBus {
    struct IEventBuffer {
        virtual ~IEventBuffer() = default;
        virtual void dispatchAll(const std::vector<IEventListener*>& listeners) = 0;
        virtual void clear() = 0;
    };

    template<typename EventType>
    struct EventBuffer : IEventBuffer {
        std::vector<std::unique_ptr<EventType>> events;

        void dispatchAll(const std::vector<IEventListener*>& listeners) override {
            auto drained = std::move(events);
            events.clear();
            for (auto& evt : drained) {
                for (auto* l : listeners) l->onEvent(*evt);
            }
        }
        void clear() override { events.clear(); }
    };

    std::unordered_map<size_t, std::vector<IEventListener*>> m_listeners;
    std::unordered_map<size_t, std::unique_ptr<IEventBuffer>> m_buffers;

    const std::thread::id m_ownerThread = std::this_thread::get_id();

    void assertOwnerThread() const {
        auto t = std::this_thread::get_id();
        assert(t == m_ownerThread
            && "EventBus accessed from non-owner thread");
    }

    template<typename EventType>
    EventBuffer<EventType>& bufferFor();

public:
    EventBus() {

    };
    ~EventBus() = default;

    Q_DISABLE_COPY_MOVE(EventBus)

    template<typename EventType>
    requires std::is_base_of_v<IEvent, EventType>
    [[nodiscard]] EventSubscription subscribe(IEventListener* listener);

    template<typename EventType>
    requires std::is_base_of_v<IEvent, EventType>
    void unsubscribe(IEventListener* listener);

    void unsubscribeById(size_t typeId, IEventListener* listener);

    void sendEvent(const IEvent& event);

    template<typename EventType>
    requires std::is_base_of_v<IEvent, EventType>
    void postEvent(std::unique_ptr<EventType> event);

    template<typename EventType, typename... Args>
    requires std::is_base_of_v<IEvent, EventType> && std::is_constructible_v<EventType, Args...>
    void emplaceEvent(Args&&... args);

    void processEvents();
};

} // namespace nuff::engine

#include "event_bus.inl"
#include "i_event_listener.inl"
#include "event_subscription.inl"
