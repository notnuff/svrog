#include "mesh_component.h"

namespace nuff::engine {

MeshComponent::MeshComponent(const renderer::Mesh* mesh, vk::DescriptorSet materialSet)
    : m_mesh(mesh), m_materialSet(materialSet) {}

void MeshComponent::init() {}

void MeshComponent::update(float /*deltaTime*/) {}

void MeshComponent::render() {}

void MeshComponent::setMesh(const renderer::Mesh* mesh) { m_mesh = mesh; }
const renderer::Mesh* MeshComponent::mesh() const { return m_mesh; }

void MeshComponent::setMaterialSet(vk::DescriptorSet set) { m_materialSet = set; }
vk::DescriptorSet MeshComponent::materialSet() const { return m_materialSet; }

} // namespace nuff::engine
