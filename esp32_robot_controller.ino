/*
  ESP32 Bluetooth Robot Controller
  - Receives single-letter commands over Classic Bluetooth (SPP): F, B, L, R, S
  - Shows every received message on a 1.8" SPI TFT (ST7735, 128x160)
  - Drives 2 wheel motors (L298N #1) for forward/backward
  - Drives 1 rotation motor (L298N #2) for left/right turn-in-place

  Libraries needed (Arduino IDE > Tools > Manage Libraries):
    - "Adafruit ST7735 and ST7789 Library" by Adafruit
    - "Adafruit GFX Library" by Adafruit
  Board: ESP32 Dev Module (Bluetooth Classic requires a non-C3/S3-only variant with BT Classic support)

  ===================== WIRING =====================
  TFT (1.8" ST7735 128x160, 8 pin):
    VCC -> 3.3V
    GND -> GND
    CS  -> GPIO 5
    RST -> GPIO 4
    A0/DC -> GPIO 2
    SDA/MOSI -> GPIO 23
    SCK -> GPIO 18
    LED -> GPIO 15 (backlight, driven HIGH)

  L298N #1 (drive wheels):
    ENA -> GPIO 25   IN1 -> GPIO 26   IN2 -> GPIO 27   (Left wheel motor)
    ENB -> GPIO 14   IN3 -> GPIO 32   IN4 -> GPIO 33   (Right wheel motor)

  L298N #2 (rotation motor, only channel A used):
    ENA -> GPIO 13   IN1 -> GPIO 16   IN2 -> GPIO 17   (Rotation motor)
  ====================================================
*/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "BluetoothSerial.h"

// ---------- Bluetooth ----------
BluetoothSerial SerialBT;
const char* BT_NAME = "ESP32_ROBOT";   // <-- must match name used by Android app

// ---------- TFT pins ----------
#define TFT_CS   5
#define TFT_RST  4
#define TFT_DC   2
#define TFT_LED  15
// SDA/MOSI = GPIO23, SCK = GPIO18 are used automatically by ESP32 default VSPI

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// ---------- Drive wheel motors (L298N #1) ----------
#define ENA 25
#define IN1 26
#define IN2 27
#define ENB 14
#define IN3 32
#define IN4 33

// ---------- Rotation motor (L298N #2) ----------
#define ENC 13
#define IN5 16
#define IN6 17

String lastCmd = "";

void setup() {
  Serial.begin(115200);
  SerialBT.begin(BT_NAME);
  Serial.println("Bluetooth started as " + String(BT_NAME));

  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENC, OUTPUT); pinMode(IN5, OUTPUT); pinMode(IN6, OUTPUT);
  stopAll();

  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);           // backlight on

  tft.initR(INITR_BLACKTAB);             // most common 1.8" 128x160 variant
  tft.setRotation(1);                    // landscape; change 0-3 if orientation is wrong
  tft.fillScreen(ST77XX_BLACK);
  showMessage("Waiting...");
}

void loop() {
  if (SerialBT.available()) {
    String msg = SerialBT.readStringUntil('\n');
    msg.trim();
    if (msg.length() > 0) {
      lastCmd = msg;
      showMessage(msg);
      handleCommand(msg);
    }
  }
}

void handleCommand(String cmd) {
  if      (cmd == "FORWARD") forward();
  else if (cmd == "BACK") backward();
  else if (cmd == "LEFT") rotateLeft();
  else if (cmd == "RIGHT") rotateRight();
  else if (cmd == "STOP") stopAll();
}

void forward() {
  // Left Motor
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  // Right Motor (Reversed)
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void backward() {
  // Left Motor
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  // Right Motor (Reversed)
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void rotateLeft() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  digitalWrite(ENA, LOW); digitalWrite(ENB, LOW);
  digitalWrite(IN5, HIGH); digitalWrite(IN6, LOW); digitalWrite(ENC, HIGH);
}

void rotateRight() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  digitalWrite(ENA, LOW); digitalWrite(ENB, LOW);
  digitalWrite(IN5, LOW); digitalWrite(IN6, HIGH); digitalWrite(ENC, HIGH);
}

void stopAll() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  digitalWrite(IN5, LOW); digitalWrite(IN6, LOW);
  digitalWrite(ENA, LOW); digitalWrite(ENB, LOW); digitalWrite(ENC, LOW);
}

void showMessage(String msg) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(4, 6);
  tft.print("Received:");

  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.setCursor(10, 40);
  tft.print(msg);
}
