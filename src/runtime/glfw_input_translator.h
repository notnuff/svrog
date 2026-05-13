#pragma once

#include "engine/input/input_codes.h"

namespace nuff::runtime::input {

engine::input::KeyCode     translateKey(int glfwKey);
engine::input::MouseButton translateMouseButton(int glfwButton);
engine::input::KeyAction   translateAction(int glfwAction);
engine::input::KeyMod      translateModifiers(int glfwMods);

} // namespace nuff::runtime::input
