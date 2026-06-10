
    // Variables estáticas para la rutina de búsqueda
    static bool busquedaDerecha = true;
    static unsigned char faseBusqueda = 0;
    static unsigned long inicioFase = 0;
#include <Arduino.h>
#include <avr/wdt.h>
#include "Robot.h"
#include "Definiciones.h"

// ------------------ Control remoto (nivel) ------------------
bool robot_encendido          = false;

Robot::Robot() :
    motores(),
    sensorPisoIzq(SENSOR_DE_PISO_IZQUIERDO, BLANCO),
    sensorPisoDer(SENSOR_DE_PISO_DERECHO, BLANCO),
    sensorFrontal(SENSOR_FRONTAL_CENTRA),
    sensorFrontalIzq(SENSOR_FRONTAL_IZQUIERDO),
    sensorFrontalDer(SENSOR_FRONTAL_DERECHO),
    sensorLateralIzq(SENSOR_LATERAL_IZQUIERDO),
    sensorLateralDer(SENSOR_LATERAL_DERECHO),
    bordeDetectado(false)
{}

bool Robot::leerPiso(bool &pisoIzq, bool &pisoDer) {
    pisoIzq = sensorPisoIzq.detectar();
    pisoDer = sensorPisoDer.detectar();
    return pisoIzq || pisoDer;
}

bool Robot::esperarConPrioridadPiso(unsigned long duracionMs) {
    unsigned long inicio = millis();
    while (millis() - inicio < duracionMs) {
        bool pisoIzq = false;
        bool pisoDer = false;
        if (leerPiso(pisoIzq, pisoDer)) {
            
            return true;
        }
        delay(5);
    }
    return false;
}

void Robot::retrocesoSeguro(unsigned long duracionMs) {
    unsigned long inicio = millis();
    unsigned long pistaEstableDesde = 0;
    const unsigned long retrocesoMinimoMs = 40;
    while (millis() - inicio < duracionMs) {
        retroceder();

        bool pisoIzq = false;
        bool pisoDer = false;
        bool enBorde = leerPiso(pisoIzq, pisoDer);

        if (!enBorde) {
            if (pistaEstableDesde == 0) {
                pistaEstableDesde = millis();
            }
            if ((millis() - inicio) > retrocesoMinimoMs && (millis() - pistaEstableDesde) > 25) {
                return;
            }
        } else {
            pistaEstableDesde = 0;
        }
    }
}

void Robot::giroEscapeSeguro(bool haciaDerecha, unsigned long duracionMs) {
    unsigned long inicio = millis();
    unsigned long pistaEstableDesde = 0;
    while (millis() - inicio < duracionMs) {
        bool pisoIzq = false;
        bool pisoDer = false;
        bool enBorde = leerPiso(pisoIzq, pisoDer);

        if (haciaDerecha) {
            moverDerecha();
        } else {
            moverIzquierda();
        }

        // Requiere una pequeña ventana estable fuera del borde para terminar el giro.
        if (!enBorde) {
            if (pistaEstableDesde == 0) {
                pistaEstableDesde = millis();
            }
            if ((millis() - inicio) > 40 && (millis() - pistaEstableDesde) > 30) {
                return;
            }
        } else {
            pistaEstableDesde = 0;
        }
    }
}

void Robot::giroEscapeCompleto(bool haciaDerecha, unsigned long duracionMs) {
    unsigned long inicio = millis();
    unsigned long pistaEstableDesde = 0;
    const unsigned long giroMinimoMs = 80;
    while (millis() - inicio < duracionMs) {
        bool pisoIzq = false;
        bool pisoDer = false;
        bool enBorde = leerPiso(pisoIzq, pisoDer);

        if (haciaDerecha) {
            moverDerecha();
        } else {
            moverIzquierda();
        }

        if (!enBorde) {
            if (pistaEstableDesde == 0) {
                pistaEstableDesde = millis();
            }
            if ((millis() - inicio) > giroMinimoMs && (millis() - pistaEstableDesde) > 45) {
                return;
            }
        } else {
            pistaEstableDesde = 0;
        }
    }
}

void Robot::setup() {
    Serial.begin(9600);
    pinMode(SENSOR_DE_PISO_IZQUIERDO, INPUT);
    pinMode(SENSOR_DE_PISO_DERECHO, INPUT);
    pinMode(SENSOR_FRONTAL_DERECHO, INPUT);
    pinMode(SENSOR_FRONTAL_CENTRA, INPUT);
    pinMode(SENSOR_FRONTAL_IZQUIERDO, INPUT);
    pinMode(SENSOR_LATERAL_IZQUIERDO, INPUT);
    pinMode(SENSOR_LATERAL_DERECHO, INPUT);
    pinMode(MA2A, OUTPUT);
    pinMode(MA1A, OUTPUT);
    pinMode(PWMA, OUTPUT);
    pinMode(MA2B, OUTPUT);
    pinMode(MA1B, OUTPUT);
    pinMode(PWMB, OUTPUT);
    pinMode(Pin_Control_Remoto, INPUT);
}

