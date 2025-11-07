#include <TinyGPSPlus.h>

// GPS en Serial2
// GPS TX -> ESP32 GPIO16
// GPS RX -> ESP32 GPIO17 (opcional)
TinyGPSPlus gps;

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 2000; // ms

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println("Inicializando módulo GPS (GY-NEO6MV2)...");
  Serial.println("Esperando señal GPS...");
}

void loop() {
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
  }

  if (millis() - lastUpdate > updateInterval) {
    lastUpdate = millis();
    printGPSData();
  }
}

void printGPSData() {
  Serial.println("====================================");

  if (gps.location.isValid()) {
    Serial.print("Latitud: ");
    Serial.println(gps.location.lat(), 6);
    Serial.print("Longitud: ");
    Serial.println(gps.location.lng(), 6);
  } else {
    Serial.println("Lat/Long: No disponible (sin fix)");
  }

  if (gps.altitude.isValid()) {
    Serial.print("Altitud: ");
    Serial.print(gps.altitude.meters());
    Serial.println(" m");
  }

  if (gps.satellites.isValid()) {
    Serial.print("Satélites: ");
    Serial.println(gps.satellites.value());
  }

  if (gps.hdop.isValid()) {
    Serial.print("HDOP: ");
    Serial.println(gps.hdop.hdop());
  }

  if (gps.speed.isValid()) {
    Serial.print("Velocidad: ");
    Serial.print(gps.speed.kmph());
    Serial.println(" km/h");
  }

  if (gps.date.isValid() && gps.time.isValid()) {
    Serial.print("Fecha: ");
    Serial.printf("%02d/%02d/%04d\n",
                  gps.date.day(), gps.date.month(), gps.date.year());

    Serial.print("Hora UTC: ");
    Serial.printf("%02d:%02d:%02d\n",
                  gps.time.hour(), gps.time.minute(), gps.time.second());
  }

  Serial.println("====================================\n");
}
