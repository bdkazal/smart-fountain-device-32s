#include "HardwareValidator.h"

#include "HardwareOutputs.h"

void HardwareValidator::begin(HardwareOutputs &outputs)
{
    outputs.setPumpEnabled(false);
    outputs.setCobEnabled(false);
    outputs.setNeoPixelsEnabled(false);

    Serial.println();
    Serial.println("Interactive hardware console ready.");
    printHelp();
    printOutputState(outputs);
}

void HardwareValidator::update(HardwareOutputs &outputs)
{
    handleSerialCommands(outputs);
}

void HardwareValidator::handleSerialCommands(HardwareOutputs &outputs)
{
    while (Serial.available() > 0)
    {
        const char command = static_cast<char>(Serial.read());

        switch (command)
        {
        case 'p':
        case 'P':
            outputs.setPumpEnabled(!outputs.isPumpEnabled());
            Serial.print("Pump test output GPIO25: ");
            Serial.println(outputs.isPumpEnabled() ? "ON" : "OFF");
            break;

        case 'c':
        case 'C':
            outputs.setCobEnabled(!outputs.isCobEnabled());
            Serial.print("COB test output GPIO26: ");
            Serial.println(outputs.isCobEnabled() ? "ON" : "OFF");
            break;

        case 'a':
        case 'A':
            outputs.setPumpEnabled(true);
            outputs.setCobEnabled(true);
            Serial.println("Pump and COB test outputs: ON");
            break;

        case 'x':
        case 'X':
            outputs.setPumpEnabled(false);
            outputs.setCobEnabled(false);
            outputs.setNeoPixelsEnabled(false);
            Serial.println("All test outputs: OFF");
            break;

        case 'r':
        case 'R':
            setNeoPixelColor(outputs, ColorTestLevel, 0, 0, "RED");
            break;

        case 'g':
        case 'G':
            setNeoPixelColor(outputs, 0, ColorTestLevel, 0, "GREEN");
            break;

        case 'b':
        case 'B':
            setNeoPixelColor(outputs, 0, 0, ColorTestLevel, "BLUE");
            break;

        case 'w':
        case 'W':
            setNeoPixelColor(outputs, WhiteTestLevel, WhiteTestLevel, WhiteTestLevel, "WHITE");
            break;

        case 'n':
        case 'N':
            outputs.setNeoPixelsEnabled(false);
            Serial.println("NeoPixels: OFF");
            break;

        case 's':
        case 'S':
            printOutputState(outputs);
            break;

        case 'h':
        case 'H':
        case '?':
            printHelp();
            break;

        default:
            break;
        }
    }
}

void HardwareValidator::printHelp() const
{
    Serial.println("Serial commands:");
    Serial.println("  p = toggle pump test LED on GPIO25");
    Serial.println("  c = toggle COB test LED on GPIO26");
    Serial.println("  a = turn pump and COB test LEDs ON");
    Serial.println("  x = turn all outputs OFF");
    Serial.println("  r = NeoPixel red");
    Serial.println("  g = NeoPixel green");
    Serial.println("  b = NeoPixel blue");
    Serial.println("  w = NeoPixel balanced white");
    Serial.println("  n = NeoPixel OFF");
    Serial.println("  s = print current output state");
    Serial.println("  h or ? = print this help");
}

void HardwareValidator::printOutputState(const HardwareOutputs &outputs) const
{
    Serial.println("Current output state:");
    Serial.print(" - pump GPIO25: ");
    Serial.println(outputs.isPumpEnabled() ? "ON" : "OFF");
    Serial.print(" - COB GPIO26: ");
    Serial.println(outputs.isCobEnabled() ? "ON" : "OFF");
    Serial.print(" - NeoPixels GPIO27: ");
    Serial.println(outputs.areNeoPixelsEnabled() ? "ON" : "OFF");
}

void HardwareValidator::setNeoPixelColor(
    HardwareOutputs &outputs,
    uint8_t red,
    uint8_t green,
    uint8_t blue,
    const char *name)
{
    outputs.setNeoPixelColor(red, green, blue);
    outputs.setNeoPixelsEnabled(true);

    Serial.print("NeoPixels: ");
    Serial.println(name);
}
