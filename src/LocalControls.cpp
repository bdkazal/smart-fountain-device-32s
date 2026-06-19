#include "LocalControls.h"

#include "HardwarePins.h"

void LocalControls::begin()
{
    initializeButton(pumpButton, HardwarePins::PumpButton);
    initializeButton(cobButton, HardwarePins::CobButton);

    Serial.print("Pump local button initialized on GPIO");
    Serial.println(static_cast<int>(HardwarePins::PumpButton));
    Serial.print("COB local button initialized on GPIO");
    Serial.println(static_cast<int>(HardwarePins::CobButton));
}

void LocalControls::update()
{
    updateButton(pumpButton);
    updateButton(cobButton);
}

bool LocalControls::consumePumpToggleRequest()
{
    const bool requested = pumpButton.toggleRequested;
    pumpButton.toggleRequested = false;
    return requested;
}

bool LocalControls::consumeCobToggleRequest()
{
    const bool requested = cobButton.toggleRequested;
    cobButton.toggleRequested = false;
    return requested;
}

void LocalControls::initializeButton(ButtonState &button, gpio_num_t pin)
{
    button.pin = pin;
    pinMode(pin, INPUT_PULLUP);

    button.lastRawPressed = readPressed(pin);
    button.debouncedPressed = button.lastRawPressed;
    button.lastRawChangedAt = millis();
    button.toggleRequested = false;
}

void LocalControls::updateButton(ButtonState &button)
{
    const bool rawPressed = readPressed(button.pin);
    const unsigned long now = millis();

    if (rawPressed != button.lastRawPressed)
    {
        button.lastRawPressed = rawPressed;
        button.lastRawChangedAt = now;
    }

    if (button.debouncedPressed != rawPressed &&
        now - button.lastRawChangedAt >= HardwarePins::LocalButtonDebounceMs)
    {
        button.debouncedPressed = rawPressed;

        if (button.debouncedPressed)
        {
            button.toggleRequested = true;
        }
    }
}

bool LocalControls::readPressed(gpio_num_t pin) const
{
    const int state = digitalRead(pin);
    return HardwarePins::LocalButtonsActiveLow ? state == LOW : state == HIGH;
}
