


//---------------------------------------------------------------------------------------------------------------------------------------
//                 INCLUDES
//---------------------------------------------------------------------------------------------------------------------------------------
  #include <Arduino.h>
  #include <Wire.h>
  #include <WiFi.h>
  #include <ScioSense_ENS16x.h>
  #include <SD.h>
  #include <SPI.h>
  #include <time.h>
  #include <SensirionI2cSht4x.h>
  #include <cmath>
  #include <PubSubClient.h>
//---------------------------------------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------------------------------------
//              DEFINICIONES
//---------------------------------------------------------------------------------------------------------------------------------------
  #define SD_CS_PIN 10
  #define I2C_ADDRESS 0x52
  #define I2C_SDA 8
  #define I2C_SCL 9

  #define SD_SCK   12   // <--  SCK
  #define SD_MISO  13   // <--  MISO
  #define SD_MOSI  11   // <--  MOSI

  #define RED_LED_PIN 4   // LED red
  #define GREEN_LED_PIN 5 // LED green
  #define BLUE_LED_PIN 6  // LED blue



//---------------------------------------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------------------------------------
//               VARIABLES GLOBALES
//---------------------------------------------------------------------------------------------------------------------------------------

  // ENS160
  ENS160 ens160;
  SensirionI2cSht4x SHT40;
  static char errorMessage[64];
  static int16_t error;


  // Credenciales WiFi
  const char* ssid = "Tu ssid";     
  const char* password = "Tu contraseña";   



  // IP del router
  const uint8_t IP1_router = 192;
  const uint8_t IP2_router = 168;
  const uint8_t IP3_router = 1;
  const uint8_t IP4_router = 1;


  // IP del dispositivo (ESP32)
  const uint8_t IP1_dispositivo = 192;
  const uint8_t IP2_dispositivo = 168;
  const uint8_t IP3_dispositivo = 1;
  const uint8_t IP4_dispositivo = 150;


  // Datos
  uint8_t aqi = 0;
  uint16_t tvoc = 0, eco2 = 0;
  uint32_t rs0 = 0, rs1 = 0, rs2 = 0, rs3 = 0;
  float aTemperature = 0.0;
  float aHumidity = 0.0;


//---------------------------------------------------------------------------------------------------------------------------------------






// Configuración MQTT
const char* mqtt_topic  = "esp32/sensores";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;





void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    // Intentar conectar
    if (client.connect("ESP32_S3_CalidadAire")) {
      Serial.println("✅ Conectado al Broker Mosquitto");
    } else {
      Serial.print("❌ Falló, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 5 segundos...");
      delay(5000);
    }
  }
}







//-------------------------------------------------------------------
//        FUNCIONES PARA AJUSTAR EL RELOJ INTERNO Y AHORRAR ENERGÍA
//-------------------------------------------------------------------


  void setupPowerManagement() {
      // Establecer la frecuencia de la CPU a 80 MHz para optimización de consumo
      setCpuFrequencyMhz(80);

      // Imprimir la frecuencia del cristal XTAL
      Serial.print("Frecuencia del Cristal XTAL: ");
      Serial.print(getXtalFrequencyMhz());
      Serial.println(" MHz");

      // Imprimir la frecuencia de la CPU
      Serial.print("Frecuencia de la CPU: ");
      Serial.print(getCpuFrequencyMhz());
      Serial.println(" MHz");

      // Imprimir la frecuencia del bus APB
      Serial.print("Frecuencia del Bus APB: ");
      Serial.print(getApbFrequency());
      Serial.println(" Hz");
  }

//---------------------------------------------------------------------------------------------------------------------------------------













