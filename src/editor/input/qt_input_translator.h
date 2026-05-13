#pragma once

#include <Qt>

#include "engine/input/input_codes.h"

namespace nuff::editor::input {

engine::input::KeyCode     translateKey(int qtKey);
engine::input::MouseButton translateMouseButton(Qt::MouseButton qtButton);
engine::input::KeyMod      translateModifiers(Qt::KeyboardModifiers qtMods);

} // namespace nuff::editor::input
