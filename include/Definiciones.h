#ifndef DEFINICIONES_H
#define DEFINICIONES_H

// Pines de sensores
#define S_PISO_IZQ   A0
#define S_PISO_DER   A7
#define S_FRONT_IZQ  A2
#define S_FRONT_CEN  A4
#define S_FRONT_DER  A3
#define S_LAT_IZQ    A1
#define S_LAT_DER    A5

// Selector de modo de arranque
// Tocar linea: primero busca borde para posicionarse y luego combate.
// Pelea frente: entra a combate directo sin ir primero a la linea.
#define MODO_ARRANQUE_TOCAR_LINEA   0
#define MODO_ARRANQUE_PELEA_FRENTE  1
#define MODO_ARRANQUE_DEFAULT       MODO_ARRANQUE_TOCAR_LINEA
#define USAR_SELECTOR_MODO_ARRANQUE true
#define PIN_SELECTOR_MODO_ARRANQUE_0 2
#define PIN_SELECTOR_MODO_ARRANQUE_1 3
#define PIN_SELECTOR_MODO_ARRANQUE_2 4
#define NIVEL_SELECTOR_ACTIVO        LOW

// Compatibilidad con nombres anteriores
#define SENSOR_DE_PISO_IZQUIERDO  S_PISO_IZQ
#define SENSOR_DE_PISO_DERECHO    S_PISO_DER
#define SENSOR_FRONTAL_CENTRA     S_FRONT_CEN
#define SENSOR_FRONTAL_DERECHO    S_FRONT_DER
#define SENSOR_FRONTAL_IZQUIERDO  S_FRONT_IZQ
#define SENSOR_LATERAL_DERECHO    S_LAT_DER
#define SENSOR_LATERAL_IZQUIERDO  S_LAT_IZQ

// Pines de motores
#define PWMA 5
#define MA1A 7
#define MA2A 6
#define PWMB 10
#define MA1B 8
#define MA2B 9

// Ajuste de sentido por cableado/driver.
// Si un motor gira al revés, cambia true/false segun corresponda.
#define INVERTIR_MOTOR_IZQUIERDO false
#define INVERTIR_MOTOR_DERECHO   true

// ------------------ Control remoto (nivel) ------------------
// Pin_Control_Remoto: START=1, STOP=0
const int Pin_Control_Remoto         = DD4;
// Si tu control remoto entrega HIGH al activarse, deja esto en true.
// El arranque ahora exige un ciclo inactivo->activo, para evitar encendidos espurios.
const bool REMOTE_ACTIVE_HIGH        = true;
// Variables de estado (definidas en Robot.cpp)
extern bool robot_encendido;

// Parámetros
// BLANCO: umbral analógico del sensor de piso. Sube este valor si no detecta el borde a tiempo.
// Un valor más alto = detecta el borde antes (más margen de seguridad).
#define BLANCO 240

// Velocidades optimizadas para motores 700rpm en dojo 70x70 - rápido en ataque, seguro en piso
#define VELOCIDAD_ATAQUE_FRONTAL     240
#define VELOCIDAD_ATAQUE_LATERAL     255
#define VELOCIDAD_BUSQUEDA_AVANCE    22
#define VELOCIDAD_BUSQUEDA_GIRO      160
#define VELOCIDAD_ESCAPE_RETROCESO   200
#define VELOCIDAD_ESCAPE_GIRO        190
#define VELOCIDAD_APROXIMACION_BORDE 70
#define VELOCIDAD_SEGUIR_ENEMIGO     130

// Parámetros de detección y seguridad para 70x70 circular
#define MARGEN_SEGURIDAD_PISO        150
#define VENTANA_PISO_SEGURO_MS       500
#define DURACION_RETROCESO_AMBOS     520
#define DURACION_RETROCESO_SIMPLE    460
#define DURACION_GIRO_ESCAPE         760
#define COOLDOWN_BORDE_AMBOS         2100
#define COOLDOWN_BORDE_SIMPLE        1900

// Compatibilidad con nombres anteriores
#define Velocidad_movimiento_seguir VELOCIDAD_SEGUIR_ENEMIGO
#define Velocidad_estandar          VELOCIDAD_ESCAPE_RETROCESO
#define Velocidad_normal            VELOCIDAD_BUSQUEDA_AVANCE
#define Velocidad_maxima            VELOCIDAD_ATAQUE_FRONTAL
#define Velocidad_maxima_Ataque     VELOCIDAD_ATAQUE_FRONTAL
#define Velocidad_borde             VELOCIDAD_APROXIMACION_BORDE


#endif // DEFINICIONES_H