//---------------------------------------------------------------------------------------------------------------------------------------
//         FUNCIONES AUXILIARES 
//---------------------------------------------------------------------------------------------------------------------------------------
  // Control de LED RGB con PWM
  void PWM_led(int pin, int value) {
    // value de 0 a 255
    analogWrite(pin, value);
    delay(15);
  }


  // Controla el color del LED según el nivel de AQI
  void nivelAQI_led(uint8_t aqi){

  switch (aqi)
    {
    case 1:   // Buena
      PWM_led(RED_LED_PIN, 255);
      PWM_led(GREEN_LED_PIN, 255);
      PWM_led(BLUE_LED_PIN, 0);
      break;
    case 2:   // Moderada
      PWM_led(RED_LED_PIN, 255);
      PWM_led(GREEN_LED_PIN, 0);
      PWM_led(BLUE_LED_PIN, 255);
      break;
    case 3:   // Aceptable
      PWM_led(RED_LED_PIN, 0);
      PWM_led(GREEN_LED_PIN, 0);  
      PWM_led(BLUE_LED_PIN, 255);
      break;
    case 4:   // Dañina 
      PWM_led(RED_LED_PIN, 0);
      PWM_led(GREEN_LED_PIN, 255-128);
      PWM_led(BLUE_LED_PIN, 255);
      break;
    case 5:   // Muy dañina
      PWM_led(RED_LED_PIN, 0);  
      PWM_led(GREEN_LED_PIN, 255);
      PWM_led(BLUE_LED_PIN, 255);
      break;
    
    default:
      PWM_led(RED_LED_PIN, 0);  
      PWM_led(GREEN_LED_PIN, 255);
      PWM_led(BLUE_LED_PIN, 255);
      break;
    }

  }


  // Convierte temperatura en °C a formato ENS16x
  static inline uint16_t ens16x_temp_from_c(float tC) {
    float k64 = (tC + 273.15f) * 64.0f;
    if (k64 < 0) k64 = 0;
    if (k64 > 65535.f) k64 = 65535.f;
    return (uint16_t)lroundf(k64);
  }


  // Convierte humedad relativa en % a formato ENS16x
  static inline uint16_t ens16x_rh_from_percent(float rh) {
    if (rh < 0) rh = 0;
    if (rh > 100) rh = 100;
    return (uint16_t)lroundf(rh * 512.0f);
  }







