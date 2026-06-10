#ifndef DEFINICIONES_H
#define DEFINICIONES_H

// Pines de sensores
// #define PIN_MODO_COMBATE 12
#define S_PISO_IZQ   A0
#define S_PISO_DER   A7
#define S_FRONT_IZQ  A2
#define S_FRONT_CEN  A4
#define S_FRONT_DER  A3
#define S_LAT_IZQ    A1
#define S_LAT_DER    A5

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
// Si tu receptor es activo-alto (START=1, STOP=0), true.
// Si lo vieras invertido, pon false.
const bool REMOTE_ACTIVE_HIGH        = true;
// Variables de estado (definidas en Robot.cpp)
extern bool robot_encendido;

// Parámetros
#define BLANCO 120

// Velocidades ajustadas para motores de 700 rpm.
#define VELOCIDAD_ATAQUE_FRONTAL     100
#define VELOCIDAD_ATAQUE_LATERAL     180
#define VELOCIDAD_BUSQUEDA_AVANCE    140
#define VELOCIDAD_BUSQUEDA_GIRO      180
#define VELOCIDAD_ESCAPE_RETROCESO   200
#define VELOCIDAD_ESCAPE_GIRO        255
#define VELOCIDAD_APROXIMACION_BORDE 100
#define VELOCIDAD_SEGUIR_ENEMIGO     180

// Compatibilidad con nombres anteriores
#define Velocidad_movimiento_seguir VELOCIDAD_SEGUIR_ENEMIGO
#define Velocidad_estandar          VELOCIDAD_ESCAPE_RETROCESO
#define Velocidad_normal            VELOCIDAD_BUSQUEDA_AVANCE
#define Velocidad_maxima            VELOCIDAD_ATAQUE_FRONTAL
#define Velocidad_maxima_Ataque     VELOCIDAD_ATAQUE_FRONTAL
#define Velocidad_borde             VELOCIDAD_APROXIMACION_BORDE


#endif // DEFINICIONES_H