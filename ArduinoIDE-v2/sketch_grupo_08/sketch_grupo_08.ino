/*
  TP1 - IoT (ESP32)
  Grupo 8: Rodriguez, Amicucci, Reyna, Alvarez Veron, Sciarra
  Versión: Arduino IDE
  ------------------------------------------------------------
  Funcionalidad:
  - Modo 1: Mostrar Temp/Humedad (DHT) en OLED
  - Modo 2: Control PWM del LED integrado con potenciómetro
  - Modo 3: Configurar Humedad mínima (HmD) con Touch + / Touch -
  - Modo 4: Configurar Temperatura máxima (TMD) con Touch + / Touch -
  - Alarma: Si T > TMD o H < HmD, enciende LED EXTERNO (NO el integrado)

  Notas importantes (ajustables según la placa de cátedra):
  - DHT en GPIO 33
  - Potenciómetro en GPIO 32 (ADC1)
  - Pulsador de modo en GPIO 19 (pull-up interno)
  - Touch + en GPIO 13  | Touch - en GPIO 4
  - LED integrado (PWM) en GPIO 23
  - LED EXTERNO/Alarma en GPIO 14 (puede ser el pin del relé si está cableado a un LED externo)
  - Display I2C SSD1306 128x64 en SDA=21, SCL=22, addr 0x3C

  Si tu placa usa otros pines, modifícalos en los #define de más abajo.
*/
//Librerias
#include <DHT.h>
#include <Wire.h>
//Configuracion para la visualizacion de la pantalla
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <splash.h>
//#include "config.h"

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire);

#define DHTTYPE DHT22
#define DHTPIN 33 //DHT: SDA conectado al pin 33
#define POT_PIN 32 //Potenciómetro: SIG conectado al pin 32
#define LED_PIN 2 // LED integrado en el microcontrolador ESP32
#define LED_ALARMA 23   // LED externo

#define TOUCH_PLUS 13  // PIN+ que se encuentra en el PIN13
#define TOUCH_MINUS 4 // PIN- que se encuentra en el PIN4

//Variables globales
int contador = 1; //Cuantas veces se ha presionado el botón.
const short PIN_PULSADOR = 19; //Boton conectado al pin 19
bool encendido = true;

//Objeto dht para manejar el sensor.
DHT dht(DHTPIN, DHTTYPE);

//Tiempo de espera entre lecturas de botón, para evitar múltiples conteos rápidos
int esperaEntreTransicion = 1000;

// Configuraciones ajustables
float humedadMinDeseada = 40.0;
float tempMaxDeseada = 30.0;

void setup() {
  Serial.begin(115200); //Inicializa el monitor serial.
  delay(1000);
  pinMode(PIN_PULSADOR, INPUT_PULLUP);  //Configurar pin del boton
  pinMode(LED_ALARMA, OUTPUT); // Configurar el pin de la alarma LED
  dht.begin(); //Inicializa el sensor DHT

  //Inicializar la pantalla
  Serial.println(F("Iniciando Display..."));
  displayInit();
  display.println("INICIO");  
  Serial.println(F("Display Iniciado"));
}

void loop() {
  int lectura = digitalRead(PIN_PULSADOR);
  if (lectura == LOW) {
    contador +=1;
    if(contador>4) {
      contador = 1;
    }
      
    delay(esperaEntreTransicion);
  }

  display.clearDisplay();
  display.setCursor(0,0);
  

  switch (contador) {
    case 1:
      variablesAmbientales();
      break;
    case 2:
      controlarPWM();
      break;
    case 3:
      humedadMinima();
      break;
    case 4:
      temperaturaMaxima();
      break;
    default:
      contador = 0;
      Serial.print("APAGADO \n");
      break;
  }
  verificarAlarma();
  display.display();
  
}

//**Modo 1: Visualizar variables ambientales **
void variablesAmbientales() {
      display.print("Modo 1\n");
      float humedad = dht.readHumidity();
      float temperatura = dht.readTemperature();
      display.print("Humedad: "); display.print(humedad); display.print(" %\n");
      display.print("Temperatura: "); display.print(temperatura); display.print("C\n");
      delay(500);
}

//*Modo 2: Visualizar intensidad del LED y valor del potenciómetro*
void controlarPWM() {
  display.print("Modo 2\n");
  int valor = analogRead(POT_PIN);
  int pwm = map(valor, 0, 4095, 0, 255);
  analogWrite(LED_PIN, pwm);
  display.print("Valor potenciometro: "); display.print(valor);
  display.print("PWM: \n"); display.println(pwm);
  delay(500);
}

//*Modo 3: Configurar humedad mínima deseada*
void humedadMinima() {
  display.print("Modo 3\n");
  if (touchRead(TOUCH_PLUS) < 30) {
    humedadMinDeseada += 1;
    delay(500);
  }
  if (touchRead(TOUCH_MINUS) < 30) {
    humedadMinDeseada -= 1;
    delay(500);
  }
  display.print("Humedad minima deseada: "); display.print(humedadMinDeseada); display.print(" %\n");
  delay(300);
}

//*Modo 4: Configurar temperatura máxima deseada*
void temperaturaMaxima() {
  display.print("Modo 4\n");
  if (touchRead(TOUCH_PLUS) < 30) {
    tempMaxDeseada += 1;
    delay(500);
  }
  if (touchRead(TOUCH_MINUS) < 30) {
    tempMaxDeseada -= 1;
    delay(500);
  }
  display.print("Temperatura maxima deseada: "); display.print(tempMaxDeseada); display.print("C\n");
  delay(300);
}

// Encender LED externo si hay condiciones críticas
void verificarAlarma() { 
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (h < humedadMinDeseada || t > tempMaxDeseada) {
    digitalWrite(LED_ALARMA, HIGH);
    display.print("¡ALERTA! Valores fuera de rango.\n");
  } else {
    digitalWrite(LED_ALARMA, LOW);
  }
}

void displayInit(){
  display.begin(0x3C, true);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);
  display.println(F("*TP 1*"));
  display.println(F("Datos:"));
  display.printf("Fecha sistema: %s\n",_DATE_);
  display.printf("Hora sistema: %s\n",_TIME_);
  display.display();
}