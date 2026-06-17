#pragma once

#include <Arduino.h>

class LocalControls
{
public:
    void begin();
    void update();

    bool consumePumpToggleRequest();
    bool consumeCobToggleRequest();

private:
    struct ButtonState
    {
        gpio_num_t pin;
        bool lastRawPressed = false;
        bool debouncedPressed = false;
        unsigned long lastRawChangedAt = 0;
        bool toggleRequested = false;
    };

    ButtonState pumpButton{};
    ButtonState cobButton{};

    void initializeButton(ButtonState &button, gpio_num_t pin);
    void updateButton(ButtonState &button);
    bool readPressed(gpio_num_t pin) const;
};
