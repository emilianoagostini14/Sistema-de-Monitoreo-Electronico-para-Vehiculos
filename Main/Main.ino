// ========================== LIBRERÍAS ==========================
// Librería para decodificar datos NMEA del GPS (TinyGPS++)
#include <TinyGPSPlus.h>

// Librerías para comunicación I2C y manejo del display OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Librería para el conversor analógico-digital ADS1115
#include <Adafruit_ADS1X15.h>

// ========================== CONFIGURACIÓN OLED ==========================
#define SCREEN_WIDTH 128   // Ancho del display en píxeles
#define SCREEN_HEIGHT 64   // Alto del display en píxeles
#define OLED_RESET -1      // Pin de reset (-1 = no usado)
#define OLED_ADDR 0x3C     // Dirección I2C del módulo OLED

// Creación del objeto display (usando el controlador SH1106)
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ========================== CONFIGURACIÓN ADC (ADS1115) ==========================
// Creación del objeto ADC de 16 bits
Adafruit_ADS1115 ads;
float voltage;      // Variable para guardar el voltaje leído
float temperatura;  // Variable para la temperatura convertida

// ========================== CONFIGURACIÓN GPS ==========================
// Objeto para manejar los datos del módulo GPS
TinyGPSPlus gps;

// Variables para suavizar la velocidad usando un filtro exponencial
float velocidadFiltrada = 0;
const float alpha = 0.25; // Coeficiente de suavizado (más alto = más respuesta rápida)

// Variables de tiempo para actualizar datos a intervalos fijos
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 500; // Tiempo entre actualizaciones (ms)

// ========================== SETUP ==========================
void setup() {
  Serial.begin(115200); // Comunicación serie para monitoreo
  Wire.begin(21, 22);   // Pines I2C de la ESP32 (SDA=21, SCL=22)

  // ---------- INICIALIZACIÓN OLED ----------
  if (!display.begin(OLED_ADDR, true)) {
    Serial.println("Error al inicializar OLED");
    while (1); // Se queda bloqueado si no detecta el display
  }
  display.setRotation(1); // Modo vertical
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Inicializando...");
  display.display();

  // ---------- INICIALIZACIÓN DEL ADC ----------
  if (!ads.begin()) {
    Serial.println("Error al inicializar ADS1115");
    display.println("ADS1115 ERROR!");
    display.display();
    while (1); // Se detiene si no detecta el ADC
  }

  // Configura el rango de entrada del ADC a ±6.144V
  // (1 bit = 0.1875 mV)
  ads.setGain(GAIN_TWOTHIRDS);

  // ---------- INICIALIZACIÓN DEL GPS ----------
  // Comunicación UART con el módulo GPS NEO-6M
  // RX = pin 16, TX = pin 17, velocidad = 9600 baudios
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
}

// ========================== LOOP PRINCIPAL ==========================
void loop() {
  // Leer la temperatura desde el ADC
  temperatura = leerTemperatura();

  // Leer datos del GPS mientras haya bytes disponibles
  while (Serial2.available()) gps.encode(Serial2.read());

  // Actualizar la pantalla y datos cada 500 ms
  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();
    mostrarDatos();  // Actualiza la información en la pantalla
    printGPSData();  // Muestra los datos por el monitor serie
  }
}

