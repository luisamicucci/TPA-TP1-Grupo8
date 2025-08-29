/*
  TP1 - IoT (ESP32) - VERSIÓN 2 ESTABLE
  ====================================================
  Grupo 8: Rodriguez, Amicucci, Reyna, Alvarez Veron, Sciarra
  Materia: Tecnologías para la Automatización (TPA)
  Universidad Tecnológica Nacional (UTN)
  Versión: Arduino IDE v2 - Completamente funcional
  Fecha: Agosto 2025

  DESCRIPCIÓN GENERAL:
  Sistema de monitoreo ambiental con ESP32 que permite visualizar temperatura
  y humedad, controlar LED con PWM, configurar umbrales y activar alarmas.

  FUNCIONALIDADES POR MODO:
  - Modo 1: Visualización de variables ambientales (Temp/Humedad)
  - Modo 2: Control PWM del LED integrado mediante potenciómetro
  - Modo 3: Configuración de humedad mínima deseada (HmD)
  - Modo 4: Configuración de temperatura máxima deseada (TMD)
  - Sistema de alarma: LED externo se activa si T > TMD o H < HmD

  CONFIGURACIÓN DE PINES (ajustables según placa):
  - Sensor DHT22: GPIO 33 (datos)
  - Potenciómetro: GPIO 32 (entrada analógica ADC1)
  - Pulsador de modo: GPIO 19 (pull-up interno activado)
  - Touch Plus: GPIO 13 (sensor capacitivo)
  - Touch Minus: GPIO 4 (sensor capacitivo)
  - LED integrado (PWM): GPIO 2 (LED_BUILTIN del ESP32)
  - LED alarma externo: GPIO 23
  - Display SH1106 I2C: SDA=21, SCL=22, dirección 0x3C

  NOTA: Modificar los #define si su placa tiene configuración diferente
*/
// ===================================================================
// LIBRERÍAS NECESARIAS
// ===================================================================
#include <DHT.h>             // Manejo del sensor DHT22/DHT11
#include <Wire.h>            // Comunicación I2C
#include <Adafruit_GFX.h>    // Librería gráfica base para displays
#include <Adafruit_SH110X.h> // Driver específico para display SH1106
#include <splash.h>          // Pantalla de inicio de Adafruit
// ===================================================================
// CONFIGURACIÓN DEL DISPLAY Y SENSOR
// ===================================================================
// Inicialización del display SH1106 (128x64 píxeles) vía I2C
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire);

// ===================================================================
// DEFINICIÓN DE PINES Y CONSTANTES
// ===================================================================
#define DHTTYPE DHT22 // Tipo de sensor (cambiar a DHT11 si es necesario)
#define DHTPIN 33     // Pin de datos del sensor DHT
#define POT_PIN 32    // Pin analógico del potenciómetro (ADC1)
#define LED_PIN 2     // LED integrado del ESP32 (GPIO2 = LED_BUILTIN)
#define LED_ALARMA 23 // LED externo para alarmas

#define TOUCH_PLUS 13 // Pin capacitivo para incrementar valores
#define TOUCH_MINUS 4 // Pin capacitivo para decrementar valores

// ===================================================================
// VARIABLES GLOBALES DEL SISTEMA
// ===================================================================
int contador = 1;              // Contador de modo actual (1-4)
const short PIN_PULSADOR = 19; // Pin del pulsador para cambio de modo
bool encendido = true;         // Estado general del sistema

// Instancia del objeto DHT para lectura del sensor
DHT dht(DHTPIN, DHTTYPE);

// Control de tiempo para debounce del pulsador
int esperaEntreTransicion = 1000; // Milisegundos de espera entre cambios de modo

// ===================================================================
// CONFIGURACIONES DE UMBRALES (modificables por usuario)
// ===================================================================
float humedadMinDeseada = 40.0; // Humedad mínima antes de activar alarma (%)
float tempMaxDeseada = 30.0;    // Temperatura máxima antes de activar alarma (°C)

// ===================================================================
// FUNCIÓN DE CONFIGURACIÓN INICIAL
// ===================================================================
void setup()
{
  Serial.begin(115200); // Inicializar comunicación serie para debug
  delay(1000);          // Pausa para estabilización del sistema

  // Configuración de pines de entrada y salida
  pinMode(PIN_PULSADOR, INPUT_PULLUP); // Pulsador con resistencia pull-up interna
  pinMode(LED_ALARMA, OUTPUT);         // LED de alarma como salida digital

  // Inicialización del sensor DHT
  dht.begin();

  // Configuración e inicialización del display
  Serial.println(F("Iniciando Display..."));
  displayInit(); // Función personalizada de inicialización
  display.println("INICIO");
  Serial.println(F("Display Iniciado"));
}

