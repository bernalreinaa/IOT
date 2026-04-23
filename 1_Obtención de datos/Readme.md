# 1. Obtención de Datos (Node Sensor ESP32-S3)

Este módulo constituye la capa de hardware y adquisición primaria del proyecto. Se encarga de la captura de variables ambientales, el procesamiento local y la preparación de los datos para su posterior transmisión mediante un flujo de trabajo optimizado.

## 🛠️ Hardware Utilizado

| Componente | Función | Protocolo / Interfaz |
| :--- | :--- | :--- |
| **ESP32-S3** | Microcontrolador central (Cerebro del nodo) | Wi-Fi / I2C / PWM |
| **ScioSense ENS160** | Sensor digital multi-gas (AQI, eCO2, TVOC) | I2C (Dirección 0x52) |
| **Sensirion SHT40** | Sensor de alta precisión (Temp / Humedad) | I2C (Dirección 0x44) |
| **LED RGB** | Indicador visual de estado AQI | PWM (Control analógico) |
| **Micro SD Card** | Almacenamiento local de respaldo (Datalogger) | SPI |

## 📂 Estructura de la Carpeta
* `Esp32_Wifi_nodos`: Código fuente principal desarrollado en el ecosistema Arduino.


## 🔧 Configuración del Entorno

### Librerías Requeridas
Es necesario instalar las siguientes dependencias en Arduino IDE o PlatformIO:
* `ScioSense_ENS16x` por ScioSense
* `SensirionI2cSht4x` por Sensirion
* `PubSubClient` por Nick O'Leary
* `Wire`, `WiFi`, `SPI`, `SD` (Librerías nativas de ESP32)

### Definición de Pines (Pinout)
```cpp
#define I2C_SDA       8
#define I2C_SCL       9
#define RED_LED_PIN   4
#define GREEN_LED_PIN 5
#define BLUE_LED_PIN  6
#define SD_CS_PIN     10
#define ENS160_PWR    45 // Pin de alimentación del sensor
