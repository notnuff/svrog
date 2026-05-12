#pragma once

#include "../systems/type_id_system.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace nuff::engine {

class EventBus;

class SubscriptionHandle {
public:
    SubscriptionHandle() = default;
    SubscriptionHandle(EventBus* bus, size_t typeId, uint64_t subId)
        : m_bus(bus), m_typeId(typeId), m_subId(subId) {}

    SubscriptionHandle(const SubscriptionHandle&) = delete;
    SubscriptionHandle& operator=(const SubscriptionHandle&) = delete;

    SubscriptionHandle(SubscriptionHandle&& o) noexcept
        : m_bus(o.m_bus), m_typeId(o.m_typeId), m_subId(o.m_subId) {
        o.m_bus = nullptr;
    }
    SubscriptionHandle& operator=(SubscriptionHandle&& o) noexcept {
        if (this != &o) {
            reset();
            m_bus = o.m_bus; m_typeId = o.m_typeId; m_subId = o.m_subId;
            o.m_bus = nullptr;
        }
        return *this;
    }
    ~SubscriptionHandle() { reset(); }

    void reset();

private:
    EventBus* m_bus = nullptr;
    size_t    m_typeId = 0;
    uint64_t  m_subId = 0;
};

class EventBus {
public:
    template <typename E>
    SubscriptionHandle subscribe(std::function<void(const E&)> handler);

    template <typename E>
    void publish(const E& event);

    void unsubscribe(size_t typeId, uint64_t subId);

private:
    struct Entry {
        uint64_t id;
        std::function<void(const void*)> fn;
    };
    std::unordered_map<size_t, std::vector<Entry>> m_subs;
    uint64_t m_nextSubId = 1;
};

inline void SubscriptionHandle::reset() {
    if (m_bus) {
        m_bus->unsubscribe(m_typeId, m_subId);
        m_bus = nullptr;
    }
}

inline void EventBus::unsubscribe(size_t typeId, uint64_t subId) {
    auto it = m_subs.find(typeId);
    if (it == m_subs.end()) return;
    auto& vec = it->second;
    vec.erase(std::remove_if(vec.begin(), vec.end(),
                  [subId](const Entry& e){ return e.id == subId; }),
              vec.end());
}

template <typename E>
SubscriptionHandle EventBus::subscribe(std::function<void(const E&)> handler) {
    const size_t typeId = TypeIDSystem::getTypeID<E>();
    const uint64_t subId = m_nextSubId++;
    m_subs[typeId].push_back({subId,
        [h = std::move(handler)](const void* p) {
            h(*static_cast<const E*>(p));
        }});
    return SubscriptionHandle{this, typeId, subId};
}

template <typename E>
void EventBus::publish(const E& event) {
    const size_t typeId = TypeIDSystem::getTypeID<E>();
    auto it = m_subs.find(typeId);
    if (it == m_subs.end()) return;
    auto snapshot = it->second;
    for (auto& e : snapshot) e.fn(&event);
}

} // namespace nuff::engine
