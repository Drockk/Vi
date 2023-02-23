#pragma once

#include "Vi/Core/KeyCodes.hpp"
#include "Vi/Core/MouseCodes.hpp"

#include <glm/glm.hpp>

namespace Vi {
    class Input {
    public:
        static bool isKeyPressed(KeyCode key);

        static bool isMouseButtonPressed(MouseCode button);
        static glm::vec2 getMousePosition();
        static float getMouseX();
        static float getMouseY();
    };
}
