#include "Motor.h"
#include "Definiciones.h"

namespace {
void setLedsMovimiento(bool txOn, bool rxOn) {
#if defined(TXLED0) && defined(TXLED1) && defined(RXLED0) && defined(RXLED1)
    if (txOn) {
        TXLED0;
    } else {
        TXLED1;
    }

    if (rxOn) {
        RXLED0;
    } else {
        RXLED1;
    }
#else
    static bool ledInicializado = false;
    if (!ledInicializado) {
        pinMode(LED_BUILTIN, OUTPUT);
        ledInicializado = true;
    }

    // En placas con un solo LED (Nano), usa patrones para distinguir estados.
    if (!txOn && !rxOn) {
        digitalWrite(LED_BUILTIN, LOW);
        return;
    }

    const unsigned long ahora = millis();
    if (txOn && rxOn) {
        // Giro: parpadeo rapido
        digitalWrite(LED_BUILTIN, ((ahora / 120) % 2) ? HIGH : LOW);
    } else if (txOn) {
        // Avance: fijo encendido
        digitalWrite(LED_BUILTIN, HIGH);
    } else {
        // Retroceso/curva izquierda: parpadeo lento
        digitalWrite(LED_BUILTIN, ((ahora / 260) % 2) ? HIGH : LOW);
    }
#endif
}
} // namespace

Motor::Motor(int a1, int a2, int pwm, bool invertir)
    : pinA1(a1), pinA2(a2), pinPWM(pwm), invertido(invertir), pwmActual(0), sentidoActual(0), ultimoPasoRampaMs(0) {}

void Motor::aplicarPWMsuave(int velocidadObjetivo) {
    velocidadObjetivo = constrain(velocidadObjetivo, 0, 255);
    const int pasoSubida = 5;
    const int pasoBajada = 14;
    const int pwmMinArranque = 58;
    const int pwmImpulsoInicial = 64;
    const unsigned long intervaloRampaMs = 5;

    if (millis() - ultimoPasoRampaMs < intervaloRampaMs) {
        analogWrite(pinPWM, pwmActual);
        return;
    }
    ultimoPasoRampaMs = millis();

    if (velocidadObjetivo > 0 && velocidadObjetivo < pwmMinArranque) {
        velocidadObjetivo = pwmMinArranque;
    }

    // Evita micro-paradas: al salir de reposo da un impulso breve controlado.
    if (pwmActual == 0 && velocidadObjetivo > 0) {
        pwmActual = min(velocidadObjetivo, pwmImpulsoInicial);
        analogWrite(pinPWM, pwmActual);
        return;
    }

    if (velocidadObjetivo > pwmActual) {
        pwmActual += pasoSubida;
        if (pwmActual > velocidadObjetivo) {
            pwmActual = velocidadObjetivo;
        }
    } else if (velocidadObjetivo < pwmActual) {
        pwmActual -= pasoBajada;
        if (pwmActual < velocidadObjetivo) {
            pwmActual = velocidadObjetivo;
        }
    }

    analogWrite(pinPWM, pwmActual);
}

void Motor::avanzar(int velocidad) {
    const uint8_t pin1 = invertido ? HIGH : LOW;
    const uint8_t pin2 = invertido ? LOW : HIGH;
    digitalWrite(pinA1, pin1);
    digitalWrite(pinA2, pin2);
    sentidoActual = 1;
    aplicarPWMsuave(velocidad);
}

void Motor::retroceder(int velocidad) {
    // El retroceso de escape debe ser inmediato, sin rampa.
    velocidad = constrain(velocidad, 0, 255);
    const uint8_t pin1 = invertido ? LOW : HIGH;
    const uint8_t pin2 = invertido ? HIGH : LOW;
    digitalWrite(pinA1, pin1);
    digitalWrite(pinA2, pin2);
    sentidoActual = -1;
    pwmActual = velocidad;
    analogWrite(pinPWM, velocidad);
}

void Motor::detener() {
    digitalWrite(pinA1, LOW);
    digitalWrite(pinA2, LOW);
    sentidoActual = 0;
    pwmActual = 0;
    analogWrite(pinPWM, 0);
}

// Implementación de Motores
Motores::Motores() :
    motorIzq(MA1A, MA2A, PWMA, INVERTIR_MOTOR_IZQUIERDO),
    motorDer(MA1B, MA2B, PWMB, INVERTIR_MOTOR_DERECHO) {}

void Motores::adelante(int velocidad) {
    motorIzq.avanzar(velocidad);
    motorDer.avanzar(velocidad);
    setLedsMovimiento(true, false);
}

void Motores::retroceder(int velocidad) {
    motorIzq.retroceder(velocidad);
    motorDer.retroceder(velocidad);
    setLedsMovimiento(false, true);
}

void Motores::detener() {
    motorIzq.detener();
    motorDer.detener();
    setLedsMovimiento(false, false);
}

void Motores::derecha(int velocidad) {
    // Giro con pivote: rueda derecha detenida.
    motorIzq.avanzar(velocidad);
    motorDer.detener();
    setLedsMovimiento(true, false);
}

void Motores::izquierda(int velocidad) {
    // Giro con pivote: rueda izquierda detenida.
    motorIzq.detener();
    motorDer.avanzar(velocidad);
    setLedsMovimiento(false, true);
}

void Motores::curvaDerecha(int velocidad) {
    const int velocidadInterna = max(45, velocidad / 2);
    motorIzq.avanzar(velocidad);
    motorDer.avanzar(velocidadInterna);
    setLedsMovimiento(true, false);
}

void Motores::curvaIzquierda(int velocidad) {
    const int velocidadInterna = max(45, velocidad / 2);
    motorIzq.avanzar(velocidadInterna);
    motorDer.avanzar(velocidad);
    setLedsMovimiento(false, true);
}

Motor& Motores::getIzquierdo() {
    return motorIzq;
}

Motor& Motores::getDerecho() {
    return motorDer;
}
