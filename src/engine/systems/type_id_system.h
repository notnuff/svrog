#pragma once

#include <cstddef>

namespace nuff::engine {

// there is absolutely no need for this typesystem, but it looks fun
class TypeIDSystem {
private:
    static size_t m_nextTypeID;

public:
    template<typename T>
    static size_t getTypeID() {
        static size_t typeID = m_nextTypeID++;
        return typeID;
    }
};

} // namespace nuff::engine
