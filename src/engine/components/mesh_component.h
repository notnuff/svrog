#pragma once

#include "i_component.h"

#include "common/vk_common.h"

namespace nuff::renderer { class Mesh; }

namespace nuff::engine {

class MeshComponent : public IComponent {
public:
    MeshComponent() = default;
    MeshComponent(const renderer::Mesh* mesh, vk::DescriptorSet materialSet);

    void init() override;
    void update(float deltaTime) override;
    void render() override;

    void setMesh(const renderer::Mesh* mesh);
    const renderer::Mesh* mesh() const;

    void setMaterialSet(vk::DescriptorSet set);
    vk::DescriptorSet materialSet() const;

private:
    const renderer::Mesh* m_mesh = nullptr;
    vk::DescriptorSet m_materialSet{};
};

} // namespace nuff::engine
