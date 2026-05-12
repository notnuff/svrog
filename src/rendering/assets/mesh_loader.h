#pragma once

#include "primitives/mesh_data.h"

#include <filesystem>
#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace nuff::renderer {

struct MaterialData {
    std::vector<unsigned char> pixels;
    uint32_t  width = 0;
    uint32_t  height = 0;
    glm::vec4 baseColorFactor{1.0f};
};

struct LoadedNode {
    MeshData     mesh;
    glm::mat4    transform{1.0f};
    MaterialData material;
};

struct LoadedModel {
    std::vector<LoadedNode> nodes;
};

class IMeshLoader {
public:
    virtual ~IMeshLoader() = default;

    [[nodiscard]] virtual LoadedModel load(const std::filesystem::path& path) = 0;
};

} // namespace nuff::renderer
