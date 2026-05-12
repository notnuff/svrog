#include "mesh_component.h"

#include "../entities/entity.h"
#include "transform_component.h"

namespace nuff::engine {
MeshComponent::MeshComponent(MeshHandle *mesh, MaterialHandle *material)
: m_mesh(mesh), m_material(material)
{}

void MeshComponent::init() {}

void MeshComponent::update(float /*deltaTime*/) {}

void MeshComponent::render() {
    if (!m_mesh || !m_material) {
        return;
    }

    auto* transform = owner()->getComponent<TransformComponent>();
    if (!transform) {
        return;
    }

    // todo rendering
}

void MeshComponent::setMesh(MeshHandle *mesh) {
    m_mesh = mesh;
}

MeshHandle * MeshComponent::mesh() const {
    return m_mesh;
}

void MeshComponent::setMaterial(MaterialHandle *material) {
    m_material = material;
}

MaterialHandle * MeshComponent::material() const {
    return m_material;
}

} // namespace nuff::engine
