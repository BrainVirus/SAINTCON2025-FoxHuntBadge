#include "joystick.h"

JoystickManager Joystick;

JoystickManager::JoystickManager() {
    _btnUp = { JOY_UP_PIN, HIGH, false, 0, 0, false };
    _btnDown = { JOY_DOWN_PIN, HIGH, false, 0, 0, false };
    _btnLeft = { JOY_LEFT_PIN, HIGH, false, 0, 0, false };
    _btnRight = { JOY_RIGHT_PIN, HIGH, false, 0, 0, false };
    _btnCenter = { JOY_CENTER_PIN, HIGH, false, 0, 0, false };
}

void JoystickManager::begin() {
    pinMode(JOY_UP_PIN, INPUT_PULLUP);
    pinMode(JOY_DOWN_PIN, INPUT_PULLUP);
    pinMode(JOY_LEFT_PIN, INPUT_PULLUP);
    pinMode(JOY_RIGHT_PIN, INPUT_PULLUP);
    pinMode(JOY_CENTER_PIN, INPUT_PULLUP);
}

bool JoystickManager::checkButton(ButtonState &btn, bool isCenter, JoystickEvent &event) {
    bool raw = digitalRead(btn.pin);
    bool pressed = (raw == LOW);
    unsigned long now = millis();

    if (pressed && !btn.isPressed) {
        // Initial press edge
        btn.isPressed = true;
        btn.pressTime = now;
        btn.lastRepeatTime = now;
        btn.holdFired = false;

        if (!isCenter) {
            // Directional buttons fire immediately on press
            return true;
        }
    } else if (pressed && btn.isPressed) {
        // Button held
        if (isCenter) {
            if (!btn.holdFired && (now - btn.pressTime > 550)) {
                btn.holdFired = true;
                event = JOY_CENTER_HOLD;
                return true;
            }
        } else {
            // Auto-repeat for directional buttons: delay 350ms, then every 90ms
            if (now - btn.pressTime > 350 && (now - btn.lastRepeatTime > 90)) {
                btn.lastRepeatTime = now;
                return true;
            }
        }
    } else if (!pressed && btn.isPressed) {
        // Release edge
        btn.isPressed = false;
        if (isCenter) {
            if (!btn.holdFired && (now - btn.pressTime > 25)) {
                event = JOY_CENTER_CLICK;
                return true;
            }
        }
    }

    return false;
}

JoystickEvent JoystickManager::update() {
    JoystickEvent ev = JOY_NONE;

    if (checkButton(_btnCenter, true, ev)) {
        return ev;
    }
    if (checkButton(_btnUp, false, ev)) {
        return JOY_UP;
    }
    if (checkButton(_btnDown, false, ev)) {
        return JOY_DOWN;
    }
    if (checkButton(_btnLeft, false, ev)) {
        return JOY_LEFT;
    }
    if (checkButton(_btnRight, false, ev)) {
        return JOY_RIGHT;
    }

    return JOY_NONE;
}