// ===================================================================
// BUCLE PRINCIPAL DEL PROGRAMA
// ===================================================================
void loop()
{
  // Lectura del pulsador para cambio de modo
  int lectura = digitalRead(PIN_PULSADOR);
  if (lectura == LOW)
  {                // Pulsador presionado (lógica invertida por pull-up)
    contador += 1; // Incrementar contador de modo
    if (contador > 4)
    { // Si supera modo 4, volver al modo 1
      contador = 1;
    }
    delay(esperaEntreTransicion); // Debounce por software
  }

  // Preparar display para nueva información
  display.clearDisplay();
  display.setCursor(0, 0);

  // Ejecutar función según modo seleccionado
  switch (contador)
  {
  case 1:
    variablesAmbientales(); // Modo 1: Mostrar temp/humedad
    break;
  case 2:
    controlarPWM(); // Modo 2: Control LED con potenciómetro
    break;
  case 3:
    humedadMinima(); // Modo 3: Configurar humedad mínima
    break;
  case 4:
    temperaturaMaxima(); // Modo 4: Configurar temperatura máxima
    break;
  default:
    contador = 0;
    Serial.print("APAGADO \n");
    break;
  }

  verificarAlarma(); // Verificar condiciones de alarma en todos los modos
  display.display(); // Actualizar información en pantalla
}

// ===================================================================
// FUNCIONES DE CADA MODO DE OPERACIÓN
// ===================================================================

/**
 * MODO 1: Visualización de variables ambientales
 * Lee y muestra en pantalla la temperatura y humedad actual del sensor DHT
 */
void variablesAmbientales()
{
  display.print("Modo 1\n");
  float humedad = dht.readHumidity();        // Lectura de humedad relativa (%)
  float temperatura = dht.readTemperature(); // Lectura de temperatura (°C)

  // Mostrar valores en display
  display.print("Humedad: ");
  display.print(humedad);
  display.print(" %\n");
  display.print("Temperatura: ");
  display.print(temperatura);
  display.print("C\n");
  delay(500); // Pausa para estabilidad de lectura
}

/**
 * MODO 2: Control PWM del LED integrado
 * Lee el potenciómetro y ajusta la intensidad del LED proporcionalmente
 */
void controlarPWM()
{
  display.print("Modo 2\n");
  int valor = analogRead(POT_PIN);       // Leer potenciómetro (0-4095 en ESP32)
  int pwm = map(valor, 0, 4095, 0, 255); // Mapear a rango PWM (0-255)
  analogWrite(LED_PIN, pwm);             // Aplicar PWM al LED

  // Mostrar valores en display
  display.print("Valor potenciometro: ");
  display.print(valor);
  display.print("PWM: \n");
  display.println(pwm);
  delay(500);
}

/**
 * MODO 3: Configuración de humedad mínima deseada
 * Permite ajustar el umbral de humedad usando sensores touch
 */
void humedadMinima()
{
  display.print("Modo 3\n");

  // Detectar toque en sensor + (incrementar)
  if (touchRead(TOUCH_PLUS) < 30)
  { // Umbral de detección capacitiva
    humedadMinDeseada += 1;
    delay(500); // Debounce para evitar incrementos múltiples
  }

  // Detectar toque en sensor - (decrementar)
  if (touchRead(TOUCH_MINUS) < 30)
  {
    humedadMinDeseada -= 1;
    delay(500);
  }

  display.print("Humedad minima deseada: ");
  display.print(humedadMinDeseada);
  display.print(" %\n");
  delay(300);
}

/**
 * MODO 4: Configuración de temperatura máxima deseada
 * Permite ajustar el umbral de temperatura usando sensores touch
 */
void temperaturaMaxima()
{
  display.print("Modo 4\n");

  // Detectar toque en sensor + (incrementar)
  if (touchRead(TOUCH_PLUS) < 30)
  {
    tempMaxDeseada += 1;
    delay(500);
  }

  // Detectar toque en sensor - (decrementar)
  if (touchRead(TOUCH_MINUS) < 30)
  {
    tempMaxDeseada -= 1;
    delay(500);
  }

  display.print("Temperatura maxima deseada: ");
  display.print(tempMaxDeseada);
  display.print("C\n");
  delay(300);
}

// ===================================================================
// FUNCIONES DE SISTEMA Y UTILIDADES
// ===================================================================

/**
 * Función de verificación de alarma
 * Evalúa las condiciones ambientales contra los umbrales configurados
 * Activa LED externo si se detectan condiciones críticas
 */
void verificarAlarma()
{
  float h = dht.readHumidity();    // Lectura actual de humedad
  float t = dht.readTemperature(); // Lectura actual de temperatura

  // Verificar si alguna condición crítica se cumple
  if (h < humedadMinDeseada || t > tempMaxDeseada)
  {
    digitalWrite(LED_ALARMA, HIGH); // Encender LED de alarma
    display.print("¡ALERTA! Valores fuera de rango.\n");
  }
  else
  {
    digitalWrite(LED_ALARMA, LOW); // Apagar LED de alarma
  }
}

/**
 * Función de inicialización del display
 * Configura el display SH1106 y muestra pantalla de bienvenida con información del sistema
 */
void displayInit()
{
  display.begin(0x3C, true);          // Iniciar display en dirección I2C 0x3C
  display.clearDisplay();             // Limpiar buffer del display
  display.setTextSize(1);             // Tamaño de fuente pequeño
  display.setTextColor(SH110X_WHITE); // Color de texto blanco
  display.setCursor(0, 0);            // Posicionar cursor en esquina superior izquierda

  // Mostrar información del proyecto
  display.println(F("*TP 1*"));
  display.println(F("Datos:"));
  display.printf("Fecha sistema: %s\n", _DATE_); // Fecha de compilación
  display.printf("Hora sistema: %s\n", _TIME_);  // Hora de compilación
  display.display();                             // Enviar buffer al display
}