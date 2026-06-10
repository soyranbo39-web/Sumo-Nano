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

// Parámetros (Ajustes para motores de 700 RPM)
#define BLANCO 300 // Aumentado para detectar el blanco un poco antes
#define Velocidad_movimiento_seguir 80 // Reducido para mejor control con 700 rpm
#define Velocidad_estandar 100 // Para retrocesos (rápido pero sin salirse)
#define Velocidad_normal 80
#define Velocidad_maxima 110 // Para giros y búsquedas (evita giros descontrolados)
#define Velocidad_maxima_Ataque 220 // Embestida frontal
#define Velocidad_borde 60 // Avance cuidadoso hacia el borde


#endif // DEFINICIONES_H

//los motores son de 700 rpm quiero que ajustes los movimientos y la deteccion del piso para que nos e salga o no haga cosas extrañas