// ========================== FUNCIÓN: Mostrar Datos en OLED ==========================
void mostrarDatos() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  // Si el GPS tiene posición y velocidad válidas
  if (gps.location.isValid() && gps.speed.isValid()) {

    float vel = gps.speed.kmph(); // Velocidad en km/h
    // Filtro de media exponencial para suavizar la lectura
    velocidadFiltrada = (alpha * vel) + ((1 - alpha) * velocidadFiltrada);
    
    int digits = 0;

    // Determina la cantidad de dígitos de la velocidad (para centrar el texto)
    if (velocidadFiltrada < 10) digits = 1;
    else if (velocidadFiltrada < 100) digits = 2;
    else digits = 3;

    // Define tamaño de texto y posición según cantidad de dígitos
    int textSize;
    int posX;

    if (digits == 1) {
      textSize = 4;
      posX = 20;
    }
    else if (digits == 2) {
      textSize = 4;
      posX = 8;
    }
    else {
      textSize = 3;
      posX = 5;
    }

    // Mostrar la velocidad en grande en el centro superior
    display.setTextSize(textSize);
    display.setCursor(posX, 5);
    display.print((int)velocidadFiltrada);
    display.setTextSize(2);
    display.setCursor(9, 40);
    display.print("km/h");

    // ---------- Mostrar temperatura ----------
    textSize = ((int)temperatura < 100) ? 2 : 1;
    int numDigits = ((int)temperatura < 10) ? 1 : (((int)temperatura < 100) ? 2 : 3);
    int numWidth = numDigits * 6 * textSize;
    int totalWidth = numWidth + (6 * textSize);
    posX = (64 - totalWidth) / 2;

    display.setTextSize(textSize);
    display.setCursor(posX, 80);
    display.print((int)temperatura);

    // Dibuja el símbolo °C
    int circleX = posX + numWidth + 3;
    int circleY = 80 + (textSize * 2);
    int circleR = max(1, textSize - 1);
    display.fillCircle(circleX, circleY, circleR, SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(circleX + circleR + 5, 83);
    display.print("C");

    // ---------- Mostrar hora local ----------
    if (gps.time.isValid()) {
      int hourLocal = (gps.time.hour() + 24 - 3) % 24; // Ajuste UTC-3 (Argentina)
      char hora[6];
      sprintf(hora, "%02d:%02d", hourLocal, gps.time.minute());
      display.setCursor(2, 108);
      display.setTextSize(2);
      display.print(hora);
    }

  } else {
    // Si no hay señal GPS (sin "fix")
    display.setTextSize(1);
    display.setCursor(14, 5);
    display.print("NO FIX");

    // Mostrar solo la temperatura
    int textSize = ((int)temperatura < 100) ? 2 : 1;
    int numDigits = ((int)temperatura < 10) ? 1 : (((int)temperatura < 100) ? 2 : 3);
    int numWidth = numDigits * 6 * textSize;
    int totalWidth = numWidth + (6 * textSize);
    int posX = (64 - totalWidth) / 2;

    display.setTextSize(textSize);
    display.setCursor(posX, 80);
    display.print((int)temperatura);

    // Dibuja °C
    int circleX = posX + numWidth + 3;
    int circleY = 80 + (textSize * 2);
    int circleR = max(1, textSize - 1);
    display.fillCircle(circleX, circleY, circleR, SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(circleX + circleR + 5, 83);
    display.print("C");

    // Mostrar hora (si está disponible)
    if (gps.time.isValid()) {
      int hourLocal = (gps.time.hour() + 24 - 3) % 24;
      char hora[6];
      sprintf(hora, "%02d:%02d", hourLocal, gps.time.minute());
      display.setCursor(2, 108);
      display.setTextSize(2);
      display.print(hora);
    }
  }

  // Actualizar pantalla
  display.display();
}

// ========================== FUNCIÓN: Leer Temperatura ==========================
// Convierte el valor leído del ADC en voltaje y luego a temperatura
float leerTemperatura() {
  int16_t adc0 = ads.readADC_SingleEnded(0); // Lectura canal A0 del ADS1115

  // Cada bit equivale a 0.1875 mV cuando GAIN_TWOTHIRDS
  voltage = adc0 * 0.1875 / 1000.0;

  // Conversión lineal a °C según calibración experimental
  float t = -(85.5 * voltage) + 254;
  return t;
}

// ========================== FUNCIÓN: Mostrar Datos en el Monitor Serie ==========================
void printGPSData() {
  Serial.println("====================================");

  // Latitud / Longitud
  if (gps.location.isValid()) {
    Serial.print("Latitud: ");
    Serial.println(gps.location.lat(), 6);
    Serial.print("Longitud: ");
    Serial.println(gps.location.lng(), 6);
  } else {
    Serial.println("Lat/Long: No disponible (sin fix)");
  }

  // Altitud
  if (gps.altitude.isValid()) {
    Serial.print("Altitud: ");
    Serial.print(gps.altitude.meters());
    Serial.println(" m");
  }

  // Satélites detectados
  if (gps.satellites.isValid()) {
    Serial.print("Satélites: ");
    Serial.println(gps.satellites.value());
  }

  // Precisión (HDOP)
  if (gps.hdop.isValid()) {
    Serial.print("HDOP: ");
    Serial.println(gps.hdop.hdop());
  }

  // Velocidad en km/h
  if (gps.speed.isValid()) {
    Serial.print("Velocidad: ");
    Serial.print(gps.speed.kmph());
    Serial.println(" km/h");
  }

  // Fecha y hora UTC
  if (gps.date.isValid() && gps.time.isValid()) {
    Serial.print("Fecha: ");
    Serial.printf("%02d/%02d/%04d\n", gps.date.day(), gps.date.month(), gps.date.year());

    Serial.print("Hora UTC: ");
    Serial.printf("%02d:%02d:%02d\n",
                  gps.time.hour(), gps.time.minute(), gps.time.second());
  }

  // Voltaje y temperatura leídos del sensor
  Serial.print("Voltaje: ");
  Serial.print(voltage, 3);
  Serial.print(" V -> Temp: ");
  Serial.print(temperatura, 1);
  Serial.println(" °C");

  Serial.println("====================================\n");
}
