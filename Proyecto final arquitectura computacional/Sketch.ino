/**
 * @file sketch.ino
 * @date 2023-12-13
 *@version 1.0
 * @author Anderson Vinasco Giraldo <avinasco@unicauca.edu.co>
 *         Duber Andres ErasoUni <dubereraso@unicauca.edu.co>
 *         Eider Yesid Obando <eyobando@unicauca.edu.co>
 * @brief Un programa que utiliza una maquna de estados finito, juntos con funciones asíncronicas para detectar cambios en la temperatura y emitir alertas.
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

// ----- SISTEMA DE SEGURIDAD -----


 /** 
 *@brief Definicion del pin rs.
 *        - `rs`:  Pin de registro
 */
const int rs = 12;
  /** 
 *@brief Definicion del pin en.
 *        - `en`:  Pin de habilitacion
 */
const int en = 11;
  /** 
 *@brief Definicion del pin d4.
 *        - `d4`:  Pin de datos.
 */
const int d4 = 5;
/**
 *@brief Definicion del pin d5.
 *        - `d5`:  Pin de datos.
 */
const int d5 = 4;
/** 
 *@brief Definicion del pin d6.
 *        - `d6`:  Pin de datos.
 */
const int d6 = 3;
   /** 
 *@brief Definicion del pin d4.
 *        - `d7`:  Pin de datos.
 */
const int d7 = 2;




/**
*@brief  Creación de un objeto de tipo LiquidCrystal para la pantalla LCD.
*@param rs Pin de registro.
*@param en Pin de habilitación.
*@param d4 Pin de datos.
*@param d5 Pin de datos.
*@param d6 Pin de datos.
*@param d7 Pin de datos.
*/
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

/**
*@brief Definicion de constantes que representan el número de filas que tendrá el keypad.
*/
const byte ROWS = 4;
/**
*@brief Definicion de constantes que representan el número de columnas que tendrá el keypad.
*/
const byte COLS = 4;

/**
*@brief Declaracion de los caracteres que representan el teclado para el keypad.
*/
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

/**
*@brief Variable de tipo byte que almacena el arreglo que representan los pines de las filas del keypad.
*/
byte rowPins[ROWS] = { 24, 26, 28, 30 };
/**
*@brief Variable de tipo byte que almacena el arreglo que representan los pines de las columnas del keypad.
*/
byte colPins[COLS] = { 32, 34, 36, 38 };

/**
*@brief Declaración de un objeto de la clase Keypad.
*@param makeKeymap(keys) Funcion del mapa de teclas basado en el arreglo keys. 
*@param rowPins El arreglo rowPins
*@param colPins El arreglo colsPins
*@param ROWS  El arreglo ROWS
*@param COLS  El arreglo COLS
*/
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


/**
*@brief Variable que almacena la contraseña
*/
char password[4] = "1234";


/**
*@brief Variable auxiliar temporal
*/
char temp1[4];

/**
*@brief Variable de timpo numChar inicializada en 0.
*/
int numChar = 0;

/**
*@brief Variable de tipo byte que almacena el número de intentos
*/
byte attempts = 0;

 /** 
 *@brief Definicion del pin para el LED rojo.
 *        - `Rojo`:   Pin 8
 */
// ----- LEDs -----
#define red 8
 /** 
 *@brief Definicion del Pin para el LED verde.
 *        - `Verde`: Pin 9
 */
#define green 9
 /** 
 *@brief Definicion del Pin para el LED azul.
 *        - `Azul`: Pin 10
 */
#define blue 10


 /** 
 *@brief Variable que almacena el pin del sensor photocellPin.
 *        - `photocellPin`:   Pin A0
 */
const int photocellPin = A0;
 /** 
 *@brief variable que almacena el pin del sensor ledPin.
 *        - `ledPin`: Pin 13
 */
const int ledPin = 13;

/**
*@brief Variable que almacena el pin de la temperatura.
*/
const int TemperaturePin = A1;

/**
 * @brief Valor de beta del termistor.
 *
 * Este valor se utiliza en el cálculo de la temperatura en base a la resistencia del termistor.
 */
#define beta 4090      // El beta del termistor

/**
 * @brief Valor de la resistencia de pull-down.
 *
 * Este valor representa la resistencia de pull-down utilizada en el circuito.
 */
#define resistance 10  // El valor de la resistencia de pull-down


