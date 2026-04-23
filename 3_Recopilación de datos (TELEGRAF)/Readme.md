# 3. Recopilación de Datos (Telegraf Agent)

Este módulo describe la configuración de **Telegraf**, el agente encargado de actuar como puente entre el flujo de mensajería MQTT y el almacenamiento persistente.

## ⚙️ Configuración del Agente (`telegraf.conf`)

El sistema utiliza una configuración optimizada para minimizar la latencia y asegurar la integridad de las métricas ambientales.

### Bloque de Entrada: MQTT Consumer
Telegraf se suscribe al bróker local para capturar los datos procedentes del ESP32-S3:

```toml
[[inputs.mqtt_consumer]]
  servers = ["tcp://127.0.0.1:1883"]
  topics = ["esp32/sensores"]
  data_format = "influx"

```
  Bloque de Salida: Depuración
Para validar el flujo antes del almacenamiento definitivo, se utiliza la salida por consola:

```toml
[[outputs.file]]
  files = ["stdout"]
```

🚀 Ejecución y Verificación
Para poner en marcha el agente en la Raspberry Pi 5, se utiliza el siguiente comando:

```bash
telegraf --config telegraf.conf
```


Confirmación de Funcionamiento
Una ejecución exitosa mostrará en la terminal el flujo de datos procesado:

Conexión: [inputs.mqtt_consumer] Connected [tcp://127.0.0.1:1883]

Datos: calidad_aire,host=RPi5,topic=esp32/sensores temperatura=32.38,humedad=23.92,aqi=4...

🛠️ Notas Técnicas
Versión utilizada: Telegraf v1.38.3.

Formato de datos: Influx Line Protocol (Nativo).

Frecuencia de recolección: 10 segundos (ajustable en la sección [agent]).


### 💡PlatformIO:
Para que mediante PlatformIO  se manden los datos a Mosquitto correctamente, debes añadir la librería **PubSubClient** a tu archivo `platformio.ini`. 

```ini
lib_deps = 
    adafruit/Adafruit BusIO
    sciosense/ScioSense_ENS16x@^2.0.3
    sensirion/Sensirion I2C SHT4x@^1.1.2
    knolleary/PubSubClient@^2.8
