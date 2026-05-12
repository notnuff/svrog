#include "i_component.h"

namespace nuff::engine {

void IComponent::setOwner(Entity* owner) {
    m_owner = owner;
}

Entity* IComponent::owner() const {
    return m_owner;
}

} // namespace nuff::engine
