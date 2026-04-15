#include "core_initializer.h"
#include <QLoggingCategory>

#include "core_builders.h"
#include "core/context/ctx.h"

namespace L {
Q_LOGGING_CATEGORY(vkInitializer, "nuff.renderer.vk.initializer")
}

namespace nuff::renderer {

CoreInitializer::CoreInitializer() = default;

CoreInitializer& CoreInitializer::setVertexShaderPath(const std::string& path) {
    m_vertexShaderPath = path;
    return *this;
}

CoreInitializer& CoreInitializer::setFragmentShaderPath(const std::string& path) {
    m_fragmentShaderPath = path;
    return *this;
}

void CoreInitializer::configureCtx(CoreCtx& /*ctx*/) {
    // Default: no extra context configuration
}

std::unique_ptr<CoreCtx> CoreInitializer::buildCtx() {
    auto ctx = std::make_unique<CoreCtx>();
    if (!m_buildersPrepared) {
        prepareBuilders();
        m_buildersPrepared = true;
    }

    configureCtx(*ctx);

    qCInfo(L::vkInitializer) << "Starting Vulkan initialization...";

    for (const auto& builder : m_builders) {
        try {
            builder->build(*ctx);
        } catch (const vk::SystemError& err) {
            qCCritical(L::vkInitializer) << "Vulkan error: " << err.what();
            return nullptr;
        } catch (const std::exception& err) {
            qCCritical(L::vkInitializer)  << "Error: " << err.what();
            return nullptr;
        }
    }


    qCInfo(L::vkInitializer) << "Vulkan initialization complete!";

    return ctx;
}

void CoreInitializer::prepareBuilders() {
    m_builders.insert_back<InstanceBuilder>();
    m_builders.insert_back<DeviceBuilder>();

    {
        auto& builder = m_builders.insert_back<PipelineBuilder>().as<PipelineBuilder>();
        builder.setVertexShaderPath(m_vertexShaderPath);
        builder.setFragmentShaderPath(m_fragmentShaderPath);
    }

    m_builders.insert_back<CommandBuilder>();
    m_builders.insert_back<SyncBuilder>();
}
} // namespace nuff::renderer