/**
*@brief Definicion de la constante del buzzer establecido en 7
*/
#define buzzer 7



/**
 * @brief Función que se llama cuando se produce un tiempo de espera (timeout).
 *
 *
 * @details
 * Esta función puede ser utilizada como una devolución de llamada (callback).
 * @note
 * Asegúrate de ajustar el tiempo de espera en la tarea asíncrona o temporizador
 * que utiliza esta función para que coincida con el comportamiento deseado.
 *
 *@return Esta funcion no retorna ningún valor.
 */
void function_TimeOut(void);

/**
 * @brief Tarea asincrónica para esperar 3 segundos.
 *
 * Esta tarea asincrónica se utiliza para programar una espera de 3 segundos
 * antes de ejecutar la función `function_TimeOut`.
 *
 * @param[in] 3000 Milisegundos de espera (3 segundos).
 * @param[in] false Indica que la tarea no se ejecutará repetidamente.
 * @param[in] function_TimeOut Función que se ejecutará al completarse el tiempo de espera.
 */
AsyncTask esperar3s(3000, false, function_TimeOut);
/**
 * @brief Tarea asincrónica para esperar 4 segundos.
 *
 * Esta tarea asincrónica se utiliza para programar una espera de 4 segundos
 * antes de ejecutar la función `function_TimeOut`.
 *
 * @param[in] 4000 Milisegundos de espera (4 segundos).
 * @param[in] false Indica que la tarea no se ejecutará repetidamente.
 * @param[in] function_TimeOut Función que se ejecutará al completarse el tiempo de espera.
 */
AsyncTask esperar4s(4000, false, function_TimeOut);
/**
 * @brief Tarea asincrónica para esperar 5 segundos.
 *
 * Esta tarea asincrónica se utiliza para programar una espera de 5 segundos
 * antes de ejecutar la función `function_TimeOut`.
 *
 * @param[in] 5000 Milisegundos de espera (5 segundos).
 * @param[in] false Indica que la tarea no se ejecutará repetidamente.
 * @param[in] function_TimeOut Función que se ejecutará al completarse el tiempo de espera.
 */
AsyncTask esperar5s(5000, false, function_TimeOut);
/**
 * @brief Tarea asincrónica para esperar 6 segundos.
 *
 * Esta tarea asincrónica se utiliza para programar una espera de 6 segundos
 * antes de ejecutar la función `function_TimeOut`.
 *
 * @param[in] 6000 Milisegundos de espera (6 segundos).
 * @param[in] false Indica que la tarea no se ejecutará repetidamente.
 * @param[in] function_TimeOut Función que se ejecutará al completarse el tiempo de espera.
 */
AsyncTask esperar6s(6000, false, function_TimeOut);

/**
 * @brief Tarea asincrónica para esperar 10 segundos.
 *
 * Esta tarea asincrónica se utiliza para programar una espera de 10 segundos
 * antes de ejecutar la función `function_TimeOut`.
 *
 * @param[in] 10000 Milisegundos de espera (10 segundos).
 * @param[in] false Indica que la tarea no se ejecutará repetidamente.
 * @param[in] function_TimeOut Función que se ejecutará al completarse el tiempo de espera.
 */
AsyncTask esperar10s(10000, false, function_TimeOut);

/**
 * @brief Función encargada de gestionar el sistema de seguridad.
 *
 * Esta función se llama para manejar las operaciones relacionadas con el sistema de seguridad.
 * Puede incluir la interacción con un teclado para ingresar claves, activar o desactivar alarmas, etc.
 *@return Esta funcion no retorna ningún valor.
 */
void sistemaDeSeguridad(void);

/**
 * @brief Tarea asincrónica para el sistema de seguridad.
 *
 * Esta tarea asincrónica se configura para ejecutar la función `sistemaDeSeguridad`
 * de forma periódica o en respuesta a algún evento específico.
 *
 * @param[in] 0 No hay tiempo de espera inicial.
 * @param[in] true Indica que la tarea se ejecutará repetidamente.
 * @param[in] sistemaDeSeguridad Función asociada a la tarea que se ejecutará.
 */
AsyncTask taskSistemaDeSeguridad(0, true, sistemaDeSeguridad);

/**
 * @brief Función para gestionar el bloqueo del sistema.
 *
 * Esta función se llama para realizar acciones relacionadas con el bloqueo del sistema,
 * como mostrar mensajes, activar señales de advertencia, etc.
 *@return Esta funcion no retorna ningún valor.
 */
