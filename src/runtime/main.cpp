#include "runtime_app.h"

#include <QLoggingCategory>

#include <cstdlib>
#include <stdexcept>

namespace L {
Q_LOGGING_CATEGORY(runtimeMain, "nuff.runtime.main")
}

int main() {
    nuff::runtime::RuntimeApp app;

    try {
        app.run();
    } catch (const std::exception& e) {
        qCCritical(L::runtimeMain) << "Error:" << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

