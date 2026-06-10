
#include <Arduino.h>
#include <avr/wdt.h>
#include "Robot.h"
#include "Definiciones.h"

// Variables estáticas para la rutina de búsqueda
static bool busquedaDerecha = true;
static uint8_t faseBusqueda = 0;
static unsigned long inicioFase = 0;
static unsigned long bordeCooldownHasta = 0;
static unsigned long pistaSeguraDesde = 0;
// -1: borde izquierdo, 1: borde derecho, 2: ambos, 0: sin dato.
static int ultimoBordeDetectado = 0;

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

// Suavizado exponencial móvil (EMA) para detección robusta de piso
void Robot::aplicarEMAsuavizado(int &suaveActual, int valorNuevo) {
    suaveActual = (int)(factorEMA * valorNuevo + (1.0f - factorEMA) * suaveActual);
}

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
    const unsigned long retrocesoMinimoMs = 80;
    while (millis() - inicio < duracionMs) {
        retroceder();

        bool pisoIzq = false;
        bool pisoDer = false;
        bool enBorde = leerPiso(pisoIzq, pisoDer);

        if (!enBorde) {
            if (pistaEstableDesde == 0) {
                pistaEstableDesde = millis();
            }
            if ((millis() - inicio) > retrocesoMinimoMs && (millis() - pistaEstableDesde) > 20) {
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
            motores.curvaDerecha(80);
        } else {
            motores.curvaIzquierda(80);
        }

        // Requiere una pequeña ventana estable fuera del borde para terminar el giro.
        if (!enBorde) {
            if (pistaEstableDesde == 0) {
                pistaEstableDesde = millis();
            }
            if ((millis() - inicio) > 100 && (millis() - pistaEstableDesde) > 50) {
                return;
            }
        } else {
            pistaEstableDesde = 0;
        }
    }
}

