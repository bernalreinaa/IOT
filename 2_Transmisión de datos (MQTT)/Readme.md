# 2. Transmisión de Datos (MQTT Broker)
Este módulo describe la implementación del bróker de mensajería, el cual actúa como el nodo central de comunicaciones (Hub) en la arquitectura del proyecto. Su función principal es recibir los datos telemétricos del ESP32-S3 y servirlos en tiempo real a los diferentes suscriptores del sistema (Telegraf, bases de datos e interfaces OPC UA).

## 🚀 Despliegue en Raspberry Pi 5
Para este proyecto se ha seleccionado la Raspberry Pi 5 como servidor dedicado. Se utiliza Eclipse Mosquitto v2.1.2, un bróker de código abierto ligero y eficiente, considerado el estándar de facto en soluciones de IoT e Industria 4.0.

⚙️ Instalación y Configuración
El despliegue se realiza mediante la gestión de paquetes del sistema operativo y la configuración del demonio para permitir tráfico externo:


## 1. Actualizar repositorios e instalar el bróker y clientes
```Bash
sudo apt update && sudo apt install mosquitto mosquitto-clients -y
```
## 2. Habilitar el inicio automático del servicio al arrancar el sistema
sudo systemctl enable mosquitto
Para asegurar la interoperabilidad en la red local privada, el archivo de configuración (mosquitto.conf) se ha ajustado con los siguientes parámetros:

Puerto de escucha: 1883

Acceso: allow_anonymous true (Optimizado para entorno de desarrollo y pruebas).

Interfaz: Escucha activa en todas las interfaces de red para permitir la conexión del ESP32.

📊 Protocolo y Formato del Mensaje
Los datos se transmiten siguiendo una estructura optimizada para su posterior ingesta en bases de datos temporales (InfluxDB Line Protocol). La cadena de texto sigue este esquema:

Formato: nombre_medicion campo1=valor1,campo2=valor2

Ejemplo real: calidad_aire temperatura=24.5,humedad=40.2,aqi=1,eco2=400

🔍 Verificación y Monitorización
Para validar que la cadena de datos desde el hardware de campo hasta el servidor es correcta, se utiliza el cliente de suscripción integrado directamente en la terminal de la Raspberry Pi:

# Suscribirse al tópico de sensores en modo detallado (verbose) para ver el flujo en tiempo real
```Bash
mosquitto_sub -h localhost -t "esp32/sensores" -v
```
Este comando permite confirmar que el bróker está operando como puente de datos de forma estable antes de proceder a la fase de visualización en Grafana.
