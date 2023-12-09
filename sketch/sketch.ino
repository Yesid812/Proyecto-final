
/**
 * \file sketch.ino
 * \date 2023-06-27
 * \author Anderson Vinasco Giraldo <avinasco@unicauca.edu.co>
 *         Duber Andres ErasoUni <dubereraso@unicauca.edu.co>
 *         Eider Yesid Obando <eyobando@unicauca.edu.co>
 * \brief ejecucion completa.
 *
 * \par Copyright
 * Information contained herein is proprietary to and constitutes valuable
 * confidential trade secrets of Unicauca, and
 * is subject to restrictions on use and disclosure.
 *
 * \par
 * Copyright (c) Unicauca 2023. All rights reserved.
 *
 * \par
 * The copyright notices above do not evidence any actual or
 * intended publication of this material.
 ******************************************************************************
 */

#include "StateMachineLib.h"
#include "AsyncTaskLib.h"
#include <Keypad.h>
#include <LiquidCrystal.h>

/**
********************************************//**
 *  Configuracion del LCD
 * \addtogroup LCD_Config
 * @{
 * \brief Configuracion del LCD
 ***********************************************
*/
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;

/** \addtogroup Sistema_Seguridad
 * @{
 * ----- SISTEMA DE SEGURIDAD -----
 */

