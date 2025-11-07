#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long lastUpdate = 0;
int counter = 0;

void setup() {
  Serial.begin(9600);
  delay(1000);

  if (!display.begin(SCREEN_ADDRESS, true)) {
    Serial.println(F("Error al iniciar la pantalla SH1106"));
    for (;;);
  }

  display.clearDisplay();
  display.display();
}

void loop() {
  if (millis() - lastUpdate >= 2000) {  // Cada 2 segundos
    lastUpdate = millis();

    display.clearDisplay();

    display.setTextSize(3);  // Tamaño grande
    display.setTextColor(SH110X_WHITE);

    // Para centrar el texto, ajustamos la posición según la cantidad de dígitos
    int x;
    if (counter < 10) x = 55;
    else if (counter < 100) x = 45;
    else x = 35;

    display.setCursor(x, 20);
    display.print(counter);

    display.display();

    Serial.println(counter);

    counter++;
    if (counter > 100) counter = 0;
  }
}
