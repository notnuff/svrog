#pragma once

#include <type_traits>
#include <vector>

#include "event_subscription.h"
#include "i_event.h"

namespace nuff::engine::events {

class EventBus;

class IEventListener {
public:
    virtual ~IEventListener() = default;
    virtual void onEvent(const IEvent& event) = 0;

protected:
    template<typename EventType>
    requires std::is_base_of_v<IEvent, EventType>
    void subscribeTo(EventBus& bus);

    void unsubscribeAll() { m_subscriptions.clear(); }

private:
    std::vector<EventSubscription> m_subscriptions;
};

} // namespace nuff::engine
