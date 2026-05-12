#include "assets/gltf_loader.h"

#include <QLoggingCategory>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <tiny_gltf.h>

namespace L {
Q_LOGGING_CATEGORY(gltfLoader, "nuff.assets.gltf")
}

namespace nuff::renderer {

namespace {

glm::mat4 nodeTransform(const tinygltf::Node& node) {
    if (node.matrix.size() == 16) {
        glm::mat4 m(1.0f);
        for (int i = 0; i < 16; ++i) {
            m[i / 4][i % 4] = static_cast<float>(node.matrix[i]);
        }
        return m;
    }
    glm::mat4 t(1.0f), r(1.0f), s(1.0f);
    if (node.translation.size() == 3) {
        t = glm::translate(glm::mat4(1.0f),
            {static_cast<float>(node.translation[0]),
             static_cast<float>(node.translation[1]),
             static_cast<float>(node.translation[2])});
    }
    if (node.rotation.size() == 4) {
        glm::quat q(static_cast<float>(node.rotation[3]),
                    static_cast<float>(node.rotation[0]),
                    static_cast<float>(node.rotation[1]),
                    static_cast<float>(node.rotation[2]));
        r = glm::mat4_cast(q);
    }
    if (node.scale.size() == 3) {
        s = glm::scale(glm::mat4(1.0f),
            {static_cast<float>(node.scale[0]),
             static_cast<float>(node.scale[1]),
             static_cast<float>(node.scale[2])});
    }
    return t * r * s;
}

const std::byte* attrBytes(const tinygltf::Model& model, int accessorIdx,
                            size_t& count, size_t& stride, size_t elemSize) {
    const auto& acc  = model.accessors[accessorIdx];
    const auto& view = model.bufferViews[acc.bufferView];
    const auto& buf  = model.buffers[view.buffer];
    count = acc.count;
    int bs = acc.ByteStride(view);
    stride = bs > 0 ? static_cast<size_t>(bs) : elemSize;
    return reinterpret_cast<const std::byte*>(buf.data.data()) + view.byteOffset + acc.byteOffset;
}

void readIndices(const tinygltf::Model& model, const tinygltf::Accessor& acc,
                  std::vector<uint32_t>& out) {
    const auto& view = model.bufferViews[acc.bufferView];
    const auto* base = reinterpret_cast<const std::byte*>(model.buffers[view.buffer].data.data())
                       + view.byteOffset + acc.byteOffset;
    out.reserve(acc.count);
    switch (acc.componentType) {
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
        const auto* p = reinterpret_cast<const uint32_t*>(base);
        for (size_t i = 0; i < acc.count; ++i) out.push_back(p[i]);
        break;
    }
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
        const auto* p = reinterpret_cast<const uint16_t*>(base);
        for (size_t i = 0; i < acc.count; ++i) out.push_back(static_cast<uint32_t>(p[i]));
        break;
    }
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
        const auto* p = reinterpret_cast<const uint8_t*>(base);
        for (size_t i = 0; i < acc.count; ++i) out.push_back(static_cast<uint32_t>(p[i]));
        break;
    }
    default:
        qCWarning(L::gltfLoader) << "Unsupported index componentType:" << acc.componentType;
        break;
    }
}

MeshData buildMesh(const tinygltf::Model& model, const tinygltf::Primitive& prim) {
    MeshData out;
    if (prim.mode != TINYGLTF_MODE_TRIANGLES && prim.mode != -1) return out;

    auto posIt = prim.attributes.find("POSITION");
    if (posIt == prim.attributes.end()) return out;

    size_t vc = 0, ps = 0, ns = 0, us = 0, dummy = 0;
    const std::byte* posB = attrBytes(model, posIt->second, vc, ps, sizeof(float) * 3);
    const std::byte* nrmB = nullptr;
    const std::byte* uvB  = nullptr;
    if (auto it = prim.attributes.find("NORMAL"); it != prim.attributes.end())
        nrmB = attrBytes(model, it->second, dummy, ns, sizeof(float) * 3);
    if (auto it = prim.attributes.find("TEXCOORD_0"); it != prim.attributes.end())
        uvB = attrBytes(model, it->second, dummy, us, sizeof(float) * 2);

    out.vertices.reserve(vc);
    for (size_t i = 0; i < vc; ++i) {
        Vertex v{};
        const auto* p = reinterpret_cast<const float*>(posB + i * ps);
        v.pos = {p[0], p[1], p[2]};
        if (nrmB) {
            const auto* n = reinterpret_cast<const float*>(nrmB + i * ns);
            v.normal = {n[0], n[1], n[2]};
        } else {
            v.normal = {0.0f, 0.0f, 1.0f};
        }
        if (uvB) {
            const auto* u = reinterpret_cast<const float*>(uvB + i * us);
            v.texCoord = {u[0], u[1]};
        }
        out.vertices.push_back(v);
    }

    if (prim.indices >= 0) {
        readIndices(model, model.accessors[prim.indices], out.indices);
    } else {
        out.indices.resize(out.vertices.size());
        for (uint32_t i = 0; i < out.indices.size(); ++i) out.indices[i] = i;
    }
    return out;
}

