# 2. Transmisión de Datos (MQTT Broker)

Este apartado describe la configuración del bróker de mensajería que actúa como puente entre el hardware de campo y el backend de datos.

## 🚀 Despliegue en Raspberry Pi 5

El servidor utiliza **Mosquitto v2.1.2**. La configuración permite la interoperabilidad entre el protocolo de sensores y el sistema de almacenamiento.

### Comandos de Instalación
```bash
# Actualizar repositorios e instalar
sudo apt update && sudo apt install mosquitto -y

# Habilitar inicio automático al arrancar la Raspberry Pi
sudo systemctl enable mosquitto
