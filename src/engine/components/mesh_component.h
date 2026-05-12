#pragma once

#include "i_component.h"

#include "common/glm_common.h"

#include <string>

namespace nuff::engine {

class MeshHandle;
class MaterialHandle;

class MeshComponent : public IComponent {
public:
    MeshComponent(MeshHandle* mesh = nullptr, MaterialHandle* material = nullptr);

    void init() override;
    void update(float deltaTime) override;
    void render() override;

    void setMesh(MeshHandle* mesh);
    MeshHandle* mesh() const;

    void setMaterial(MaterialHandle* material);
    MaterialHandle* material() const;

private:
    MeshHandle* m_mesh = nullptr;
    MaterialHandle* m_material = nullptr;
};

} // namespace nuff::engine
