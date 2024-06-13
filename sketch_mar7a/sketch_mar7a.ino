#include <Wire.h>

#include <EEPROM.h>
//#include <Key.h>  //se puede quitar sin problema
#include <Keypad.h>
#include <stdlib.h>
#include <math.h>
#include <LiquidCrystal.h>

#define DS1307_r2C_ADDRESS 0x68  //Inicializar el reloj de tiempo real


//LiquidCrystal lcd(RS, E, D4, D5, D6, D7);
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
const byte ROWS = 4;  //FILAS
const byte COLS = 4;  //COLUMNAS
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
//                     F1 F2 F3 F4
byte rowPins[ROWS] = {
  30,
  32,
  34,
  36,
};
//                     c1 c2 c3 c4
byte colPins[COLS] = {
  22,
  24,
  26,
  28,
};

void imprimirMensajeBienvenida() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" BIENVENIDOS");
  lcd.setCursor(0, 1);
  lcd.print("ARQUITECTURA 401");
  delay(1000);
}

void setup() {   //configuracion de pines
  Wire.begin();  //Inicializar libreria
  lcd.begin(16, 2);
  lcd.clear();          //inicializa la libreria
  Serial.begin(9600);   //Incializar la libreria i2C
  pinMode(13, OUTPUT);  //salida
  pinMode(2, INPUT);    //entrada
  pinMode(8, OUTPUT);

  imprimirMensajeBienvenida();

  lcd.clear();
  lcd.print("INGRESE LA CLAVE");
  delay(1000);
}

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
int i, dato, clave[4], b, datoa, datob, add;
byte datop[6];
byte datoe[2];
byte datoi[6];
char datom;
long segundos, auxsegundos;

/*************************************** Rutinas de la memoria Externa ***********************************************/
//Escribir memoria externa
void i2c_eeprom_write_byte(int deviceaddress, unsigned int eeaddress, byte data) {
  int rdata = data;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));
  Wire.write((int)(eeaddress >> 0xFF));
  Wire.write(rdata);
  Wire.endTransmission();
}

//Leer memoria externa
byte i2c_eeprom_read_byte(int deviceaddress, unsigned int eeaddress) {
  byte rdata = 0xFF;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));
  Wire.write((int)(eeaddress >> 0xFF));
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress, 1);
  if (Wire.available()) rdata = Wire.read();
  return rdata;
}

void verFechaHora() {
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fecha: ");
  lcd.print(dayOfMonth);
  lcd.print("/");
  lcd.print(month);
  lcd.print("/");
  lcd.print(year);
  lcd.setCursor(0, 1);
  lcd.print("Hora: ");
  lcd.print(hour);
  lcd.print(":");
  lcd.print(minute);
  lcd.print(":");
  lcd.print(second);
  delay(600);
}

void escribirMemoria() {
  for (i = 0; i < 2; i++) {
    EEPROM.write(add, clave[i]);
    add++;
    delay(5);
  }
}

void leerMemoria() {
  for (int i = 0; i < 6; i++) {   // Usa una variable local para el índice del bucle
    datop[i] = EEPROM.read(add);  // Lee el dato de la EEPROM y guárdalo en el array
    add++;
    delay(5);
  }
}

void derecha() {
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
  delay(100);
}
//escribir la memoria externa
void escribir_ext_memoria() {
  for (i = 0; i < 2; i++) {
    i2c_eeprom_write_byte(0x50, i, clave[i]);
    delay(5);
  }
}
//leer la memoria externa
void leer_ext_memoria() {
  for (i = 0; i < 2; i++) {
    datoe[i] = i2c_eeprom_read_byte(0x50, i);
    delay(5);
  }
}

void teclado() {
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
            delay(10);
            break;
          case 2:
            clave[1] = datom - 0x30;
            clave[0] = clave[0] + clave[1];
            lcd.print("*");
            delay(10);
            break;
          case 3:
            clave[2] = datom - 0x30;
            clave[2] = (clave[2] << 4);
            lcd.print("*");
            delay(10);
            break;
          case 4:
            clave[1] = datom - 0x30;
            clave[1] = clave[1] + clave[2];
            lcd.print("*");
            delay(10);
            break;
        }
        i++;
        delay(1000);
        auxsegundos = 0;
        segundos = 0;
      }
      auxsegundos++;
      if (auxsegundos > 20) {
        segundos++;
        auxsegundos = 0;
      }
    } while ((segundos < 8) && (i < 4));

    clave[i] = "#";
    lcd.setCursor(b, 1);
    lcd.print("*");
    b++;
    auxsegundos = 0;
    segundos = 0;
  }
}

boolean flag = true;  // Declarar la variable booleana flag

void un_digito() {
  flag = true;
  do {
    datom = keypad.getKey();  // Lee el botón presionado en el teclado
    delay(500);
    if (datom != NO_KEY) {  // Verifica si se ha presionado algún botón
      lcd.setCursor(8, 1);
      lcd.print(datom);
      flag = false;
      delay(500);
    }
  } while (flag == true);
  datom = datom - 0x30;
}

