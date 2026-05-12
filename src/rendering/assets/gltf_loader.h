#pragma once

#include "assets/mesh_loader.h"

namespace nuff::renderer {

class GltfLoader : public IMeshLoader {
public:
    [[nodiscard]] LoadedModel load(const std::filesystem::path& path) override;
};

} // namespace nuff::renderer