void bloqueo(void);

/**
 * @brief Tarea asincrónica para el bloqueo del sistema.
 *
 * Esta tarea asincrónica se configura para ejecutar la función `bloqueo`
 * en respuesta a algún evento específico, pero no se repetirá automáticamente.
 *
 * @param[in] 0 No hay tiempo de espera inicial.
 * @param[in] false Indica que la tarea no se ejecutará repetidamente.
 * @param[in] bloqueo Función asociada a la tarea que se ejecutará.
 */
AsyncTask taskBloqueo(0, false, bloqueo);

/**
 * @brief Función para monitorear la intensidad de luz.
 *
 * Esta función se llama para medir y gestionar la intensidad de luz en el entorno.
 *@return Esta funcion no retorna ningún valor.
 */
void monitorLuz(void);

/**
 * @brief Tarea asincrónica para el monitoreo de la luz.
 *
 * Esta tarea asincrónica se configura para ejecutar la función `monitorLuz`
 * de forma periódica.
 *
 * @param[in] 500 Milisegundos de espera inicial antes de la primera ejecución.
 * @param[in] true Indica que la tarea se ejecutará repetidamente.
 * @param[in] monitorLuz Función asociada a la tarea que se ejecutará.
 */
AsyncTask taskMonitorLuz(500, true, monitorLuz);

/**
 * @brief Función para monitorear la temperatura.
 *
 * Esta función se llama para medir y gestionar la temperatura en el entorno.
*@return Esta funcion no retorna ningún valor.
 */
void monitorTemperatura(void);

/**
 * @brief Tarea asincrónica para el monitoreo de la temperatura.
 *
 * Esta tarea asincrónica se configura para ejecutar la función `monitorTemperatura`
 * de forma periódica.
 *
 * @param[in] 500 Milisegundos de espera inicial antes de la primera ejecución.
 * @param[in] true Indica que la tarea se ejecutará repetidamente.
 * @param[in] monitorTemperatura Función asociada a la tarea que se ejecutará.
 */
AsyncTask taskMonitorTemperatura(500, true, monitorTemperatura);

/**
 * @brief Función para manejar alertas de alta intensidad de luz.
 *
 * Esta función se llama para realizar acciones relacionadas con alertas de alta intensidad de luz,
 * como activar señales de advertencia, mostrar mensajes, etc.
*@return Esta funcion no retorna ningún valor.
 */
void alertaAltaLuz(void);

/**
 * @brief Tarea asincrónica para alertas de baja intensidad de luz.
 *
 * Esta tarea asincrónica se configura para ejecutar la función `alertaAltaLuz`
 * en respuesta a algún evento específico, pero no se repetirá automáticamente.
 *
 * @param[in] 0 No hay tiempo de espera inicial.
 * @param[in] true Indica que la tarea no se ejecutará repetidamente.
 * @param[in] alertaAltaLuz Función asociada a la tarea que se ejecutará.
 */
AsyncTask taskAlertaBajaLuz(0, true, alertaAltaLuz);

/**
 * @brief Función para manejar alertas de alta temperatura.
 *
 * Esta función se llama para realizar acciones relacionadas con alertas de alta temperatura,
 * como activar señales de advertencia, mostrar mensajes, etc.
 *@return Esta funcion no retorna ningún valor.
 */
void alertaAltaTemperatura(void);

/**
 * @brief Tarea asincrónica para alertas de alta temperatura.
 *
 * Esta tarea asincrónica se configura para ejecutar la función `alertaAltaTemperatura`
 * en respuesta a algún evento específico, pero no se repetirá automáticamente.
 *
 * @param[in] 0 No hay tiempo de espera inicial.
 * @param[in] true Indica que la tarea no se ejecutará repetidamente.
 * @param[in] alertaAltaTemperatura Función asociada a la tarea que se ejecutará.
 */
AsyncTask taskAlertaAltaTemperatura(0, true, alertaAltaTemperatura);




// ----- MAQUINA DE ESTADOS -----
// Nombres de estados
/**
 * @brief Enumeración que representa los estados de la máquina de estados.
 */
enum State {
  /**
   * @brief Estado de inicialización.
   */
  Init = 0,

  /**
   * @brief Estado bloqueado.
   */
  Bloqueado = 1,

  /**
   * @brief Estado de monitoreo de luz.
   */
  MonitorLuz = 2,

