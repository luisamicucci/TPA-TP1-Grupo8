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

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ======== Pines (ajustar según placa) ========
#define PIN_DHT         33      // DHT data
#define PIN_POT         32      // ADC1 (pot)
#define PIN_BUTTON      19      // Pulsador modos (pull-up)
#define PIN_TOUCH_PLUS  13      // Touch +
#define PIN_TOUCH_MINUS 4       // Touch -
#define PIN_LED_INT     23      // LED integrado (PWM)
#define PIN_LED_ALARM   14      // LED EXTERNO (alarma) - usar el LED externo de la placa, NO el integrado

// I2C (SSD1306): SDA=21, SCL=22
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ======== DHT ========
#define DHTTYPE DHT22   // Cambiar a DHT11 si es necesario
DHT dht(PIN_DHT, DHTTYPE);

// ======== OLED ========
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ======== PWM (LEDC) ========
const int LEDC_CHANNEL = 0;
const int LEDC_FREQ    = 5000; // 5kHz
const int LEDC_RES     = 8;    // 8 bits (0..255)

// ======== Lógica de modos y umbrales ========
volatile uint8_t modo = 1;   // 1..4
float HmD = 40.0;            // Humedad mínima deseada (%)
float TMD = 30.0;            // Temp. máxima deseada (°C)
const float HmD_min = 20.0, HmD_max = 80.0;
const float TMD_min = 10.0, TMD_max = 60.0;

// Lecturas y tiempos
float lastTemp = NAN, lastHum = NAN;
unsigned long lastDhtMs = 0;
const unsigned long dhtPeriodMs = 2000;

// Debounce / toque
unsigned long lastBtnMs = 0;
const unsigned long btnDebounce = 200; // ms
unsigned long lastTouchPlusMs = 0, lastTouchMinusMs = 0;
const unsigned long touchDebounce = 200; // ms

// Calibración touch (baseline)
uint16_t basePlus = 0, baseMinus = 0;
const float touchRatio = 0.7; // Dispara si lectura < 70% del baseline

void IRAM_ATTR onButtonISR() {
  // Interrupción por flanco - se usa un debounce por tiempo
  unsigned long now = millis();
  if (now - lastBtnMs > btnDebounce) {
    modo++;
    if (modo > 4) modo = 1;
    lastBtnMs = now;
  }
}

void drawHeader(const char* title) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(title);
  display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
}

void showMode1() {
  // Mostrar T/H
  drawHeader("Modo 1 - Ambiente");
  display.setCursor(0, 16);
  if (isnan(lastTemp) || isnan(lastHum)) {
    display.println("Sensor DHT: ERROR");
  } else {
    display.print("Temp: "); display.print(lastTemp, 1); display.println(" C");
    display.print("Hum : "); display.print(lastHum, 1); display.println(" %");
    display.println();
    display.print("TMD: "); display.print(TMD, 1); display.println(" C");
    display.print("HmD: "); display.print(HmD, 1); display.println(" %");
  }
  display.display();
}

void showMode2() {
  // Mostrar brillo LED y pot
  int potRaw = analogRead(PIN_POT);        // 0..4095
  int pwmVal = map(potRaw, 0, 4095, 0, 255);
  ledcWrite(LEDC_CHANNEL, pwmVal);

  drawHeader("Modo 2 - LED PWM");
  display.setCursor(0, 16);
  display.print("Pot: "); display.print(potRaw);
  display.println(" (0..4095)");
  display.print("PWM: "); display.print(pwmVal);
  display.println(" (0..255)");
  display.display();
}

bool touchPressed(uint8_t pin, uint16_t base, unsigned long &lastMs) {
  // Un toque se considera cuando la lectura cae por debajo de base*touchRatio
  uint16_t val = touchRead(pin);
  unsigned long now = millis();
  if (val < (uint16_t)(base * touchRatio) && (now - lastMs) > touchDebounce) {
    lastMs = now;
    return true;
  }
  return false;
}

void showMode3() {
  // Configurar Humedad mínima
  drawHeader("Modo 3 - Config HmD");
  // Detectar toques
  if (touchPressed(PIN_TOUCH_PLUS, basePlus, lastTouchPlusMs)) {
    HmD = min(HmD + 1.0, HmD_max);
  }
  if (touchPressed(PIN_TOUCH_MINUS, baseMinus, lastTouchMinusMs)) {
    HmD = max(HmD - 1.0, HmD_min);
  }

  display.setCursor(0, 16);
  display.print("HmD (min %): "); display.println(HmD, 1);
  display.println("Touch + (GPIO13) suma");
  display.println("Touch - (GPIO4)  resta");
  display.display();
}

void showMode4() {
  // Configurar Temp maxima
  drawHeader("Modo 4 - Config TMD");
  if (touchPressed(PIN_TOUCH_PLUS, basePlus, lastTouchPlusMs)) {
    TMD = min(TMD + 0.5, TMD_max);
  }
  if (touchPressed(PIN_TOUCH_MINUS, baseMinus, lastTouchMinusMs)) {
    TMD = max(TMD - 0.5, TMD_min);
  }

  display.setCursor(0, 16);
  display.print("TMD (max C): "); display.println(TMD, 1);
  display.println("Touch + (GPIO13) suma");
  display.println("Touch - (GPIO4)  resta");
  display.display();
}

void readDHTIfNeeded() {
  unsigned long now = millis();
  if (now - lastDhtMs >= dhtPeriodMs) {
    lastDhtMs = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h)) lastHum = h;
    if (!isnan(t)) lastTemp = t;

    // Evaluar alarma
    bool alarm = false;
    if (!isnan(lastTemp) && lastTemp > TMD) alarm = true;
    if (!isnan(lastHum) && lastHum < HmD) alarm = true;
    digitalWrite(PIN_LED_ALARM, alarm ? HIGH : LOW);
  }
}

void setup() {
  Serial.begin(115200);

  // I2C + OLED
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // Si falla, continuar sin display
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("TP1 IoT - ESP32");
  display.display();

  // DHT
  dht.begin();

  // LED PWM (integrado)
  ledcSetup(LEDC_CHANNEL, LEDC_FREQ, LEDC_RES);
  ledcAttachPin(PIN_LED_INT, LEDC_CHANNEL);

  // LED alarma (externo)
  pinMode(PIN_LED_ALARM, OUTPUT);
  digitalWrite(PIN_LED_ALARM, LOW);

  // Botón de modo
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON), onButtonISR, FALLING);

  // Calibración touch (tomar baseline)
  delay(500);
  basePlus  = touchRead(PIN_TOUCH_PLUS);
  baseMinus = touchRead(PIN_TOUCH_MINUS);

  // Mensaje inicial
  delay(800);
}

void loop() {
  readDHTIfNeeded();

  switch (modo) {
    case 1: showMode1(); break;
    case 2: showMode2(); break;
    case 3: showMode3(); break;
    case 4: showMode4(); break;
  }

  delay(30); // Pequeño respiro para el sistema
}
