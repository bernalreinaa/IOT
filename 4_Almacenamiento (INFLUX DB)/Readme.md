# 4. Almacenamiento (InfluxDB v2)

Este módulo describe la configuración de la base de datos de series temporales utilizada para almacenar las métricas ambientales del proyecto.

## 📊 Configuración de la Base de Datos

El sistema utiliza **InfluxDB v2** corriendo en el puerto `8086`. La estructura de almacenamiento se define mediante los siguientes parámetros:

* **Organización:** `MiCasa`
* **Bucket:** `Sensores`
* **Token:** Token de acceso seguro con permisos de escritura (generado en la interfaz UI).

## ⚙️ Conexión con Telegraf

Para que los datos fluyan correctamente, el archivo `telegraf.conf` debe incluir el siguiente bloque de salida:

```toml
[[outputs.influxdb_v2]]
  urls = ["[http://127.0.0.1:8086](http://127.0.0.1:8086)"]
  token = "PASTE_YOUR_LONG_TOKEN_HERE"
  organization = "MiCasa"
  bucket = "Sensores"
```

🚀 Puesta en Marcha (Pipeline Completo)
Para iniciar el sistema de almacenamiento y procesamiento, se deben ejecutar los siguientes servicios en la Raspberry Pi:

Broker MQTT: mosquitto -c servidor.conf -v

InfluxDB Engine: influxd

Telegraf Agent: telegraf --config telegraf.conf

🔍 Verificación de Datos
Una vez iniciados los servicios, los datos se pueden previsualizar en el panel de control de InfluxDB (http://localhost:8086):

Acceder a Data Explorer.

Seleccionar el Bucket Sensores.

Filtrar por el measurement calidad_aire.

Visualizar los campos aqi, eco2, tvoc, temperatura y humedad.
