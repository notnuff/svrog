#pragma once

#include <cstddef>

#include "../systems/type_id_system.h"

namespace nuff::engine::events {

class IEvent {
public:
    virtual ~IEvent() = default;

    virtual size_t typeId() const = 0;
};

template<typename Derived>
class Event : public IEvent {
public:
    size_t typeId() const override { return TypeIDSystem::getTypeID<Derived>(); }
};

} // namespace nuff::engine
