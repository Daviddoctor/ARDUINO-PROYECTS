//#include <SerialESP8266wifi.h>
#include <ArduinoJson.h>
#include <Key.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <WiFi.h>


//LiquidCrystal lcd(RS, E, D4, D5, D6, D7);
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

const int pirPin = 2;
const int radarPin = 4;  // Pin al que está conectado el sensor RCWL-0516
const int ledPin = 3;

const char ssid[] = "SSID";
const char password[] = "CONTRASEÑA";
const char apiKey[] = "API_KEY";
const char host[] = " ";      //SERVIDOR
const int Port = 80;          //PUERTO PARA LA SULISITUD HTTP
const char endPoint[] = " ";  //RUTA DE LA API

const byte ROWS = 4;  //FILAS
const byte COLS = 4;  //COLUMNAS
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
//                  F1  F2  F3  F4
byte rowPins[ROWS] = { 30, 32, 34, 36 };
byte colPins[COLS] = { 22, 24, 26, 28 };

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
int i, dato, datoa, datob, b;
byte datoe[6];
byte datof[2];

void teclado() {
  LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
  int clave[2];
  char datom;
  long segundos, auxsegundos;
  lcd.begin(16, 2);
  lcd.clear();
  auxsegundos = 0;
  segundos = 0;
  b = 8;

  for (i = 0; i < 4; i++) {
    do {
      datom = keypad.getKey();
      delay(60);
      if (datom != '\0') {
        switch (i + 1) {
          case 1:
            clave[0] = datom - 0x30;
            clave[0] = (clave[0] << 4);
            lcd.setCursor(6, 1);
            lcd.print("*");
            delay(50);
            break;
          case 2:
            clave[1] = datom - 0x30;
            clave[0] = clave[0] + clave[1];
            lcd.print("*");
            delay(50);
            break;
          case 3:
            clave[2] = datom - 0x30;
            clave[2] = (clave[2] << 4);
            lcd.print("*");
            delay(50);
            break;
          case 4:
            clave[1] = datom - 0x30;
            clave[1] = clave[1] + clave[2];
            lcd.print("*");
            delay(50);
            break;
        }
        i++;
        delay(500);
        auxsegundos = 0;
        segundos = 0;
      }
      auxsegundos++;
      if (auxsegundos > 20) {
        segundos++;
        auxsegundos = 0;
      }
    } while ((segundos < 6) && (i < 4));

    clave[i] = "#";
    lcd.setCursor(b, 1);
    lcd.print("*");
    b++;
    auxsegundos = 0;
    segundos = 0;
  }
}

void connectWiFi() {
  Serial.print("CONECTANDO A WIFI....");
  WiFi.begin(ssid, password);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 10) {
    delay(500);
    Serial.print(".");
    intentos ++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nError: No se pudo conectar al WiFi después de 10 intentos. Abortando.");
    return;
  }

  Serial.print("");
  Serial.print("WIFI CONECTADO");
}

void sendNotification() {
  WiFiClient client;
  Serial.print("Conectando a ");
  Serial.println(host);

  int intentos = 0;
  if (!client.connect(host, Port) && intentos < 5) {
    Serial.println("Error de conexión");
    Serial.println(intentos + 1);
    intentos ++;
    delay(1000);
  }

  if (intentos > 5) {
    Serial.println("No se pudo establecer la conexión después de 5 intentos. Abortando.");
    return;
  }

  LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
  String url = String("POST ") + endPoint + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Content-Type: application/json\r\n" + "x-api-key: " + apiKey + "\r\n" + "Content-Length: 25\r\n\r\n" + "{\"message\":\"Movimiento detectado\"}";

  Serial.println("Enviando notificación...");
  client.print(url);

  delay(10);
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("Notificación enviada");
  client.stop();
}

void setup() {
  Wire.begin();
  Serial.begin(115200);
  lcd.begin(16, 2);
  pinMode(pirPin, INPUT);
  pinMode(radarPin, INPUT);  // Configura el pin del sensor RCWL-0516 como entrada
  pinMode(ledPin, OUTPUT);

  teclado();
  connectWiFi();
}

void loop() {
  LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
  int lastPirState = LOW;

  int motionDetecte = digitalRead(pirPin);
  if (motionDetecte == HIGH) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MOVIMIENTO DETECTADO");
    digitalWrite(ledPin, HIGH);
    delay(500);

  } else {
    digitalWrite(ledPin, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SIN MOVIMINETO");
    delay(500);
  }
  int pirState = digitalRead(pirPin);
  int radarState = digitalRead(radarPin);  // Lee el estado del sensor RCWL-0516

  if (pirState == HIGH && lastPirState == LOW) {
    Serial.println("MOVIMIENTO DETECTADO");
    sendNotification();
    delay(2000);
  }
  lastPirState = pirState;
}