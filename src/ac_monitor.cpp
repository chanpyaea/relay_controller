#include "ac_monitor.h"

#include "config.h"
#include "mqtt_manager.h"

namespace
{
volatile uint32_t pulseCount = 0;
portMUX_TYPE pulseMux = portMUX_INITIALIZER_UNLOCKED;
unsigned long lastCheck = 0;
bool electricityOn = false;
bool firstReading = true;
uint8_t pendingChecks = 0;

void IRAM_ATTR pulseISR()
{
    portENTER_CRITICAL_ISR(&pulseMux);
    pulseCount++;
    portEXIT_CRITICAL_ISR(&pulseMux);
}
}

void acMonitorInit()
{
    // H_PF is sampled on falling edges. The internal pull-up prevents GPIO 25
    // from floating when the detector is inactive or disconnected.
    pinMode(AC_SENSE_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(AC_SENSE_PIN), pulseISR, FALLING);
    lastCheck = millis();
    Serial.printf("AC monitor started on GPIO %u\n", AC_SENSE_PIN);
}

void acMonitorLoop()
{
    const unsigned long now = millis();
    if(now - lastCheck < AC_CHECK_INTERVAL_MS)
    {
        return;
    }

    portENTER_CRITICAL(&pulseMux);
    const uint32_t pulses = pulseCount;
    pulseCount = 0;
    portEXIT_CRITICAL(&pulseMux);

    const bool detectedOn = pulses >= AC_MIN_PULSES_PER_CHECK;

    // Publish a known initial OFF state, then require multiple consecutive
    // samples before accepting either transition. An isolated EMI/static edge
    // can therefore never report that mains power is present.
    if(firstReading)
    {
        firstReading = false;
        mqttPublishElectricityState(electricityOn);
    }

    if(detectedOn == electricityOn)
    {
        pendingChecks = 0;
    }
    else
    {
        pendingChecks++;
        const uint8_t requiredChecks = detectedOn
            ? AC_ON_CONFIRM_CHECKS
            : AC_OFF_CONFIRM_CHECKS;

        if(pendingChecks >= requiredChecks)
        {
            electricityOn = detectedOn;
            pendingChecks = 0;
            Serial.printf("Electricity: %s (%lu pulses)\n",
                          electricityOn ? "ON" : "OFF",
                          static_cast<unsigned long>(pulses));
            mqttPublishElectricityState(electricityOn);
        }
    }

    lastCheck = now;
}

bool isElectricityOn()
{
    return electricityOn;
}