void Robot::detenerse() {
    motores.detener();
}

void Robot::ataqueEnemigo() {
    motores.adelante(Velocidad_maxima_Ataque);
}

void Robot::moverAdelante() {
    motores.adelante(Velocidad_movimiento_seguir);
}

void Robot::retroceder() {
    motores.retroceder(Velocidad_estandar);
}

void Robot::moverDerecha() {
    motores.derecha(Velocidad_maxima);
}

void Robot::moverIzquierda() {
    motores.izquierda(Velocidad_maxima);
}

void Robot::sensoresPiso(bool pisoIzq, bool pisoDer) {
    static bool giroAlternadoDerecha = true;

    if (pisoIzq && pisoDer) {
        retrocesoSeguro(150);
        giroEscapeSeguro(giroAlternadoDerecha, 120);
        giroAlternadoDerecha = !giroAlternadoDerecha;
    } else if (pisoDer) {
        retrocesoSeguro(180);
        giroEscapeCompleto(false, 180);
    } else if (pisoIzq) {
        retrocesoSeguro(180);
        giroEscapeCompleto(true, 180);
    }
}

void Robot::sensoresFrontales(bool central, bool derecho, bool izquierdo) {
    // Aquí solo llega si es frontal asimétrico (no central), así que ataca hacia ese lado
      
    if (derecho && !izquierdo) {
        moverDerecha();
        if (esperarConPrioridadPiso(70)) return;
        motores.adelante(Velocidad_maxima);
        esperarConPrioridadPiso(70);
    } else if (izquierdo && !derecho) {
        moverIzquierda();
        if (esperarConPrioridadPiso(70)) return;
        motores.adelante(Velocidad_maxima);
        esperarConPrioridadPiso(70);
    }
}

void Robot::sensoresLaterales(bool sensorIzquierdo, bool sensorDerecho) {
    if (sensorIzquierdo && sensorDerecho) return;

    bool pisoIzq = false, pisoDer = false;
    leerPiso(pisoIzq, pisoDer);
    if (pisoIzq || pisoDer) return;

    // Posicionamiento rápido: solo una llanta gira y la otra queda detenida.
    // Verifica piso cada 5ms para abortar si hay borde.
    if (sensorIzquierdo) {
        motores.getIzquierdo().detener();
        motores.getDerecho().avanzar(Velocidad_maxima_Ataque);
        esperarConPrioridadPiso(80);
    } else if (sensorDerecho) {
        motores.getDerecho().detener();
        motores.getIzquierdo().avanzar(Velocidad_maxima_Ataque);
        esperarConPrioridadPiso(80);
    }
}