void secuencia() {  //metodo
  for (i = 0; i < 5; i++) {
    digitalWrite(13, HIGH);  //ALTO
    delay(300);
    digitalWrite(13, LOW);  //BAJO
    delay(300);
  }
}

boolean mensajeBienvenida = false;
boolean fechaEstablecida = false;

void loop() {

  leerMemoria();
  teclado();

  if (clave[0] == 0x12 && clave[1] == 0x34) {
    mostrarMenu();
  } else {
    lcd.clear();
    lcd.print("Clave incorrecta");
    lcd.setCursor(0, 1);
    delay(1000);
  }
}

void mostrarMenu() {
  lcd.clear();
  lcd.print("BIENVENIDOS AL");
  lcd.setCursor(0, 1);
  lcd.print("MENU DEL DISENADOR");
  delay(2000);

  lcd.clear();
  lcd.print("1. PARA DAVID");
  lcd.setCursor(0, 1);
  lcd.print("2. PARA DANIEL");
  delay(2000);

  lcd.clear();
  lcd.print("3.PARA DUVAN");
  lcd.setCursor(0, 1);
  lcd.print("4. PARA SALIR");
  delay(1000);
  lcd.clear();
  un_digito();

  // Determina la acción a realizar según la opción seleccionada
  switch (datom) {
    case 0x01:
      menuUsuario(0); // Usuario David
      break;
    case 0x02:
      menuUsuario(1); // Usuario Daniel
      break;
    case 0x03:
      menuUsuario(2); // Usuario Duvan
      break;
    case 0x04:
      lcd.clear();
      lcd.print("Hasta luego!");
      delay(1000);
      break;
    default:
      lcd.clear();
      lcd.print("Error de número");
      delay(1000);
      mostrarMenu();
  }
}

void menuUsuario(int usuario) {
  lcd.clear();
  lcd.print("Bienvenido al");
  lcd.setCursor(0, 1);
  lcd.print("menú de usuario");
  delay(1000);

  lcd.clear();
  lcd.print("MARQUE UN NUMERO");
  lcd.setCursor(0, 1);
  lcd.print("PARA: ");
  delay(500);

  // Menú usuario
  switch (usuario) {
    case 0: // Usuario 1
      lcd.setCursor(0, 0);
      lcd.print("1. Mostrar Fecha");
      lcd.setCursor(0, 1);
      lcd.print("2. Cambiar clave");
      break;
    case 1: // Usuario 2
      lcd.setCursor(0, 0);
      lcd.print("1. Mostrar Fecha");
      lcd.setCursor(0, 1);
      lcd.print("2. Cambiar clave");
      break;
    
    default:
      lcd.clear();
      lcd.print("No se encontró");
      lcd.setCursor(0, 1);
      lcd.print("el usuario");
  }
  un_digito();

  switch (datom) {
    case 1:
      verFechaHora();
      break;
    case 2:
      cambiarClave();
      break;
    default:
      lcd.clear();
      lcd.print("Opción no válida");
      delay(5000);
  }
}

void cambiarClave() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ingrese nueva clave: ");
  delay(2000);

  //Solicitar y guardar nueva clave
  teclado();
  for (int i = 0; i < 4; i++) {
    clave[i] = datom - '0';
    lcd.setCursor(i, 1);
    lcd.print('*');
  }

  //Guardar clave en EEPROM
  guardarClave();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Clave cambiada");
  delay(2000);
}

void guardarClave() {
  int direccionInicioClave = 10;
  for (int i = 0; i < 4; i++) {
    EEPROM.write(direccionInicioClave + i, clave[i]);
  }
}

//Convierte numeros decimales a BCD
byte decToBcd(byte val) {
  return ((val / 10 * 16) + (val % 10));
}
//Convierte BCD a numero decimales
byte bcdToDec(byte val) {
  return ((val / 16 * 10) + (val % 16));
}
//Escribirle al reloj de tiempo real
void setDateDs1307(byte second,      //0-59
                   byte minute,      //0-59
                   byte hour,        //0-23
                   byte dayOfWeek,   //1-7
                   byte dayOfMonth,  //1-28/29/30/31
                   byte month,       //1-12
                   byte year         //0-99
) {
  Wire.beginTransmission(DS1307_r2C_ADDRESS);
  Wire.write(0);
  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(dayOfWeek));
  Wire.write(decToBcd(dayOfMonth));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.endTransmission();
}
//Obtiene la fecha y el tiempo del DS1307
void getDateDs1307(byte *second,
                   byte *minute,
                   byte *hour,
                   byte *dayOfWeek,
                   byte *dayOfMonth,
                   byte *month,
                   byte *year) {
  Wire.begin(DS1307_r2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_r2C_ADDRESS, 7);
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}