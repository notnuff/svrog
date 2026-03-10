#include "colored_logger.h"

namespace nuff::logging {

void installColoredLogger()
{
    qSetMessagePattern(
        "%{if-debug}\033[1;34m%{endif}"
        "%{if-info}\033[1;32m%{endif}"
        "%{if-warning}\033[1;33m%{endif}"
        "%{if-critical}\033[1;31m%{endif}"
        "%{if-category}%{category}: %{endif}"
        "[%{type}] %{message}"
        "\033[0m"
    );
}

} // namespace nuff::logging

