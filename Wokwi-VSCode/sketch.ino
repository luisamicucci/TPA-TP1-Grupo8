/* 
  TP1 - IoT (ESP32) - Wokwi (VS Code)
  Grupo 8: Rodriguez, Amicucci, Reyna, Alvarez Veron, Sciarra
  ------------------------------------------------------------
  Igual que la versión Arduino, pero permite SIMULAR los "touch" con botones
  si se define WOKWI_SIM_TOUCH. En ese modo, los GPIO13 y GPIO4 se leen como
  botones (INPUT_PULLUP) en lugar de touchRead().
*/

#define WOKWI_SIM_TOUCH 1  // <<-- Dejar en 1 para simular touch con botones en Wokwi

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ======== Pines ========
#define PIN_DHT         33      // DHT data
#define PIN_POT         32      // ADC1 (pot)
#define PIN_BUTTON      19      // Pulsador modos (pull-up)
#define PIN_TOUCH_PLUS  13      // Touch +  (en Wokwi: botón a GND si WOKWI_SIM_TOUCH=1)
#define PIN_TOUCH_MINUS 4       // Touch -  (en Wokwi: botón a GND si WOKWI_SIM_TOUCH=1)
#define PIN_LED_INT     23      // LED integrado (PWM) - simulado como LED externo en el diagrama
#define PIN_LED_ALARM   14      // LED EXTERNO (alarma)

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define DHTTYPE DHT22
DHT dht(PIN_DHT, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// PWM (LEDC)
const int LEDC_CHANNEL = 0;
const int LEDC_FREQ    = 5000;
const int LEDC_RES     = 8;

// Modos y umbrales
volatile uint8_t modo = 1;
float HmD = 40.0;
float TMD = 30.0;
const float HmD_min = 20.0, HmD_max = 80.0;
const float TMD_min = 10.0, TMD_max = 60.0;

float lastTemp = NAN, lastHum = NAN;
unsigned long lastDhtMs = 0;
const unsigned long dhtPeriodMs = 2000;

unsigned long lastBtnMs = 0;
const unsigned long btnDebounce = 200;

#if !WOKWI_SIM_TOUCH
unsigned long lastTouchPlusMs = 0, lastTouchMinusMs = 0;
const unsigned long touchDebounce = 200;
uint16_t basePlus = 0, baseMinus = 0;
const float touchRatio = 0.7;
#else
unsigned long lastPlusBtnMs = 0, lastMinusBtnMs = 0;
const unsigned long plusMinusDebounce = 150;
#endif

void IRAM_ATTR onButtonISR() {
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
  int potRaw = analogRead(PIN_POT);
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

#if !WOKWI_SIM_TOUCH
bool touchPressed(uint8_t pin, uint16_t base, unsigned long &lastMs) {
  uint16_t val = touchRead(pin);
  unsigned long now = millis();
  if (val < (uint16_t)(base * touchRatio) && (now - lastMs) > touchDebounce) {
    lastMs = now;
    return true;
  }
  return false;
}
#else
bool buttonPressed(uint8_t pin, unsigned long &lastMs) {
  unsigned long now = millis();
  if (digitalRead(pin) == LOW && (now - lastMs) > plusMinusDebounce) {
    // mantener hasta soltar
    while (digitalRead(pin) == LOW) { delay(5); }
    lastMs = now;
    return true;
  }
  return false;
}
#endif

void showMode3() {
  drawHeader("Modo 3 - Config HmD");
#if !WOKWI_SIM_TOUCH
  static unsigned long lastPlus = 0, lastMinus = 0;
  if (touchPressed(PIN_TOUCH_PLUS, basePlus, lastPlus))  HmD = min(HmD + 1.0, HmD_max);
  if (touchPressed(PIN_TOUCH_MINUS, baseMinus, lastMinus)) HmD = max(HmD - 1.0, HmD_min);
#else
  if (buttonPressed(PIN_TOUCH_PLUS, lastPlusBtnMs))  HmD = min(HmD + 1.0, HmD_max);
  if (buttonPressed(PIN_TOUCH_MINUS, lastMinusBtnMs)) HmD = max(HmD - 1.0, HmD_min);
#endif
  display.setCursor(0, 16);
  display.print("HmD (min %): "); display.println(HmD, 1);
#if WOKWI_SIM_TOUCH
  display.println("GPIO13: +  | GPIO4: -");
#else
  display.println("Touch + (GPIO13) suma");
  display.println("Touch - (GPIO4)  resta");
#endif
  display.display();
}

void showMode4() {
  drawHeader("Modo 4 - Config TMD");
#if !WOKWI_SIM_TOUCH
  static unsigned long lastPlus = 0, lastMinus = 0;
  if (touchPressed(PIN_TOUCH_PLUS, basePlus, lastPlus))  TMD = min(TMD + 0.5, TMD_max);
  if (touchPressed(PIN_TOUCH_MINUS, baseMinus, lastMinus)) TMD = max(TMD - 0.5, TMD_min);
#else
  if (buttonPressed(PIN_TOUCH_PLUS, lastPlusBtnMs))  TMD = min(TMD + 0.5, TMD_max);
  if (buttonPressed(PIN_TOUCH_MINUS, lastMinusBtnMs)) TMD = max(TMD - 0.5, TMD_min);
#endif
  display.setCursor(0, 16);
  display.print("TMD (max C): "); display.println(TMD, 1);
#if WOKWI_SIM_TOUCH
  display.println("GPIO13: +  | GPIO4: -");
#else
  display.println("Touch + (GPIO13) suma");
  display.println("Touch - (GPIO4)  resta");
#endif
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

    bool alarm = false;
    if (!isnan(lastTemp) && lastTemp > TMD) alarm = true;
    if (!isnan(lastHum) && lastHum < HmD) alarm = true;
    digitalWrite(PIN_LED_ALARM, alarm ? HIGH : LOW);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("TP1 IoT - Wokwi");
  display.display();

  dht.begin();

  ledcSetup(LEDC_CHANNEL, LEDC_FREQ, LEDC_RES);
  ledcAttachPin(PIN_LED_INT, LEDC_CHANNEL);
  pinMode(PIN_LED_ALARM, OUTPUT);
  digitalWrite(PIN_LED_ALARM, LOW);

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON), onButtonISR, FALLING);

#if !WOKWI_SIM_TOUCH
  delay(500);
  // Calibración baseline touch
  uint16_t s1=0, s2=0;
  for (int i=0;i<10;i++){ s1+=touchRead(PIN_TOUCH_PLUS); s2+=touchRead(PIN_TOUCH_MINUS); delay(20); }
  basePlus = s1/10;
  baseMinus = s2/10;
#else
  // Configurar pines touch como botones (pull-up). Conectar a GND en el diagrama.
  pinMode(PIN_TOUCH_PLUS, INPUT_PULLUP);
  pinMode(PIN_TOUCH_MINUS, INPUT_PULLUP);
#endif
}

void loop() {
  readDHTIfNeeded();

  switch (modo) {
    case 1: showMode1(); break;
    case 2: showMode2(); break;
    case 3: showMode3(); break;
    case 4: showMode4(); break;
  }
  delay(30);
}
