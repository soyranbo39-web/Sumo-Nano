#include "SensorPiso.h"

SensorPiso::SensorPiso(int pin, int umbral) : pin(pin), umbral(umbral) {}

bool SensorPiso::detectar() {
    int lectura = analogRead(pin);
    lectura += analogRead(pin);
    lectura += analogRead(pin);
    lectura /= 3;
    return lectura <= umbral;
}
