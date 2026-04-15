#pragma once

#include "core_initializer.h"
#include "core/builders/debug_instance_builder.h"

namespace nuff::renderer {

class DebugInitializer : public CoreInitializer {
protected:
    void prepareBuilders() override {
        CoreInitializer::prepareBuilders();
        if (auto link = this->m_builders.get<InstanceBuilder>(); link.has_value()) {
            // Configure extensions/layers BEFORE instance creation
            link->insert_before<DebugInstanceBuilder>();
            // Set up debug messenger AFTER instance creation
            link->insert_after<DebugMessengerBuilder>();
        }
    }
};

} // namespace nuff::renderer
