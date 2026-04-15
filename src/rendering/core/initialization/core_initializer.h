#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "core_builders.h"
#include "core/builders/builder_chain.h"
#include "core/context/ctx.h"

namespace nuff::renderer {

class CoreInitializer {
public:
    CoreInitializer();
    virtual ~CoreInitializer() = default;

    CoreInitializer& setVertexShaderPath(const std::string& path);
    CoreInitializer& setFragmentShaderPath(const std::string& path);

    std::unique_ptr<CoreCtx> buildCtx();

protected:
    virtual void prepareBuilders();
    virtual void configureCtx(CoreCtx& ctx);

    BuilderChain m_builders;

private:
    std::string m_vertexShaderPath;
    std::string m_fragmentShaderPath;
    bool m_buildersPrepared = false;

};

} // namespace nuff::renderer