/** 
*brief Configuracion del teclado 
*/
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte rowPins[ROWS] = { 24, 26, 28, 30 };
byte colPins[COLS] = { 32, 34, 36, 38 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

char password[4] = "1234";
char temp1[4];

int numChar = 0;
byte attempts = 0;

// ----- LEDs -----
#define red 8
#define green 9
#define blue 10
/**
* ----- SENSORES -----
* Photoresistor
*/
const int photocellPin = A0;
const int ledPin = 13;
/**
* Analog Temperature Sensor
*/
const int TemperaturePin = A1;
#define beta 4090      //the beta of the thermistor
#define resistance 10  //the value of the pull-down resistorvoid

/**
* ----- BUZZER -----
*/
#define buzzer 7

/** 
 * addtogroup Tareas_Asincronicas
 * brief Tareas asincronicas para tiempos de espera
 */

/** 
* brief Función de tiempo de espera para las tareas asincrónicas. 
*/
void function_TimeOut(void);

/** 
*brief Tarea asincrónica para esperar 3 segundos. 
*/
AsyncTask esperar3s(3000, false, function_TimeOut);

/** 
* brief Tarea asincrónica para esperar 4 segundos. 
*/
AsyncTask esperar4s(4000, false, function_TimeOut);

/** 
* brief Tarea asincrónica para esperar 5 segundos. 
*/
AsyncTask esperar5s(5000, false, function_TimeOut);

/** * brief Tarea asincrónica para esperar 6 segundos. 
*/
AsyncTask esperar6s(6000, false, function_TimeOut);

/** 
*brief Tarea asincrónica para esperar 10 segundos. 
*/
AsyncTask esperar10s(10000, false, function_TimeOut);

void sistemaDeSeguridad(void);
AsyncTask taskSistemaDeSeguridad(0, true, sistemaDeSeguridad);

void bloqueo(void);
AsyncTask taskBloqueo(0, false, bloqueo);

void monitorLuz(void);
AsyncTask taskMonitorLuz(500, true, monitorLuz);

void monitorTemperatura(void);
AsyncTask taskMonitorTemperatura(500, true, monitorTemperatura);

void alertaAltaLuz(void);
AsyncTask taskAlertaBajaLuz(0, true, alertaAltaLuz);

void alertaAltaTemperatura(void);
AsyncTask taskAlertaAltaTemperatura(0, true, alertaAltaTemperatura);

/** 
 * addtogroup Maquina_Estados
 * brief Maquina de estados
 */

/** 
*brief Enumeracion de los estados de la maquina de estados 
*/
enum State {
  Init = 0,
  Bloqueado = 1,
  MonitorLuz = 2,
  MonitorTemperatura = 3,
  AlertaLed = 4,
  AlertaBuzzer = 5
};

/** 
* brief Enumeracion de las entradas de la maquina de estados 
*/
enum Input {
  SistemaBloqueado = 0,
  ClaveCorrecta = 1,
  LuzMenor40 = 2,
  TemperaturaMayor30 = 3,
  TimeOut = 4,
  Unknown = 5
};

// Crear maquina de estado
StateMachine stateMachine(6, 10);

// Indice para cambiar de estado
Input currentInput = Input::Unknown;

/****************************************************************************
* NAME: setupStateMachine
*----------------------------------------------------------------------------
* PARAMS:
* return:   none
*----------------------------------------------------------------------------
*brief Configuracion inicial de la maquina de estados
*----------------------------------------------------------------------------
* NOTE:
*
****************************************************************************
*/
/**
 * Funcione para configurar la maquina
 */
void setupStateMachine() {

/** 
 *  Añadir transiciones
 */
  stateMachine.AddTransition(Init, Bloqueado, []() {
    return currentInput == SistemaBloqueado;
  });
  stateMachine.AddTransition(Init, Bloqueado, []() {
    return currentInput == TimeOut;
  });
  stateMachine.AddTransition(Init, MonitorLuz, []() {
    return currentInput == ClaveCorrecta;
  });

  stateMachine.AddTransition(Bloqueado, Init, []() {
    return currentInput == TimeOut;
  });

  stateMachine.AddTransition(MonitorLuz, MonitorTemperatura, []() {
    return currentInput == TimeOut;
  });
  stateMachine.AddTransition(MonitorLuz, AlertaLed, []() {
    return currentInput == LuzMenor40;
  });

  stateMachine.AddTransition(MonitorTemperatura, MonitorLuz, []() {
    return currentInput == TimeOut;
  });
  stateMachine.AddTransition(MonitorTemperatura, AlertaBuzzer, []() {
    return currentInput == TemperaturaMayor30;
  });

  stateMachine.AddTransition(AlertaLed, MonitorLuz, []() {
    return currentInput == TimeOut;
  });

  stateMachine.AddTransition(AlertaBuzzer, MonitorTemperatura, []() {
    return currentInput == TimeOut;
  });

/** 
 *  Añadir acciones
 */
  stateMachine.SetOnEntering(Init, outputInit);
  stateMachine.SetOnEntering(Bloqueado, outputBloqueado);
  stateMachine.SetOnEntering(MonitorLuz, outputMonitorLuz);
  stateMachine.SetOnEntering(MonitorTemperatura, outputMonitorTemperatura);
  stateMachine.SetOnEntering(AlertaLed, outputAlertaLed);
  stateMachine.SetOnEntering(AlertaBuzzer, outputAlertaBuzzer);
}

void setup() {
// SISTEMA DE SEGURIDAD
  lcd.begin(16, 2);
  pinMode(red, OUTPUT);   //rojo
  pinMode(green, OUTPUT);   //verde
  pinMode(blue, OUTPUT);  //azul

/** 
 *  BUZZER
 */
  pinMode(buzzer, OUTPUT);

/** 
 *  MAQUINA DE ESTADOS
 */
  setupStateMachine();

// Asignar condicion inicial
  stateMachine.SetState(Init, false, true);
}

/** 
 * addtogroup Loop
 * brief Función principal de bucle.
 */
void loop() {
// Actualizar tareas asincronicas
  esperar3s.Update();
  esperar4s.Update();
  esperar5s.Update();
  esperar6s.Update();
  esperar10s.Update();

  taskSistemaDeSeguridad.Update();
  taskBloqueo.Update();
  taskMonitorLuz.Update();
  taskMonitorTemperatura.Update();
  taskAlertaBajaLuz.Update();
  taskAlertaAltaTemperatura.Update();

// Actualizar maquina de estados
  stateMachine.Update();
  currentInput = Unknown;
}

/** 
 *  ----- FUNCIONES DE SALIDA PARA LOS ESTADOS -----
 */

/** 
 *  brief Salida para el estado Init. 
*/
void outputInit() {
  lcd.clear();
  lcd.print("Ingrese clave: ");
  lcd.setCursor(0, 1);

  esperar10s.Start();
  taskSistemaDeSeguridad.Start();
}

/** 
* brief Salida para el estado Bloqueado. 
*/
void outputBloqueado() {
  esperar10s.Stop();
  taskSistemaDeSeguridad.Stop();

  esperar5s.Start();
  taskBloqueo.Start();
}

/** 
 *  brief Salida para el estado MonitorLuz. 
*/
void outputMonitorLuz() {
  esperar10s.Stop();
  taskSistemaDeSeguridad.Stop();

  taskMonitorTemperatura.Stop();

  taskAlertaBajaLuz.Stop();

  esperar3s.Start();
  taskMonitorLuz.Start();
}

/** 
 *  brief Salida para el estado MonitorTemperatura. 
*/
void outputMonitorTemperatura() {
  taskMonitorLuz.Stop();

  taskAlertaAltaTemperatura.Stop();

  esperar6s.Start();
  taskMonitorTemperatura.Start();
}

/** 
 *  brief Salida para el estado AlertaLed. 
*/
void outputAlertaLed() {
  esperar3s.Stop();
  taskMonitorLuz.Stop();

  esperar4s.Start();
  taskAlertaBajaLuz.Start();
}

/** 
* brief Salida para el estado AlertaBuzzer. 
*/
void outputAlertaBuzzer() {
  esperar6s.Stop();
  taskMonitorTemperatura.Stop();

  esperar5s.Start();
  taskAlertaAltaTemperatura.Start();
}

// ----- FUNCIONES PRINCIPALES -----

/** 
 *  brief Función principal para el sistema de seguridad.
*/
void sistemaDeSeguridad(void) {
  digitalWrite(red, LOW);

  char key = keypad.getKey();
  if (key) {
    lcd.print("*");
    temp1[numChar] = key;
    numChar++;
  }

  if (numChar == 4) {
    numChar = 0;
    lcd.clear();

    if (compararPalabras(4, temp1, password)) {
      lcd.print("Acceso Concedido");
      digitalWrite(green, HIGH);
      delay(1000);
      digitalWrite(green, LOW);
      lcd.clear();
      currentInput = ClaveCorrecta;

    } else {
      lcd.print("Denegado");
      attempts++;
      digitalWrite(blue, HIGH);
      delay(1000);
      digitalWrite(blue, LOW);
      lcd.clear();
      lcd.print("Ingrese clave: ");
      lcd.setCursor(0, 1);
    }

    if (attempts == 3)
      currentInput = SistemaBloqueado;
  }
}

/** 
 *  brief Función para el estado Bloqueo.
*/
void bloqueo(void) {
  digitalWrite(red, HIGH);

  if (attempts < 3) {
    lcd.clear();
    lcd.print("Tiempo");
    lcd.setCursor(0, 1);
    lcd.print("Agotado");
    delay(1000);

  } else {
    lcd.clear();
    lcd.print("Demasiados ");
    lcd.setCursor(0, 1);
    lcd.print("Intentos");
    delay(1000);
  }

  numChar = 0;
  attempts = 0;
  lcd.clear();
  lcd.print("Sistema");
  lcd.setCursor(0, 1);
  lcd.print("Bloqueado");
}

/** 
 *  brief Función para monitorear la luz. 
*/
void monitorLuz(void) {
  int luz = analogRead(photocellPin);

  lcd.clear();
  lcd.print("Luz: ");
  lcd.print(luz);

  if (luz < 40)
    currentInput = LuzMenor40;
}

/** 
 *  brief Función para monitorear la temperatura. 
*/
void monitorTemperatura(void) {
  long a = 1023 - analogRead(TemperaturePin);
  float tempC = beta / (log((1025.0 * 10 / a - 10) / 10) + beta / 298.0) - 273.0;

  lcd.clear();
  lcd.print("Temp: ");
  lcd.print(tempC);
  lcd.print(" C");

  if (tempC > 30)
    currentInput = TemperaturaMayor30;
}

/** 
 *  brief Función para alerta de luz alta. 
*/
void alertaAltaLuz(void) {
  lcd.clear();
  lcd.print("---- ALERTA ----");
  lcd.setCursor(0, 1);
  lcd.print("Alta iluminacion");

  digitalWrite(red, HIGH);
  delay(500);
  digitalWrite(red, LOW);
  delay(200);
}

/** 
 *  brief Función para alerta de temperatura alta. 
*/
void alertaAltaTemperatura(void) {
  lcd.clear();
  lcd.print("---- ALERTA ----");
  lcd.setCursor(0, 1);
  lcd.print("Alta temperatura");

  tone(buzzer, 1000, 5000); // Activar el tono a una frecuencia de 1000 Hz durante 200 milisegundos
  delay(500);
  noTone(buzzer); // Desactivar el tono en el zumbador
}

/** 
 * addtogroup Funciones_Auxiliares
 * brief Función de tiempo de espera para las tareas asincrónicas.
 */
void function_TimeOut(void) {
  currentInput = TimeOut;
}

/**
 * brief Función para comparar dos palabras. 
*/
bool compararPalabras(int longitud, char pal1[], char pal2[]) {
  int igualdad = 0;
  for (int i = 0; i < longitud; i++) {
    if (pal1[i] != pal2[i]) {
      igualdad = 1;
      break;
    }
  }

  if (igualdad == 0)
    return true;
  else
    return false;
}
