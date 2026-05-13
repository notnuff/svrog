#pragma once

#include <cstdint>

#include "i_event.h"

namespace nuff::engine::events {

struct WindowResizedEvent : public Event<WindowResizedEvent> {
private:
    std::uint32_t m_width;
    std::uint32_t m_height;

public:
    WindowResizedEvent(const std::uint32_t width, const std::uint32_t height)
        : m_width(width), m_height(height) {}

    std::uint32_t width() const { return m_width; }
    std::uint32_t height() const { return m_height; }
};


} // namespace nuff::engine