void Robot::loop() {
    // ------------------ Control remoto ------------------
    int estado_actual = digitalRead(Pin_Control_Remoto);
    bool nivel_activo = REMOTE_ACTIVE_HIGH ? (estado_actual == HIGH) : (estado_actual == LOW);
    robot_encendido = nivel_activo;

    static bool estadoAnterior = false;
    if (!robot_encendido) {
        motores.detener();
        estadoAnterior = false;
        return;
    }
    const bool recienEncendido = !estadoAnterior;
    estadoAnterior = true;

    // LECTURA DE SENSORES
    bool PisoIzq = false, PisoDer = false;
    int valorPisoIzq = analogRead(SENSOR_DE_PISO_IZQUIERDO);
    int valorPisoDer = analogRead(SENSOR_DE_PISO_DERECHO);
    PisoIzq = (valorPisoIzq <= BLANCO);
    PisoDer = (valorPisoDer <= BLANCO);
    
    bool FrontalDer = sensorFrontalDer.detectar();
    bool FrontalIzq = sensorFrontalIzq.detectar();
    bool FrontalCentral = sensorFrontal.detectar();
    bool LateralDer = sensorLateralDer.detectar();
    bool LateralIzq = sensorLateralIzq.detectar();
    const bool enemigoDetectado = FrontalDer || FrontalIzq || FrontalCentral || LateralDer || LateralIzq;


    // --- NUEVA LÓGICA: avanzar hasta detectar borde antes de rutina normal ---

    // El estado ahora es miembro y se inicializa en setup()

    // Si el robot está apagado (botón stop), resetea el estado
    if (!robot_encendido) {
        motores.detener();
        bordeDetectado = false;
        busquedaDerecha = true;
        faseBusqueda = 0;
        inicioFase = 0;
        estadoAnterior = false;
#include <avr/wdt.h>
        wdt_enable(WDTO_15MS); // Habilita el watchdog para reinicio rápido
        while (1) {} // Espera el reinicio
        return;
    }

    if (!bordeDetectado) {
        // Avanza hacia el borde a velocidad reducida
        motores.adelante(Velocidad_borde);
        if (PisoIzq || PisoDer) {
            // Al detectar el borde, retrocede para no salirse
            bordeDetectado = true;
            retrocesoSeguro(200); // Reducido para motores de 700 rpm
            sensoresPiso(PisoIzq, PisoDer);
        }
        return;
    }

    // SISTEMA DE PRIORIDADES
    // Prioridad 0 (MÁXIMA): Escape del borde
    if (PisoIzq || PisoDer) {
        sensoresPiso(PisoIzq, PisoDer);
        return;
    }

    // Prioridad 1 (ALTA): Enemigo frontal
    if (FrontalCentral || (FrontalDer && FrontalIzq)) {
        ataqueEnemigo();
        return;
    }

    // Prioridad 1B: Frontal asimétrico
    if (FrontalDer || FrontalIzq) {
        sensoresFrontales(false, FrontalDer, FrontalIzq);
        return;
    }

    // Prioridad 2 (MEDIA): Enemigo lateral (solo después de detectar el borde)
    if (LateralDer || LateralIzq) {
        sensoresLaterales(LateralIzq, LateralDer);
        return;
    }

    static unsigned long ultimoContacto = 0;
    if (enemigoDetectado) {
        ultimoContacto = millis();
    }

    // Prioridad 3 (BAJA): Búsqueda sin enemigo
    // Patrón: barrido de arco ~180° + paso corto, cubre trasero, laterales y frente.
    //   Fase 0: giro ~180° hacia busquedaDerecha       (~350 ms)
    //   Fase 1: avance corto hacia el interior          ( ~80 ms)
    //   Fase 2: giro ~180° en dirección contraria       (~350 ms)
    //   Fase 3: avance corto hacia el interior          ( ~80 ms)
    // Al completar el ciclo alterna la dirección inicial para no repetir el mismo patrón.
    static bool busquedaDerecha = true;
    static uint8_t faseBusqueda = 0;
    static unsigned long inicioFase = 0;

    if (recienEncendido) {
        faseBusqueda = 0;
        inicioFase = 0;
    }

    const unsigned long ahora = millis();
    const unsigned long tiempoSinContacto = ahora - ultimoContacto;
    const int margenSeguridadPiso = 35;
    const bool cercaBordeIzq = valorPisoIzq <= (BLANCO + margenSeguridadPiso);
    const bool cercaBordeDer = valorPisoDer <= (BLANCO + margenSeguridadPiso);

    // Si acaba de perder contacto, gira de inmediato para re-encontrar al enemigo.
    // No avanza: si el enemigo está detrás, avanzar lo alejaría.
    if (tiempoSinContacto < 130) {
        if (cercaBordeIzq || cercaBordeDer) {
            if (cercaBordeIzq && !cercaBordeDer) {
                moverDerecha();
            } else if (cercaBordeDer && !cercaBordeIzq) {
                moverIzquierda();
            } else if (busquedaDerecha) {
                moverDerecha();
            } else {
                moverIzquierda();
            }
        } else {
            // Gira en lugar de avanzar: cubre el ángulo trasero cuanto antes
            if (busquedaDerecha) moverDerecha(); else moverIzquierda();
        }
        faseBusqueda = 0;
        inicioFase = ahora;
        return;
    }

    if (inicioFase == 0) {
        inicioFase = ahora;
    }

    // Duraciones de cada fase del barrido 360° (2 arcos de ~180° con avance entre medias)
    const unsigned long duraciones[4] = {200, 60, 200, 60};

    if ((ahora - inicioFase) >= duraciones[faseBusqueda]) {
        inicioFase = ahora;
        faseBusqueda = (faseBusqueda + 1) % 4;
        // Al completar las 4 fases (vuelta completa), invierte dirección inicial
        if (faseBusqueda == 0) {
            busquedaDerecha = !busquedaDerecha;
        }
    }

    // Si hay borde cerca durante la búsqueda, re-centrar antes de seguir girando.
    if (cercaBordeIzq || cercaBordeDer) {
        if (cercaBordeIzq && !cercaBordeDer) {
            moverDerecha();
        } else if (cercaBordeDer && !cercaBordeIzq) {
            moverIzquierda();
        } else if (busquedaDerecha) {
            moverDerecha();
        } else {
            moverIzquierda();
        }
        faseBusqueda = 0;
        inicioFase = ahora;
        return;
    }

    // Fase 0: primer arco ~180°
    if (faseBusqueda == 0) {
        if (busquedaDerecha) moverDerecha(); else moverIzquierda();
    }
    // Fase 1: avance hacia el centro
    else if (faseBusqueda == 1) {
        motores.adelante(Velocidad_maxima);
    }
    // Fase 2: segundo arco ~180° en sentido contrario (completa los 360°)
    else if (faseBusqueda == 2) {
        if (busquedaDerecha) moverIzquierda(); else moverDerecha();
    }
    // Fase 3: avance hacia el centro
    else {
        motores.adelante(Velocidad_maxima);
    }
}

