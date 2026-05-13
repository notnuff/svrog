#include "event_bus.h"

#include <algorithm>
#include <utility>

namespace nuff::engine::events {

void EventBus::unsubscribeById(size_t typeId, IEventListener* listener) {
    assertOwnerThread();
    auto it = m_listeners.find(typeId);
    if (it == m_listeners.end()) return;
    auto& bucket = it->second;
    bucket.erase(std::remove(bucket.begin(), bucket.end(), listener), bucket.end());
}

void EventBus::sendEvent(const IEvent& event) {
    assertOwnerThread();
    auto it = m_listeners.find(event.typeId());
    if (it == m_listeners.end()) return;
    const auto snapshot = it->second;
    for (auto* listener : snapshot) {
        listener->onEvent(event);
    }
}

void EventBus::processEvents() {
    assertOwnerThread();

    std::vector<std::pair<size_t, IEventBuffer*>> drainList;
    drainList.reserve(m_buffers.size());
    for (auto& [id, buf] : m_buffers) drainList.emplace_back(id, buf.get());

    for (auto& [id, buf] : drainList) {
        auto lit = m_listeners.find(id);
        if (lit == m_listeners.end() || lit->second.empty()) {
            buf->clear();
            continue;
        }
        const auto snapshot = lit->second;
        buf->dispatchAll(snapshot);
    }
}

} // namespace nuff::engine