void Robot::giroEscapeCompleto(bool haciaDerecha, unsigned long duracionMs, int velocidadCurva) {
    unsigned long inicio = millis();
    unsigned long pistaEstableDesde = 0;
    const unsigned long giroMinimoMs = 170;
    while (millis() - inicio < duracionMs) {
        bool pisoIzq = false;
        bool pisoDer = false;
        bool enBorde = leerPiso(pisoIzq, pisoDer);

        if (haciaDerecha) {
            motores.curvaDerecha(velocidadCurva);
        } else {
            motores.curvaIzquierda(velocidadCurva);
        }

        if (!enBorde) {
            if (pistaEstableDesde == 0) {
                pistaEstableDesde = millis();
            }
            if ((millis() - inicio) > giroMinimoMs && (millis() - pistaEstableDesde) > 60) {
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
    pinMode(Pin_Control_Remoto, INPUT_PULLUP);
    pinMode(PIN_SELECTOR_MODO_ARRANQUE_0, INPUT_PULLUP);
    pinMode(PIN_SELECTOR_MODO_ARRANQUE_1, INPUT_PULLUP);
    pinMode(PIN_SELECTOR_MODO_ARRANQUE_2, INPUT_PULLUP);
}

void Robot::detenerse() {
    motores.detener();
}

void Robot::ataqueEnemigo() {
    motores.adelante(VELOCIDAD_ATAQUE_FRONTAL);
}

void Robot::moverAdelante() {
    motores.adelante(VELOCIDAD_SEGUIR_ENEMIGO);
}

void Robot::retroceder() {
    motores.retroceder(VELOCIDAD_ESCAPE_RETROCESO);
}

void Robot::moverDerecha() {
    motores.derecha(VELOCIDAD_BUSQUEDA_GIRO);
}

void Robot::moverIzquierda() {
    motores.izquierda(VELOCIDAD_BUSQUEDA_GIRO);
}

void Robot::sensoresPiso(bool pisoIzq, bool pisoDer) {
    static bool giroAlternadoDerecha = true;
    bool salirPorDerecha = true;

    if (pisoDer && !pisoIzq) {
        salirPorDerecha = false;
    } else if (pisoIzq && !pisoDer) {
        salirPorDerecha = true;
    } else if (pisoIzq && pisoDer) {
        salirPorDerecha = giroAlternadoDerecha;
    }

    if (pisoIzq && pisoDer) {
        ultimoBordeDetectado = 2;
        // Ambos sensores en borde: retroceso fuerte con pivote agresivo
        retrocesoSeguro(DURACION_RETROCESO_AMBOS);
        if (salirPorDerecha) {
            motores.derecha(VELOCIDAD_ESCAPE_GIRO);
        } else {
            motores.izquierda(VELOCIDAD_ESCAPE_GIRO);
        }
        delay(DURACION_GIRO_ESCAPE);
        bordeCooldownHasta = millis() + COOLDOWN_BORDE_AMBOS;
        giroAlternadoDerecha = !giroAlternadoDerecha;
    } else if (pisoDer) {
        ultimoBordeDetectado = 1;
        // Borde derecho: retroceso + pivote izquierdo fuerte
        retrocesoSeguro(DURACION_RETROCESO_SIMPLE);
        motores.izquierda(VELOCIDAD_ESCAPE_GIRO);
        delay(DURACION_GIRO_ESCAPE);
        bordeCooldownHasta = millis() + COOLDOWN_BORDE_SIMPLE;
    } else if (pisoIzq) {
        ultimoBordeDetectado = -1;
        // Borde izquierdo: retroceso + pivote derecho fuerte
        retrocesoSeguro(DURACION_RETROCESO_SIMPLE);
        motores.derecha(VELOCIDAD_ESCAPE_GIRO);
        delay(DURACION_GIRO_ESCAPE);
        bordeCooldownHasta = millis() + COOLDOWN_BORDE_SIMPLE;
    }
}

void Robot::sensoresFrontales(bool central, bool derecho, bool izquierdo) {
    // Aquí solo llega si es frontal asimétrico (no central), así que ataca hacia ese lado
      
    if (derecho && !izquierdo) {
        motores.derecha(VELOCIDAD_ATAQUE_LATERAL);
        if (esperarConPrioridadPiso(70)) return;
        motores.adelante(VELOCIDAD_ATAQUE_FRONTAL);
        esperarConPrioridadPiso(70);
    } else if (izquierdo && !derecho) {
        motores.izquierda(VELOCIDAD_ATAQUE_LATERAL);
        if (esperarConPrioridadPiso(70)) return;
        motores.adelante(VELOCIDAD_ATAQUE_FRONTAL);
        esperarConPrioridadPiso(70);
    }
}

void Robot::sensoresLaterales(bool sensorIzquierdo, bool sensorDerecho) {
    if (sensorIzquierdo && sensorDerecho) {
        // Si ambos laterales ven objetivo, empuje frontal fuerte
        motores.adelante(VELOCIDAD_ATAQUE_FRONTAL);
        esperarConPrioridadPiso(80);
        return;
    }

    bool pisoIzq = false, pisoDer = false;
    leerPiso(pisoIzq, pisoDer);
    if (pisoIzq || pisoDer) return;

    // Posicionamiento con pivote: giro rápido para alinearse
    if (sensorIzquierdo) {
        motores.izquierda(VELOCIDAD_ATAQUE_LATERAL);
        esperarConPrioridadPiso(100);
    } else if (sensorDerecho) {
        motores.derecha(VELOCIDAD_ATAQUE_LATERAL);
        esperarConPrioridadPiso(100);
    }
}

void Robot::loop() {
    // ------------------ Control remoto ------------------
    static unsigned long remotoInactivoDesde = 0;
    static bool remotoListoParaArrancar = false;
    static bool remotoActivoAnterior = false;
    const unsigned long filtroRemotoMs = 1200;

    int estado_actual = digitalRead(Pin_Control_Remoto);
    bool nivel_activo = REMOTE_ACTIVE_HIGH ? (estado_actual == HIGH) : (estado_actual == LOW);
    if (nivel_activo) {
        remotoInactivoDesde = 0;
        if (remotoListoParaArrancar && !remotoActivoAnterior) {
            robot_encendido = true;
        }
    } else {
        if (remotoInactivoDesde == 0) {
            remotoInactivoDesde = millis();
        }
        if ((millis() - remotoInactivoDesde) >= filtroRemotoMs) {
            remotoListoParaArrancar = true;
            robot_encendido = false;
        }
    }
    remotoActivoAnterior = nivel_activo;

    static bool estadoAnterior = false;
    static uint8_t modoArranqueActivo = MODO_ARRANQUE_DEFAULT;
    static unsigned long inicioModoMs = 0;
    if (!robot_encendido) {
        motores.detener();
        estadoAnterior = false;
        return;
    }
    const bool recienEncendido = !estadoAnterior;
    estadoAnterior = true;

    if (recienEncendido) {
        uint8_t modoLeido = MODO_ARRANQUE_DEFAULT;
        if (USAR_SELECTOR_MODO_ARRANQUE) {
            uint8_t selector = 0;
            if (digitalRead(PIN_SELECTOR_MODO_ARRANQUE_0) == NIVEL_SELECTOR_ACTIVO) selector |= 0x01;
            if (digitalRead(PIN_SELECTOR_MODO_ARRANQUE_1) == NIVEL_SELECTOR_ACTIVO) selector |= 0x02;
            if (PIN_SELECTOR_MODO_ARRANQUE_2 != Pin_Control_Remoto &&
                digitalRead(PIN_SELECTOR_MODO_ARRANQUE_2) == NIVEL_SELECTOR_ACTIVO) {
                selector |= 0x04;
            }

            // Mapeo actual de modos:
            // 000 => tocar linea
            // 001/010/011 => pelea frontal
            // otros valores quedan en modo por defecto
            if (selector == 0x01 || selector == 0x02 || selector == 0x03) {
                modoLeido = MODO_ARRANQUE_PELEA_FRENTE;
            } else if (selector == 0x00) {
                modoLeido = MODO_ARRANQUE_TOCAR_LINEA;
            }
        }
        modoArranqueActivo = modoLeido;
        inicioModoMs = millis();

        // Reiniciar estados de seguridad en cada arranque.
        bordeCooldownHasta = 0;
        pistaSeguraDesde = 0;
        bordeDetectado = (modoArranqueActivo == MODO_ARRANQUE_PELEA_FRENTE);
    }

    // LECTURA DE SENSORES
    bool PisoIzq = false, PisoDer = false;
    // Usar el mismo sensor (promedio de 3 lecturas) para coherencia con sensoresPiso().
    PisoIzq = sensorPisoIzq.detectar();
    PisoDer = sensorPisoDer.detectar();
    // Lecturas analógicas con suavizado EMA para margen preventivo de borde (menos ruido).
    int valorPisoIzqBruto = analogRead(SENSOR_DE_PISO_IZQUIERDO);
    valorPisoIzqBruto += analogRead(SENSOR_DE_PISO_IZQUIERDO);
    valorPisoIzqBruto += analogRead(SENSOR_DE_PISO_IZQUIERDO);
    valorPisoIzqBruto /= 3;
    int valorPisoDerBruto = analogRead(SENSOR_DE_PISO_DERECHO);
    valorPisoDerBruto += analogRead(SENSOR_DE_PISO_DERECHO);
    valorPisoDerBruto += analogRead(SENSOR_DE_PISO_DERECHO);
    valorPisoDerBruto /= 3;
    // Aplicar suavizado exponencial
    aplicarEMAsuavizado(suavePisoIzq, valorPisoIzqBruto);
    aplicarEMAsuavizado(suavePisoDer, valorPisoDerBruto);
    int valorPisoIzq = suavePisoIzq;
    int valorPisoDer = suavePisoDer;
    
    bool FrontalDer = sensorFrontalDer.detectar();
    bool FrontalIzq = sensorFrontalIzq.detectar();
    bool FrontalCentral = sensorFrontal.detectar();
    bool LateralDer = sensorLateralDer.detectar();
    bool LateralIzq = sensorLateralIzq.detectar();
    const bool enemigoDetectado = FrontalDer || FrontalIzq || FrontalCentral || LateralDer || LateralIzq;


    // Modo 1: tocar linea antes del combate.
    // Modo 2: pelea frontal inmediata (sin ir a la linea blanca primero).
    if (!bordeDetectado) {
        // Avanza hacia el borde a velocidad reducida
        motores.adelante(VELOCIDAD_APROXIMACION_BORDE);
        if (PisoIzq || PisoDer) {
            // Al detectar el borde, retrocede un poco y da un giro único.
            bordeDetectado = true;
            retrocesoSeguro(230);
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
    if (recienEncendido) {
        faseBusqueda = 0;
        inicioFase = 0;
    }

    const unsigned long ahora = millis();
    const unsigned long tiempoSinContacto = ahora - ultimoContacto;
    const bool cercaBordeIzq = valorPisoIzq <= (BLANCO + MARGEN_SEGURIDAD_PISO);
    const bool cercaBordeDer = valorPisoDer <= (BLANCO + MARGEN_SEGURIDAD_PISO);
    const bool ventanaSalidaFrontal =
        (modoArranqueActivo == MODO_ARRANQUE_PELEA_FRENTE) &&
        ((ahora - inicioModoMs) < 700) &&
        !PisoIzq && !PisoDer;
    const unsigned long tiempoPistaSegura = (pistaSeguraDesde == 0) ? 0 : (ahora - pistaSeguraDesde);
    const bool enemigoFrontal = FrontalCentral || FrontalDer || FrontalIzq;
    const bool enemigoLateral = (LateralDer || LateralIzq) && !enemigoFrontal;

    if (PisoIzq || PisoDer || ((cercaBordeIzq || cercaBordeDer) && !ventanaSalidaFrontal)) {
        pistaSeguraDesde = 0;
    } else if (pistaSeguraDesde == 0) {
        pistaSeguraDesde = ahora;
    }

    const bool enRecuperacionBorde = (ahora < bordeCooldownHasta);

    // Si ya está cerca del borde, corta el ataque y recentra antes de empujar.
    // Durante la recuperación no reinicia el escape: sigue alejándose.
    if ((cercaBordeIzq || cercaBordeDer) && !ventanaSalidaFrontal) {
        if (!enRecuperacionBorde) {
            sensoresPiso(cercaBordeIzq, cercaBordeDer);
            faseBusqueda = 0;
            inicioFase = ahora;
            return;
        }

        // Si reaparece el borde durante la recuperación, extiende la ventana
        bordeCooldownHasta = max(bordeCooldownHasta, ahora + 500);
    }

    // Ventana de seguridad después de escapar del borde: avanza hacia el centro.
    // Si el piso ya está estable, puede reanudar combate sin esperar todo el cooldown.
    // Frontal reanuda antes; lateral exige más estabilidad para evitar salidas.
    const unsigned long tiempoMinimoCombateRecuperacion = enemigoFrontal ? 110 : 180;
    const bool enemigoDetectadoRecuperacion = enemigoFrontal || enemigoLateral;
    const bool puedeAtacarEnRecuperacion =
        enemigoDetectadoRecuperacion &&
        !cercaBordeIzq && !cercaBordeDer &&
        !PisoIzq && !PisoDer &&
        tiempoPistaSegura >= tiempoMinimoCombateRecuperacion;

    if ((enRecuperacionBorde || pistaSeguraDesde == 0 || tiempoPistaSegura < VENTANA_PISO_SEGURO_MS) && !puedeAtacarEnRecuperacion) {
        if (cercaBordeIzq || cercaBordeDer || PisoIzq || PisoDer) {
            motores.retroceder(VELOCIDAD_ESCAPE_RETROCESO);
        } else if (enemigoFrontal) {
            // Mientras se recupera del borde, si hay enemigo frontal lo sigue con empuje moderado.
            motores.adelante(VELOCIDAD_SEGUIR_ENEMIGO);
        } else if (enemigoLateral) {
            // Seguimiento lateral durante recuperación para no "ignorar" al enemigo.
            if (LateralIzq && !LateralDer) {
                motores.izquierda(VELOCIDAD_SEGUIR_ENEMIGO);
            } else if (LateralDer && !LateralIzq) {
                motores.derecha(VELOCIDAD_SEGUIR_ENEMIGO);
            } else {
                motores.adelante(VELOCIDAD_SEGUIR_ENEMIGO);
            }
        } else {
            // Avanza suavemente hacia el centro durante la recuperación
            motores.adelante(35);
        }
        return;
    }

    // Prioridad 1 (ALTA): Enemigo frontal => empuje directo
    if (enemigoFrontal) {
        ataqueEnemigo();
        return;
    }

    // Prioridad 2 (MEDIA): Enemigo lateral => alinear primero para no salir del dohyo
    if (enemigoLateral) {
        sensoresLaterales(LateralIzq, LateralDer);
        return;
    }

    // Estrategia dedicada para modo pelea frontal:
    // si no hay enemigo detectado, presiona hacia adelante con barrido suave.
    if (modoArranqueActivo == MODO_ARRANQUE_PELEA_FRENTE) {
        static unsigned long inicioBarridoFrontal = 0;
        static bool barridoDerecha = true;
        const unsigned long faseBarridoMs = 140;

        if (inicioBarridoFrontal == 0) {
            inicioBarridoFrontal = ahora;
        }

        if ((ahora - inicioBarridoFrontal) >= faseBarridoMs) {
            inicioBarridoFrontal = ahora;
            barridoDerecha = !barridoDerecha;
        }

        if (barridoDerecha) {
            motores.curvaDerecha(VELOCIDAD_ATAQUE_FRONTAL);
        } else {
            motores.curvaIzquierda(VELOCIDAD_ATAQUE_FRONTAL);
        }
        return;
    }

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

    // Duraciones optimizadas para búsqueda rápida en 70x70: pivotes más cortos
    // Fase 0/2: giro ~180°, Fase 1/3: avance mínimo (solo para no quedarse quieto)
    const unsigned long duraciones[4] = {280, 10, 280, 10};

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
    // Fase 1: avance corto hacia el centro — solo si no hay borde cerca
    else if (faseBusqueda == 1) {
        if (!cercaBordeIzq && !cercaBordeDer && !PisoIzq && !PisoDer) {
            if (busquedaDerecha) {
                motores.curvaDerecha(70);
            } else {
                motores.curvaIzquierda(70);
            }
            if (esperarConPrioridadPiso(12)) {
                bool pisoI = false;
                bool pisoD = false;
                leerPiso(pisoI, pisoD);
                sensoresPiso(pisoI, pisoD);
                faseBusqueda = 0;
                inicioFase = ahora;
                return;
            }
        } else {
            // Borde detectado: cancela avance y gira hacia el centro
            faseBusqueda = 0;
            inicioFase = ahora;
            if (busquedaDerecha) moverDerecha(); else moverIzquierda();
        }
    }
    // Fase 2: segundo arco ~180° en sentido contrario (completa los 360°)
    else if (faseBusqueda == 2) {
        if (busquedaDerecha) moverIzquierda(); else moverDerecha();
    }
    // Fase 3: avance hacia el centro — solo si no hay borde cerca
    else {
        if (!cercaBordeIzq && !cercaBordeDer && !PisoIzq && !PisoDer) {
            if (busquedaDerecha) {
                motores.curvaIzquierda(70);
            } else {
                motores.curvaDerecha(70);
            }
            if (esperarConPrioridadPiso(12)) {
                bool pisoI = false;
                bool pisoD = false;
                leerPiso(pisoI, pisoD);
                sensoresPiso(pisoI, pisoD);
                faseBusqueda = 0;
                inicioFase = ahora;
                return;
            }
        } else {
            faseBusqueda = 0;
            inicioFase = ahora;
            if (busquedaDerecha) moverDerecha(); else moverIzquierda();
        }
    }
}