MaterialData extractMaterial(const tinygltf::Model& m, int materialIdx) {
    MaterialData out;
    int textureIdx = -1;

    if (materialIdx >= 0 && materialIdx < static_cast<int>(m.materials.size())) {
        const auto& mat = m.materials[materialIdx];

        auto extIt = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
        if (extIt != mat.extensions.end() && extIt->second.IsObject()) {
            const auto& spec = extIt->second;
            if (spec.Has("diffuseFactor")) {
                const auto& df = spec.Get("diffuseFactor");
                if (df.IsArray() && df.ArrayLen() == 4) {
                    for (size_t i = 0; i < 4; ++i)
                        out.baseColorFactor[i] = static_cast<float>(df.Get(static_cast<int>(i)).GetNumberAsDouble());
                }
            }
            if (spec.Has("diffuseTexture")) {
                const auto& dt = spec.Get("diffuseTexture");
                if (dt.Has("index")) textureIdx = dt.Get("index").GetNumberAsInt();
            }
        } else {
            const auto& pbr = mat.pbrMetallicRoughness;
            for (size_t i = 0; i < 4 && i < pbr.baseColorFactor.size(); ++i)
                out.baseColorFactor[i] = static_cast<float>(pbr.baseColorFactor[i]);
            textureIdx = pbr.baseColorTexture.index;
        }
    }

    if (textureIdx >= 0 && textureIdx < static_cast<int>(m.textures.size())) {
        int imgIdx = m.textures[textureIdx].source;
        if (imgIdx >= 0 && imgIdx < static_cast<int>(m.images.size())) {
            const auto& img = m.images[imgIdx];
            if (img.component == 4 && img.bits == 8 && !img.image.empty()) {
                out.width  = static_cast<uint32_t>(img.width);
                out.height = static_cast<uint32_t>(img.height);
                out.pixels = img.image;
            }
        }
    }

    if (out.pixels.empty()) {
        out.width  = 1;
        out.height = 1;
        out.pixels.resize(4);
        for (int i = 0; i < 4; ++i) {
            float v = glm::clamp(out.baseColorFactor[i], 0.0f, 1.0f);
            out.pixels[i] = static_cast<unsigned char>(v * 255.0f);
        }
    }
    return out;
}

void traverse(const tinygltf::Model& model, int nodeIdx,
               const glm::mat4& parent, LoadedModel& out) {
    const auto& node = model.nodes[nodeIdx];
    glm::mat4 world = parent * nodeTransform(node);
    if (node.mesh >= 0) {
        for (const auto& prim : model.meshes[node.mesh].primitives) {
            auto data = buildMesh(model, prim);
            if (!data.vertices.empty()) {
                out.nodes.push_back({std::move(data), world,
                                     extractMaterial(model, prim.material)});
            }
        }
    }
    for (int child : node.children) traverse(model, child, world, out);
}

} // namespace

LoadedModel GltfLoader::load(const std::filesystem::path& path) {
    tinygltf::TinyGLTF loader;
    tinygltf::Model   model;
    std::string err, warn;

    const bool isGlb = path.extension() == ".glb";
    const bool ok = isGlb
        ? loader.LoadBinaryFromFile(&model, &err, &warn, path.string())
        : loader.LoadASCIIFromFile(&model, &err, &warn, path.string());

    if (!warn.empty()) qCWarning(L::gltfLoader) << "warn:" << warn.c_str();
    if (!err.empty())  qCWarning(L::gltfLoader) << "err:"  << err.c_str();
    if (!ok) {
        qCCritical(L::gltfLoader) << "Failed to load" << path.string().c_str();
        return {};
    }

    LoadedModel out;
    const int sceneIdx = model.defaultScene >= 0 ? model.defaultScene : 0;
    if (sceneIdx >= 0 && sceneIdx < static_cast<int>(model.scenes.size())) {
        for (int n : model.scenes[sceneIdx].nodes) traverse(model, n, glm::mat4(1.0f), out);
    } else {
        for (size_t n = 0; n < model.nodes.size(); ++n)
            traverse(model, static_cast<int>(n), glm::mat4(1.0f), out);
    }
    qCInfo(L::gltfLoader) << "Loaded" << path.string().c_str()
                          << "nodes:" << out.nodes.size();
    return out;
}

} // namespace nuff::renderer
