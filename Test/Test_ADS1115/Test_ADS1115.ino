#include <Wire.h>
#include <Adafruit_ADS1X15.h>

// Crear objeto ADS1115
Adafruit_ADS1115 ads; // 0x48 es la dirección por defecto

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);   // SDA, SCL - Inicializa I2C
  ads.begin();  // Inicializa ADS1115
}

void loop() {
  int16_t adc0 = ads.readADC_SingleEnded(0); // Leer canal A0
  float voltage = adc0 * 0.1875/1000;        // 0.1875 mV por bit (valor por default PGA 2/3)
  float temp = (-85.5*voltaje)+254;
  Serial.print("Canal 0: ");
  Serial.print(adc0);
  Serial.print(" -> Voltaje: ");
  Serial.print(voltage, 4);
  Serial.print(" -> Temp: ");
  Serial.print(temp, 4);
  delay(500);
}