  /**
   * @brief Estado de monitoreo de temperatura.
   */
  MonitorTemperatura = 3,

  /**
   * @brief Estado de alerta con LED.
   */
  AlertaLed = 4,

  /**
   * @brief Estado de alerta con Buzzer.
   */
  AlertaBuzzer = 5
};


// Nombres de entradas
/**
 * @brief Enumeración que representa las entradas de la máquina de estados.
 */
enum Input {
  /**
   * @brief Entrada para indicar que el sistema está bloqueado.
   *
   * Esta entrada se utiliza cuando el sistema se encuentra en un estado bloqueado.
   */
  SistemaBloqueado = 0,

  /**
   * @brief Entrada para indicar que la clave ingresada es correcta.
   *
   * Esta entrada se utiliza cuando la clave ingresada coincide con la clave esperada.
   */
  ClaveCorrecta = 1,

  /**
   * @brief Entrada para indicar que la luz es menor a 40 unidades.
   *
   * Esta entrada se utiliza cuando la intensidad de luz medida es menor a 40 unidades.
   */
  LuzMenor40 = 2,

  /**
   * @brief Entrada para indicar que la temperatura es mayor a 30 grados Celsius.
   *
   * Esta entrada se utiliza cuando la temperatura medida es mayor a 30 grados Celsius.
   */
  TemperaturaMayor30 = 3,

  /**
   * @brief Entrada para indicar que ha ocurrido un tiempo de espera (timeout).
   *
   * Esta entrada se utiliza cuando ha ocurrido un tiempo de espera en la máquina de estados.
   */
  TimeOut = 4,

  /**
   * @brief Entrada desconocida.
   *
   * Esta entrada se utiliza cuando la entrada no se puede clasificar en ninguna categoría conocida.
   */
  Unknown = 5
};


/**
 * @brief Declaracion de la máquina de estados finitos.
 *
 * Esta máquina de estados finitos se utiliza para controlar el flujo del programa
 * en base a los diferentes estados y transiciones definidos.
 *
 * @param[in] 6 Número total de estados en la máquina.
 * @param[in] 10 Número máximo de transiciones que la máquina puede manejar simultáneamente.
 */
// Crear maquina de estado
StateMachine stateMachine(6, 10);


/**
 * @brief Indica la entrada actual para la máquina de estados.
 *
 * Esta variable se utiliza para determinar la entrada actual que afectará las transiciones en la máquina de estados.
 * Almacena el valor de la entrada actual, que puede ser uno de los valores definidos en la enumeración Input.
 */
// Indice para cambiar de estado
Input currentInput = Input::Unknown;


/**
 * @brief Configuración inicial de la máquina de estados.
 *
 * Esta función se encarga de configurar la máquina de estados con sus transiciones y acciones asociadas a cada estado.
 * Se definen las transiciones entre los estados y las funciones que se ejecutarán al entrar en cada estado.
 */
// Funcione para configurar la maquina
void setupStateMachine() {
  // Añadir transiciones
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

  // Añadir acciones
  stateMachine.SetOnEntering(Init, outputInit);
  stateMachine.SetOnEntering(Bloqueado, outputBloqueado);
  stateMachine.SetOnEntering(MonitorLuz, outputMonitorLuz);
  stateMachine.SetOnEntering(MonitorTemperatura, outputMonitorTemperatura);
  stateMachine.SetOnEntering(AlertaLed, outputAlertaLed);
  stateMachine.SetOnEntering(AlertaBuzzer, outputAlertaBuzzer);
}

/**
 * @brief Configuración inicial del programa al arrancar.
 *
 * Esta función realiza la inicialización de componentes y configuraciones esenciales
 * al iniciar el programa, incluyendo la configuración del sistema de seguridad, el buzzer,
 * y la configuración de la máquina de estados.
 */
void setup() {
  // SISTEMA DE SEGURIDAD
  lcd.begin(16, 2);
  pinMode(red, OUTPUT);   // Configurar el pin del LED rojo como salida
  pinMode(green, OUTPUT);   // Configurar el pin del LED verde como salida
  pinMode(blue, OUTPUT);  // Configurar el pin del LED azul como salida

  // BUZZER
  pinMode(buzzer, OUTPUT); // Configurar el pin del buzzer como salida

  // MAQUINA DE ESTADOS
  setupStateMachine();
  // Asignar condición inicial al estado de inicialización
  stateMachine.SetState(Init, false, true);
}



