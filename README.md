# TP1 IoT – ESP32 (Grupo 8)

Este repositorio contiene **tres versiones** del trabajo práctico de IoT con ESP32:

1. **ArduinoIDE/** – Primera versión del sketch para Arduino IDE
2. **ArduinoIDE-v2/** – **Versión estable y funcional** ⭐ (recomendada)
3. **Wokwi-VSCode/** – Proyecto para simulación en Wokwi

> **NOTA:** Se recomienda usar la **versión v2** ya que está completamente probada y funciona sin errores de compilación.

---

## 📋 Tabla de Modos de Operación

| Modo | Función | Descripción | Controles |
|------|---------|-------------|-----------|
| **1** | Variables Ambientales | Muestra temperatura y humedad en tiempo real | Solo visualización |
| **2** | Control PWM LED | Controla intensidad del LED integrado | Potenciómetro |
| **3** | Config. Humedad Mínima | Ajusta umbral de humedad para alarma | Touch +/- |
| **4** | Config. Temperatura Máxima | Ajusta umbral de temperatura para alarma | Touch +/- |

**Sistema de Alarma:** LED externo se activa automáticamente si `Temperatura > Máxima` **O** `Humedad < Mínima`

---

## 🔌 Tabla de Conexiones (Pines)

| Componente | Pin GPIO | Tipo | Descripción |
|------------|----------|------|-------------|
| **Sensor DHT22** | 33 | Digital | Datos del sensor temp/humedad |
| **Potenciómetro** | 32 | Analógico (ADC1) | Control de intensidad LED |
| **Pulsador Modo** | 19 | Digital (Pull-up) | Cambio entre modos 1-4 |
| **Touch Plus (+)** | 13 | Capacitivo | Incrementar valores config. |
| **Touch Minus (-)** | 4 | Capacitivo | Decrementar valores config. |
| **LED Integrado** | 2 | Digital PWM | LED controlado por potenciómetro |
| **LED Alarma** | 23 | Digital | Indicador de condiciones críticas |
| **Display SH1106** | 21, 22 | I2C (SDA, SCL) | Pantalla OLED 128x64, addr 0x3C |

---

## 🔧 Compilación en Arduino IDE (Versión v2 recomendada)

### Prerrequisitos
1. **Arduino IDE 2.x** instalado
2. **Soporte para ESP32** instalado vía Boards Manager
3. **Librerías necesarias** desde Library Manager:
   - `DHT sensor library` (by Adafruit)
   - `Adafruit Unified Sensor` (dependencia de DHT)
   - `Adafruit SH110X` (para display SH1106)
   - `Adafruit GFX Library` (librería gráfica base)

### Pasos de compilación
1. Abrir `ArduinoIDE-v2/sketch_grupo_08/sketch_grupo_08.ino`
2. Seleccionar placa: **ESP32 Dev Module** o equivalente
3. Configurar puerto serie correspondiente
4. Compilar y cargar

> **IMPORTANTE:** La versión v2 usa display **SH1106** en lugar de SSD1306. Asegúrese de tener la librería correcta instalada.

---

## 🎮 Simulación en Wokwi (VS Code)

### Configuración
1. Instalar extensión **Wokwi** para VS Code
2. Abrir carpeta `Wokwi-VSCode/`
3. El archivo `diagram.json` incluye:
   - ESP32 WROOM-32
   - Sensor DHT22
   - Display OLED SSD1306 128x64
   - Potenciómetro
   - 2 botones (simulan Touch +/-)
   - Pulsador de modo
   - 2 LEDs (PWM y alarma)

### Ejecutar simulación
1. Abrir `sketch.ino` en VS Code
2. Hacer clic en el ícono ▶️ de Wokwi
3. La simulación iniciará automáticamente

> **NOTA:** En simulación, los sensores **Touch capacitivos** se emulan con **botones físicos** para facilitar las pruebas.

---

## 📖 Guía de Uso

### Operación básica
1. **Encender** el dispositivo - aparece pantalla de inicio con fecha/hora de compilación
2. **Presionar pulsador** para cambiar entre modos (cicla 1→2→3→4→1...)
3. **Usar sensores touch** en modos 3 y 4 para ajustar umbrales
4. **Girar potenciómetro** en modo 2 para controlar LED
5. **Monitorear LED de alarma** - se activa automáticamente ante condiciones críticas

### Valores por defecto
- **Humedad mínima:** 40% (configurable en modo 3)
- **Temperatura máxima:** 30°C (configurable en modo 4)
- **Sensibilidad touch:** < 30 (valor capacitivo)

---

## 🔧 Personalización y Configuración

### Modificar pines (si su placa es diferente)
Editar las siguientes líneas en el código:
```cpp
#define DHTPIN 33               // Pin del sensor DHT
#define POT_PIN 32              // Pin del potenciómetro  
#define PIN_PULSADOR 19         // Pin del pulsador de modo
#define TOUCH_PLUS 13           // Pin touch +
#define TOUCH_MINUS 4           // Pin touch -
#define LED_PIN 2               // LED integrado
#define LED_ALARMA 23           // LED de alarma
```

### Cambiar tipo de sensor DHT
```cpp
#define DHTTYPE DHT22           // Cambiar a DHT11 si es necesario
```

### Ajustar sensibilidad touch
```cpp
if (touchRead(TOUCH_PLUS) < 30) {    // Cambiar valor 30 según necesidad
```

---

## 📁 Estructura del Proyecto

```
TPA-TP1-Grupo8/
├── README.md                        # Este archivo
├── txt_grupo_08.txt                 # Información del grupo
├── LIBS_Arduino.txt                 # Lista de librerías utilizadas
├── ArduinoIDE/                      # Versión inicial
│   └── sketch_grupo_08/
│       └── sketch_grupo_08.ino
├── ArduinoIDE-v2/                   # ⭐ Versión estable recomendada
│   └── sketch_grupo_08/
│       └── sketch_grupo_08.ino
└── Wokwi-VSCode/                    # Versión para simulación
    ├── diagram.json                 # Configuración del circuito
    ├── sketch.ino                   # Código para Wokwi
    └── wokwi.toml                   # Configuración del proyecto
```

---

## 🐛 Resolución de Problemas

### Error de compilación "ledcSetup not declared"
- Verificar que la placa seleccionada sea ESP32
- Asegurar que el paquete ESP32 esté correctamente instalado

### Display no funciona
- Verificar conexiones I2C (SDA=21, SCL=22)
- Confirmar dirección I2C (0x3C para SH1106)
- Verificar que la librería Adafruit_SH110X esté instalada

### Sensores touch no responden
- En hardware real: verificar conexiones a GPIO 13 y 4
- Ajustar umbral de sensibilidad (valor < 30)
- En simulación: usar los botones en lugar de touch

### Sensor DHT no lee valores
- Verificar conexión a GPIO 33
- Verificar tipo de sensor (DHT22 vs DHT11)
- Esperar al menos 2 segundos entre lecturas

---

## 📚 Archivos de Entrega

| Archivo | Ubicación | Descripción |
|---------|-----------|-------------|
| **Código fuente principal** | `ArduinoIDE-v2/sketch_grupo_08/sketch_grupo_08.ino` | Versión estable y documentada |
| **Información del grupo** | `txt_grupo_08.txt` | Datos de integrantes |
| **Lista de librerías** | `LIBS_Arduino.txt` | Dependencias del proyecto |
| **Documentación** | `README.md` | Este archivo con instrucciones completas |

---

## 👥 Integrantes (Grupo 8)

| Nombre | Legajo |
|--------|--------|
| Rodriguez, Melina Macarena | 86679 |
| Amicucci, Luis Alberto | 59739 |
| Reyna, Juan Ignacio | 89336 |
| Alvarez Veron, Mateo | 85927 |
| Sciarra, Martin | 69650 |

---

## 📄 Licencia y Uso Académico

Este proyecto fue desarrollado como parte del curso de IoT en UTN. 
Se permite su uso con fines educativos citando apropiadamente a los autores.

**Fecha de desarrollo:** Agosto 2025  
**Institución:** Universidad Tecnológica Nacional (UTN)  
**Materia:** Trabajo Práctico de Aplicaciones (TPA) - IoT