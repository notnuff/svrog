#pragma once

#include "primitives/vertex.h"

#include <vector>

namespace nuff::renderer {

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

} // namespace nuff::renderer
