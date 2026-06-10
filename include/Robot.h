#ifndef ROBOT_H
#define ROBOT_H

#include <Arduino.h>
#include "Definiciones.h"
#include "Motor.h"
#include "SensorPiso.h"
#include "SensorEnemigo.h"

// Prioridades de acción
enum class Prioridad {
    ESCAPE_BORDE = 0,      // Máxima: escape del borde
    ATAQUE_FRONTAL = 1,    // Alta: enemigo al frente
    GIRO_LATERAL = 2,      // Media: enemigo en lateral
    BUSQUEDA = 3           // Baja: sin enemigo
};

class Robot {
        bool bordeDetectado; // Estado de detección de borde
    int suavePisoIzq = 240, suavePisoDer = 240; // Valores suavizados (EMA)
    static constexpr float factorEMA = 0.3f; // Factor de promedio exponencial
    Motores motores;
    SensorPiso sensorPisoIzq, sensorPisoDer;
    SensorEnemigo sensorFrontal, sensorFrontalIzq, sensorFrontalDer, sensorLateralIzq, sensorLateralDer;
    bool leerPiso(bool &pisoIzq, bool &pisoDer);
    void aplicarEMAsuavizado(int &suaveActual, int valorNuevo);
    bool esperarConPrioridadPiso(unsigned long duracionMs);
    void retrocesoSeguro(unsigned long duracionMs);
    void giroEscapeSeguro(bool haciaDerecha, unsigned long duracionMs);
    void giroEscapeCompleto(bool haciaDerecha, unsigned long duracionMs, int velocidadCurva);
public:
    Robot();
    void setup();
    void detenerse();
    void ataqueEnemigo();
    void moverAdelante();
    void retroceder();
    void moverDerecha();
    void moverIzquierda();
    void sensoresPiso(bool pisoIzq, bool pisoDer);
    void sensoresFrontales(bool central, bool derecho, bool izquierdo);
    void sensoresLaterales(bool sensorIzquierdo, bool sensorDerecho);
    void loop();
};

#endif // ROBOT_H
