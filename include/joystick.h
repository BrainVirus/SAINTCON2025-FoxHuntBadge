#pragma once

#include <Arduino.h>
#include "config.h"

enum JoystickEvent {
    JOY_NONE = 0,
    JOY_UP,
    JOY_DOWN,
    JOY_LEFT,
    JOY_RIGHT,
    JOY_CENTER_CLICK,
    JOY_CENTER_HOLD
};

class JoystickManager {
public:
    JoystickManager();
    void begin();
    JoystickEvent update();

private:
    struct ButtonState {
        uint8_t pin;
        bool lastState;
        bool isPressed;
        unsigned long pressTime;
        unsigned long lastRepeatTime;
        bool holdFired;
    };

    ButtonState _btnUp;
    ButtonState _btnDown;
    ButtonState _btnLeft;
    ButtonState _btnRight;
    ButtonState _btnCenter;

    bool checkButton(ButtonState &btn, bool isCenter, JoystickEvent &event);
};

extern JoystickManager Joystick;
