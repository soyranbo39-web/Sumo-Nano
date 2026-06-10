#include "SensorEnemigo.h"

SensorEnemigo::SensorEnemigo(int pin) : pin(pin) {}

bool SensorEnemigo::detectar() {
    return digitalRead(pin) == HIGH;
}
