# TP1 IoT – ESP32 (Grupo 8)

Este paquete contiene **dos versiones** del trabajo práctico:

1) **ArduinoIDE/** – Sketch listo para compilar/cargar en Arduino IDE.
2) **Wokwi-VSCode/** – Proyecto para el simulador Wokwi usando la extensión de VS Code.

---

## Requerimientos de hardware (según placa de la cátedra)
- Sensor DHT (DHT22 por defecto; si usan DHT11, cambien `#define DHTTYPE DHT22` a `DHT11`).
- Display I2C SSD1306 128x64 (addr 0x3C) en SDA=21, SCL=22.
- Potenciómetro a **GPIO 32** (ADC1).
- Pulsador de modo en **GPIO 19** (con `INPUT_PULLUP`).
- Touch + en **GPIO 13** y Touch – en **GPIO 4** (capacitivos).
- LED integrado (PWM) en **GPIO 23**.
- LED EXTERNO de alarma en **GPIO 14** (ajustar si su placa usa otro).

> Estos pines salen del apunte de pines provisto. Si su placa tiene diferencias, solo editen los `#define` al comienzo del sketch.

---

## Compilación en Arduino IDE
1. Instalar librerías desde el **Library Manager**:
   - *DHT sensor library* (by Adafruit)
   - *Adafruit Unified Sensor* (dep. de DHT)
   - *Adafruit SSD1306*
   - *Adafruit GFX Library*
2. Abrir `ArduinoIDE/sketch_grupo_08.ino`.
3. Seleccionar Board: **ESP32 Dev Module** o la correspondiente a su placa.
4. Cargar.

---

## Simulación en Wokwi (VS Code)
1. Instalar la extensión **Wokwi** para VS Code.
2. Abrir la carpeta `Wokwi-VSCode/`.
3. Archivo principal: `sketch.ino` (mismo código, con `#define WOKWI_SIM_TOUCH 1`).
4. El `diagram.json` ya trae: ESP32, DHT22, OLED, potenciómetro, 2 botones simulando Touch (+/-), botón de modo y dos LEDs (PWM y alarma).
5. Ejecutar Simulación (ícono ▶️ de Wokwi).

> En simulación, los **Touch** se emulan con **botones** (GPIO13=+, GPIO4=−). En hardware real se usan los pines capacitivos.

---

## Modo de operación (pulsador de modo)
- **Modo 1:** Muestra temperatura y humedad en tiempo real.
- **Modo 2:** Controla brillo del LED integrado con el potenciómetro.
- **Modo 3:** Configura **HmD** (Humedad mínima). Touch + suma / Touch − resta.
- **Modo 4:** Configura **TMD** (Temperatura máxima). Touch + suma / Touch − resta.

**Alarma:** si `Temp > TMD` **o** `Hum < HmD`, se enciende el **LED EXTERNO** (no el integrado).

---

## Archivos solicitados por la cátedra
- **Código fuente:** `sketch_grupo_08.ino`
- **TXT del grupo:** `txt_grupo_08.txt` (incluido en la raíz de este paquete)

---

## Créditos (Grupo 8)
Rodriguez Melina Macarena - 86679  
Amicucci Luis Alberto - 59739  
Reyna Juan Ignacio - 89336  
Alvarez Veron, Mateo - 85927  
Sciarra, Martin - 69650