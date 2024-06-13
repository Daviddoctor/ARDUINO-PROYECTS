// #include <ESP8266WiFi.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Define 2 constantes para el parametro de dht(pin, modelo)
#define DHTPIN 14
#define DHTTYPE DHT21
DHT dht(DHTPIN, DHTTYPE);

// Conexion a una red
const char* ssid = "U-SIGLOXXI";
const char* password = "UdeCsigloXXI";

// Conexion con la dirección de tu broker MQTT
const char* mqtt_server = "test.mosquitto.org";

// Crea una instancia del cliente WiFi
WiFiClient espClient;
PubSubClient client(espClient);

// Función para establecer la conexión WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  //Serial.print("Conectando a: ", ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conexión WiFi establecida con ");
  Serial.println(ssid);
  Serial.print("Dirección IP asignada: ");
  Serial.println(WiFi.localIP());
}

// Función de devolución de llamada para manejar los mensajes MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensage Recibido [");
  Serial.print(topic);
  Serial.print("] ");

  String str_topic = String(topic);
  String str_payload;

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    str_payload += (char)payload[i];
  }

  Serial.println();

  if (str_topic == "carga-doctor") {
    if (str_payload == "true") {
      Serial.println("Encender carga 1");  // puede ir una acción para un pin de la tarjeta
      digitalWrite(2, HIGH);
    }
    if (str_payload == "false") {
      Serial.println("apagar carga");  // puede ir una acción para un pin de la tarjeta
      digitalWrite(2, LOW);
    }
  }
}

void setup() {
  pinMode(2, OUTPUT);
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  dht.begin();          // Inicializa libreria del sensor de humedad y temperatura
  pinMode(14, OUTPUT);  // Usamos el pin 14 para conectarlo al DATA del DHT11
}

void reconnect() {
  if (!client.connected()) {
    while (!client.connected()) {  // Intenta reconectar si no está conectado
      Serial.println("Intentando conexión MQTT...");
      String clientId = "doctor_";
      Serial.print(clientId);
      clientId += String(random(0xffff), HEX);
      if (client.connect(clientId.c_str())) {
        Serial.println("conectado");
        //client.publish("test-arana", "Prueba servidor arana");
        client.subscribe("carga-doctor");
      } else {
        Serial.print("falló, rc=");
        Serial.print(client.state());
        Serial.println(" intentando de nuevo en 5 segundos");
        delay(5000);
      }
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  // Leer humedad y temperatura del DHT dht
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  Serial.print("Humedad: ");
  Serial.print(h);
  Serial.print("%  Temperatura: ");
  Serial.print(t);
  Serial.println("°C  ");

  client.publish("temp-doctor", String(t).c_str());
  client.publish("hum-doctor", String(h).c_str());


  delay(1000);
}