/**
 * @brief Función principal que se ejecuta repetidamente.
 *
 * En cada iteración, esta función actualiza las tareas asíncronas y la máquina de estados.
 * También reinicia la entrada actual de la máquina de estados a "Unknown".
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
 * @brief Función de salida para el estado de inicialización.
 *
 * Esta función se llama al entrar en el estado de inicialización.
 * Realiza las acciones necesarias para configurar la interfaz de usuario, iniciar tareas y otros procesos asociados al inicio.
 */

//FUNCIONES DE SALIDA PARA LOS ESTADOS
void outputInit() {
  lcd.clear();
  lcd.print("Ingrese clave: ");
  lcd.setCursor(0, 1);

  esperar10s.Start();
  taskSistemaDeSeguridad.Start();
}

/**
 * @brief Función de salida para el estado bloqueado.
 *
 * Esta función se llama al entrar en el estado bloqueado. Detiene las tareas de espera y sistema de seguridad,
 * y luego inicia las tareas de bloqueo con un temporizador de 5 segundos.
 */
void outputBloqueado() {
  esperar10s.Stop();
  taskSistemaDeSeguridad.Stop();

  esperar5s.Start();
  taskBloqueo.Start();
}

/**
 * @brief Función de salida para el estado de monitoreo de luz.
 *
 * Esta función se llama al entrar en el estado de monitoreo de luz. Detiene las tareas de espera y sistema de seguridad,
 * así como las tareas de monitoreo de temperatura y alerta de baja luz. Luego, inicia la tarea de monitoreo de luz con un temporizador de 3 segundos.
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
 * @brief Función de salida para el estado de monitoreo de temperatura.
 *
 * Esta función se llama al entrar en el estado de monitoreo de temperatura. Detiene las tareas de monitoreo de luz y alerta de alta temperatura,
 * y luego inicia la tarea de monitoreo de temperatura con un temporizador de 6 segundos.
 */
void outputMonitorTemperatura() {
  taskMonitorLuz.Stop();

  taskAlertaAltaTemperatura.Stop();

  esperar6s.Start();
  taskMonitorTemperatura.Start();
}

/**
 * @brief Función de salida para el estado de alerta LED.
 *
 * Esta función se llama al entrar en el estado de alerta LED. Detiene las tareas de monitoreo de luz y alerta de baja luz,
 * y luego inicia la tarea de alerta de baja luz con un temporizador de 4 segundos.
 */
void outputAlertaLed() {
  esperar3s.Stop();
  taskMonitorLuz.Stop();

  esperar4s.Start();
  taskAlertaBajaLuz.Start();
}

/**
 * @brief Función de salida para el estado de alerta Buzzer.
 *
 * Esta función se llama al entrar en el estado de alerta Buzzer. Detiene las tareas de monitoreo de temperatura y alerta de alta temperatura,
 * y luego inicia la tarea de alerta de alta temperatura con un temporizador de 5 segundos.
 */
void outputAlertaBuzzer() {
  esperar6s.Stop();
  taskMonitorTemperatura.Stop();

  esperar5s.Start();
  taskAlertaAltaTemperatura.Start();
}


// ----- FUNCIONES PRINCIPALES -----
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

void monitorLuz(void) {
  int luz = analogRead(photocellPin);

  lcd.clear();
  lcd.print("Luz: ");
  lcd.print(luz);

  if (luz < 40)
    currentInput = LuzMenor40;
}

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

void alertaAltaTemperatura(void) {
  lcd.clear();
  lcd.print("---- ALERTA ----");
  lcd.setCursor(0, 1);
  lcd.print("Alta temperatura");

  tone(buzzer, 1000, 5000); // Activar el tono a una frecuencia de 1000 Hz durante 200 milisegundos
  delay(500);
  noTone(buzzer); // Desactivar el tono en el zumbador
}


// ----- FUNCIONES AUXILIARES -----
void function_TimeOut(void) {
  currentInput = TimeOut;
}


/**
 * @brief Compara dos arreglos de caracteres.
 *
 * Esta función compara dos arreglos de caracteres de longitud especificada.
 *
 * @param[in] longitud Longitud de los arreglos a comparar.
 * @param[in] pal1 Primer arreglo de caracteres.
 * @param[in] pal2 Segundo arreglo de caracteres.
 * @return true si los arreglos son iguales, false en caso contrario.
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