#pragma once

#include "../components/camera_component.h"
#include "../components/mesh_component.h"
#include "../components/transform_component.h"
#include "../entities/entity.h"
#include "../scene/scene.h"

#include "primitives/frame_view.h"

namespace nuff::engine {

class RenderSystem {
public:
    static renderer::FrameView buildFrameView(const Scene& scene, float totalTime) {
        renderer::FrameView frame;
        frame.time = totalTime;

        if (Entity* camera = scene.activeCamera()) {
            if (auto* cc = camera->getComponent<CameraComponent>()) {
                frame.view = cc->viewMatrix();
                frame.proj = cc->projectionMatrix();
            }
        }

        for (const auto& entity : scene.entities()) {
            if (!entity->isActive()) continue;
            auto* mc = entity->getComponent<MeshComponent>();
            if (!mc || !mc->mesh()) continue;
            auto* xf = entity->getComponent<TransformComponent>();
            const glm::mat4 model = xf ? xf->worldMatrix() : glm::mat4(1.0f);
            frame.items.push_back(renderer::DrawItem{
                .mesh = mc->mesh(),
                .materialSet = mc->materialSet(),
                .model = model
            });
        }

        return frame;
    }
};

} // namespace nuff::engine