//---------------------------------------------------------------------------------------------------------------------------------------
//                  SETUP
//---------------------------------------------------------------------------------------------------------------------------------------
void setup() {

    Serial.begin(115200);
    delay(3000);

    Serial.println(); // Salto de línea inicial


  //--------------------------- CONFIGURAR RELOJ ---------------------------  
    // Iniciar gestión de energía
    setupPowerManagement();
    Serial.println("🟢 ESP32 iniciado correctamente");
  //--------------------------------------------------------------------------


  

  //-------------------------------- INIT I2C --------------------------------  
    Wire.begin(I2C_SDA, I2C_SCL);
  //--------------------------------------------------------------------------

//-------------------------------- INIT WIFI -------------------------------

    IPAddress local_IP(IP1_dispositivo,IP2_dispositivo,IP3_dispositivo,IP4_dispositivo);      // IP del dispositivo
    IPAddress gateway(IP1_router,IP2_router,IP3_router,IP4_router);                           // IP del router
    IPAddress subnet(255,255,255,0);                                                          // Máscara de red
    IPAddress dns(8, 8, 8, 8);  // ← Google DNS, o el de tu router
                                                                  


    WiFi.disconnect(true); 
    WiFi.softAPdisconnect(true); // Desactivar modo punto de acceso si quedó activo
    delay(1000);

    Serial.println(WiFi.macAddress());

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false); // EVITA QUE EL WIFI DUERMA (Importante en redes de empresa)

      // ¡IMPORTANTE! Configurar la IP antes de iniciar WiFi
    if (!WiFi.config(local_IP, gateway, subnet, dns )) {
      Serial.println("Error al configurar IP Estática");
    }
    delay(500);

    WiFi.begin(ssid, password);
    Serial.print("Conectando a WiFi");


    // --- NUEVA LÓGICA DE TIEMPO LÍMITE (TIMEOUT) ---
    unsigned long startAttemptTime = millis();
    const unsigned long wifiTimeout = 30000; // 20 segundos

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
        delay(500);
        Serial.print(".");
        
        // TRUCO: Algunas redes corporativas necesitan un "empujón" si no responden
        if (millis() - startAttemptTime > 15000 && WiFi.status() != WL_CONNECTED) {
            WiFi.begin(ssid, password); 
        }
    }

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\n❌ Falla de conexión: Tiempo límite de 20s agotado.");
      // Aquí puedes decidir si quieres reiniciar el ESP o seguir sin WiFi
      // ESP.restart(); 
    } else {
      Serial.println("\n✅ WiFi Conectado");
      Serial.println("🟢 IP del dispositivo: " + WiFi.localIP().toString());



        String ipServer = "\n🟢 IP del router: " + String(IP1_router) + "." + String(IP2_router) + "." + String(IP3_router) + "." + String(IP4_router); 
        Serial.println(ipServer);
        Serial.println("\n🟢 IP del dispositivo: " + WiFi.localIP().toString());





        // --- NUEVO BLOQUE DE HORA ---
        configTime(0, 0, "pool.ntp.org", "time.google.com"); // Dos servidores
        setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
        tzset();

        Serial.println("Sincronizando hora NTP...");
        unsigned long t0 = millis();
        struct tm timeinfo;

        // Intentamos obtener la hora durante 15 segundos
        bool horaObtenida = false;
        while (millis() - t0 < 15000) { 
            if (getLocalTime(&timeinfo)) {
                horaObtenida = true;
                break;
            }
            delay(500);
            Serial.print(".");
        }

        if (!horaObtenida) {
            Serial.println("\n❌ ERROR: No se pudo obtener la hora NTP tras 15s.");

        } else {
            Serial.println("\n✅ HORA SINCRONIZADA:");
            Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
        }

    }
    // ------------------------------------------------




    client.setServer(mqtt_server, 1883);






  //-------------------------------- ENS160 ----------------------------------
    ens160.enableDebugging(Serial);
    pinMode(45, OUTPUT);
    digitalWrite(45, HIGH);  // Alimentación del sensor ENS160
    ens160.begin(&Wire, I2C_ADDRESS);
    Serial.println("Inicializando ENS160..."); 
    while (!ens160.init()) {
      Serial.print(".");
      delay(1000);
      Serial.println("❌ No se ha podido iniciar ENS160");
    }
    Serial.println("\n🟢 ENS160 inicializado correctamente");
    ens160.startStandardMeasure();
  //--------------------------------------------------------------------------



  //-------------------------------- SHT40 -----------------------------------
    SHT40.begin(Wire, SHT40_I2C_ADDR_44);
    SHT40.softReset();
    delay(10);
    uint32_t serialNumber = 0;
    error = SHT40.serialNumber(serialNumber);
    if(error != 0) {
      snprintf(errorMessage, sizeof(errorMessage), "Error al obtener el número de serie: %d", error);
      Serial.println(errorMessage);
      Serial.println("❌ No se ha podido iniciar SHT40");
    } else {
      //Serial.printf("🟢 SHT40 Número de serie: %08X\n", serialNumber);
      Serial.println("🟢 SHT40 inicializado correctamente");
    }
  //--------------------------------------------------------------------------
    
}









//---------------------------------------------------------------------------------------------------------------------------------------
//                  LOOP
//---------------------------------------------------------------------------------------------------------------------------------------
void loop() {

    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    // Leer sensores (tu lógica actual)
    SHT40.measureHighPrecision(aTemperature, aHumidity);
    
    uint16_t t_in  = ens16x_temp_from_c(aTemperature);
    uint16_t rh_in = ens16x_rh_from_percent(aHumidity);
    ens160.writeCompensation(t_in, rh_in);
    
    if (ens160.update() == RESULT_OK) {
      aqi = ens160.getAirQualityIndex_UBA();
      tvoc = ens160.getTvoc();
      eco2 = ens160.getEco2();
    }

    // Enviar datos cada 5 segundos
    unsigned long now = millis();
    if (now - lastMsg > 5000) {
      lastMsg = now;

      // Formato Influx: nombre_tabla temperatura=25.2,humedad=40.5...
      String payload = "calidad_aire "; // espacio después del nombre
      payload += "temperatura=" + String(aTemperature) + ",";
      payload += "humedad=" + String(aHumidity) + ",";
      payload += "aqi=" + String(aqi) + ",";
      payload += "tvoc=" + String(tvoc) + ",";
      payload += "eco2=" + String(eco2);

      Serial.print("Publicando: ");
      Serial.println(payload);
      
      if (client.publish(mqtt_topic, payload.c_str())) {
        Serial.println("📤 Enviado a Mosquitto OK");
      } else {
        Serial.println("❌ Error al publicar");
      }
    }

  delay(1000);


}



































