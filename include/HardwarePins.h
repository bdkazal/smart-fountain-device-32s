#pragma once

#include <Arduino.h>

// ESP32-WROOM-32 / generic 38-pin development board.
// These assignments are provisional until validated on the target hardware.

namespace HardwarePins
{
constexpr gpio_num_t PumpButton = GPIO_NUM_18;
constexpr gpio_num_t CobButton = GPIO_NUM_19;
constexpr gpio_num_t PumpOutput = GPIO_NUM_25;
constexpr gpio_num_t CobOutput = GPIO_NUM_26;
constexpr gpio_num_t NeoPixelData = GPIO_NUM_27;
constexpr gpio_num_t WaterLevelSwitch = GPIO_NUM_32;
constexpr gpio_num_t WifiResetButton = GPIO_NUM_33;

constexpr bool PumpActiveHigh = true;
constexpr bool CobActiveHigh = true;
constexpr bool WaterLevelActiveLow = true;
constexpr bool LocalButtonsActiveLow = true;

constexpr uint16_t NeoPixelCount = 30;
constexpr uint8_t NeoPixelMaxBrightness = 160;
constexpr unsigned long WaterLevelDebounceMs = 250;
constexpr unsigned long LocalButtonDebounceMs = 50;
}
