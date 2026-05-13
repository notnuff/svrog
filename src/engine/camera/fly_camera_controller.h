#pragma once

#include <QtCore/qtclasshelpermacros.h>

#include <array>
#include <cstddef>

#include "../components/i_component.h"
#include "../events/i_event_listener.h"
#include "../input/input_codes.h"

namespace nuff::engine::input {
class InputSystem;
}

namespace nuff::engine {

class FlyCameraController : public IComponent, public events::IEventListener {
public:
    FlyCameraController() = default;
    ~FlyCameraController() override = default;

    Q_DISABLE_COPY(FlyCameraController)

    void init() override;
    void update(float deltaTime) override;
    void render() override;

    void onEvent(const events::IEvent& event) override;

    void bindInput(input::InputSystem& input);
    void unbindInput();

    void  setMoveSpeed(float unitsPerSecond);
    float moveSpeed() const;

    void  setLookSensitivity(float radiansPerPixel);
    float lookSensitivity() const;

    void  setBoostMultiplier(float multiplier);
    float boostMultiplier() const;

private:
    std::array<bool, static_cast<std::size_t>(input::KeyCode::Count)> m_keyDown{};
    bool   m_leftMouseDown = false;
    double m_accumDx = 0.0;
    double m_accumDy = 0.0;

    float m_moveSpeed        = 5.0f;
    float m_lookSensitivity  = 0.003f;
    float m_boostMultiplier  = 4.0f;
};

} // namespace nuff::engine
