#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <BluetoothSerial.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Enable it via `idf.py menuconfig` / Arduino Tools menu.
#endif

BluetoothSerial SerialBT;

// TFT pin connections (change to match your wiring)
#define TFT_CS   5
#define TFT_RST  4
#define TFT_DC   2

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// ---- L298N Motor Driver pins (change to match your wiring) ----
// Left motor (Motor A on L298N)
#define IN1 27
#define IN2 26
#define ENA 14   // PWM speed pin for left motor

// Right motor (Motor B on L298N)
#define IN3 25
#define IN4 33
#define ENB 32   // PWM speed pin for right motor

#define MOTOR_SPEED 255   // 0-255 PWM value, tweak as needed

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_FaceRobot");   // name shown when pairing over Bluetooth

  tft.initR(INITR_BLACKTAB);   // try INITR_GREENTAB if colors/offset look wrong
  tft.setRotation(1);          // landscape: 160 wide x 128 high (better for a face)
  tft.fillScreen(ST77XX_BLACK);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  stopMotors();

  happyFace();   // initial face on boot
}

void loop() {
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    handleBluetoothCommand(command);
  }
}

// Reads the Bluetooth text and routes it to the matching face / motor function
void handleBluetoothCommand(String command) {
  if (command.length() == 0) return;

  Serial.print("BT command received: ");
  Serial.println(command);

  // ---- Face expressions ----
  if (command == "happy") {
    happyFace();
  } else if (command == "angry") {
    angryFace();
  } else if (command == "surprised") {
    surprisedFace();
  } else if (command == "sad") {
    sadFace();
  } else if (command == "sleepy") {
    sleepyFace();
  } else if (command == "love") {
    loveFace();
  } else if (command == "laugh") {
    laughingFace();
  }

  // ---- Motor movement ----
  else if (command == "forward" || command == "f") {
    moveForward();
  } else if (command == "backward" || command == "b") {
    moveBackward();
  } else if (command == "left" || command == "l") {
    rotateLeft();
  } else if (command == "right" || command == "r") {
    rotateRight();
  } else if (command == "stop" || command == "s") {
    stopMotors();
  }
  // ---- Display other text for 3 seconds ----
  else {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(5);
    tft.setTextWrap(true);
    tft.setCursor(10, 50);
    tft.println(command);

    delay(3000);

    happyFace();
  }
}

// ================= Motor control functions =================

// Both motors spin forward -> robot moves forward
void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, MOTOR_SPEED);
  analogWrite(ENB, MOTOR_SPEED);
}

// Both motors spin backward -> robot moves backward
void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, MOTOR_SPEED);
  analogWrite(ENB, MOTOR_SPEED);
}

// Left motor backward, right motor forward -> robot rotates left in place
void rotateLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, MOTOR_SPEED);
  analogWrite(ENB, MOTOR_SPEED);
}

// Left motor forward, right motor backward -> robot rotates right in place
void rotateRight() {
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, MOTOR_SPEED);
  analogWrite(ENB, MOTOR_SPEED);
}

// Cuts power to both motors
void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// ================= Face drawing functions (unchanged) =================

// Draws a sleepy face: closed half-circle eyes, small smile, "zzZ" above right eye
void sleepyFace() {
  tft.fillScreen(ST77XX_BLACK);
  int screenW = tft.width();
  int screenH = tft.height();

  // ---- Eyes (closed, drawn as downward half-circle arcs = eyelids) ----
  int eyeWidth   = 40;
  int eyeSpacing = 20;
  int eyeRadius  = eyeWidth / 2;
  int eyeThickness = 5;

  int totalWidth = (eyeWidth * 2) + eyeSpacing;
  int eyesStartX = (screenW - totalWidth) / 2;
  int eyesCenterY = (screenH / 2) - 20;   // moved higher (was centered, now -20)

  int leftEyeCenterX  = eyesStartX + eyeRadius;
  int rightEyeCenterX = eyesStartX + eyeWidth + eyeSpacing + eyeRadius;

  drawSmoothArc(leftEyeCenterX, eyesCenterY, eyeRadius, 20, 160, eyeThickness, ST77XX_YELLOW);
  drawSmoothArc(rightEyeCenterX, eyesCenterY, eyeRadius, 20, 160, eyeThickness, ST77XX_YELLOW);

  // ---- Mouth (small smile, smaller radius than happyFace) ----
  int mouthCenterX = screenW / 2;
  int mouthCenterY = eyesCenterY + eyeRadius + 25;
  int mouthRadius  = 14;
  int mouthThickness = 3;

  drawSmoothArc(mouthCenterX, mouthCenterY, mouthRadius, 20, 160, mouthThickness, ST77XX_YELLOW);

  // ---- "zzZ" text in a row, left to right, just above the right eye ----
  tft.setTextColor(ST77XX_YELLOW);
  int zBaseY = eyesCenterY - eyeRadius - 10;   // just above the eye arc
  int zStartX = rightEyeCenterX - eyeRadius + 25;   // start roughly at left edge of right eye

  tft.setTextSize(1);
  tft.setCursor(zStartX, zBaseY + 6);
  tft.print("z");

  tft.setTextSize(1);
  tft.setCursor(zStartX + 10, zBaseY + 2);
  tft.print("z");

  tft.setTextSize(2);
  tft.setCursor(zStartX + 20, zBaseY - 4);
  tft.print("Z");
}

// Draws an angry face: red eyes, tilted eyebrows, straight dash mouth
void angryFace() {
  tft.fillScreen(ST77XX_BLACK);
  int screenW = tft.width();
  int screenH = tft.height();

  // ---- Eyes ----
  int eyeWidth   = 40;
  int eyeHeight  = 48;
  int eyeSpacing = 20;
  int cornerRadius = 14;
  int totalWidth = (eyeWidth * 2) + eyeSpacing;
  int eyesStartX = (screenW - totalWidth) / 2;
  int eyesStartY = (screenH / 2) - eyeHeight + 20;   // <-- moved everything lower

  int leftEyeX  = eyesStartX;
  int rightEyeX = eyesStartX + eyeWidth + eyeSpacing;
  tft.fillRoundRect(leftEyeX, eyesStartY, eyeWidth, eyeHeight, cornerRadius, ST77XX_RED);
  tft.fillRoundRect(rightEyeX, eyesStartY, eyeWidth, eyeHeight, cornerRadius, ST77XX_RED );

  // ---- Eyebrows ----
  int browThickness = 5;
  int browLength = eyeWidth-1;
  int browGapAboveEye = 15;
  int browTiltDrop = 16;
  int browY = eyesStartY - browGapAboveEye;

  int leftBrowOuterX = leftEyeX - 4;
  int leftBrowInnerX = leftEyeX + browLength + 4;
  drawThickLine(leftBrowOuterX, browY - browTiltDrop, leftBrowInnerX, browY + browTiltDrop, browThickness, ST77XX_RED);

  int rightBrowInnerX = rightEyeX - 4;
  int rightBrowOuterX = rightEyeX + browLength + 4;
  drawThickLine(rightBrowInnerX, browY + browTiltDrop, rightBrowOuterX, browY - browTiltDrop, browThickness, ST77XX_RED);

  // ---- Mouth ----
  int mouthCenterX = screenW / 2;
  int mouthCenterY = eyesStartY + eyeHeight + 30;
  int mouthHalfWidth = 18;
  int mouthThickness = 5;
  drawThickLine(mouthCenterX - mouthHalfWidth, mouthCenterY, mouthCenterX + mouthHalfWidth, mouthCenterY, mouthThickness, ST77XX_RED);
}

// Helper: draws a thick straight line between two points using stacked circles
void drawThickLine(int x0, int y0, int x1, int y1, int thickness, uint16_t color) {
  float dx = x1 - x0;
  float dy = y1 - y0;
  float length = sqrt(dx * dx + dy * dy);
  int steps = (int)length;

  for (int i = 0; i <= steps; i++) {
    float t = (float)i / steps;
    int x = x0 + t * dx;
    int y = y0 + t * dy;
    tft.fillCircle(x, y, thickness / 2, color);
  }
}

// Draws a smooth arc by plotting a thick curved line of small circles
void drawSmoothArc(int cx, int cy, int radius, float startAngle, float endAngle, int thickness, uint16_t color) {
  for (float a = startAngle; a <= endAngle; a += 1.0) {
    float rad = a * PI / 180.0;
    int x = cx + radius * cos(rad);
    int y = cy + radius * sin(rad);
    tft.fillCircle(x, y, thickness / 2, color);
  }
}

// Draws a neat happy face: two big rounded eyes + a small smiling mouth below
void happyFace() {
  tft.fillScreen(ST77XX_BLACK);

  int screenW = tft.width();
  int screenH = tft.height();

  // ---- Eyes ----
  int eyeWidth   = 40;
  int eyeHeight  = 48;
  int eyeSpacing = 20;
  int cornerRadius = 14;

  int totalWidth = (eyeWidth * 2) + eyeSpacing;
  int eyesStartX = (screenW - totalWidth) / 2;
  int eyesStartY = (screenH / 2) - eyeHeight - 6;   // sit eyes in upper-middle area

  int leftEyeX  = eyesStartX;
  int rightEyeX = eyesStartX + eyeWidth + eyeSpacing;

  tft.fillRoundRect(leftEyeX, eyesStartY, eyeWidth, eyeHeight, cornerRadius, ST77XX_CYAN);
  tft.fillRoundRect(rightEyeX, eyesStartY, eyeWidth, eyeHeight, cornerRadius, ST77XX_CYAN);

  // ---- Mouth (small smiling arc below the eyes) ----
  int mouthCenterX = screenW / 2;
  int mouthCenterY = eyesStartY + eyeHeight + 30;   // below the eyes
  int mouthRadius  = 22;
  int mouthThickness = 4;

  // Arc from 20° to 160° draws a smiling "U" shape (screen Y grows downward)
  drawSmoothArc(mouthCenterX, mouthCenterY, mouthRadius, 20, 160, mouthThickness, ST77XX_CYAN);
}

// Draws a surprised face: centered eyes with light-blue pupils looking right, gap at top, round open mouth
void surprisedFace() {
  tft.fillScreen(ST77XX_BLACK);
  int screenW = tft.width();
  int screenH = tft.height();

  // ---- Eyes (outer white shape, centered) ----
  int eyeWidth   = 40;
  int eyeHeight  = 48;
  int eyeSpacing = 20;
  int cornerRadius = 14;

  int topGap = 25;   // gap from top of screen before eyes start
  int eyesStartY = topGap;

  int totalWidth = (eyeWidth * 2) + eyeSpacing;
  int eyesStartX = (screenW - totalWidth) / 2;   // centered like happy/angry face

  int leftEyeX  = eyesStartX;
  int rightEyeX = eyesStartX + eyeWidth + eyeSpacing;

  tft.fillRoundRect(leftEyeX, eyesStartY, eyeWidth, eyeHeight, cornerRadius, ST77XX_WHITE);
  tft.fillRoundRect(rightEyeX, eyesStartY, eyeWidth, eyeHeight, cornerRadius, ST77XX_WHITE);

  // ---- Pupils (light blue, shifted toward right edge inside each eye) ----
  int pupilRadius = 9;
  int pupilRightMargin = 10;   // distance from right edge of eye shape
  int pupilTopMargin = 12;      // distance from top edge of eye shape
  int pupilCenterY = eyesStartY + pupilTopMargin;    // <-- now shifted toward top instead of center

  int leftPupilX  = leftEyeX + eyeWidth - pupilRightMargin;
  int rightPupilX = rightEyeX + eyeWidth - pupilRightMargin;

  tft.fillCircle(leftPupilX, pupilCenterY, pupilRadius, 0x841F);   // light blue
  tft.fillCircle(rightPupilX, pupilCenterY, pupilRadius, 0x841F);  // light blue

  // ---- Mouth (small round open "O" shape) ----
  int mouthCenterX = screenW / 2;
  int mouthCenterY = eyesStartY + eyeHeight + 30;
  int mouthOuterRadius = 12;
  int mouthInnerRadius = 6;

  tft.fillCircle(mouthCenterX, mouthCenterY, mouthOuterRadius, ST77XX_WHITE);
  tft.fillCircle(mouthCenterX, mouthCenterY, mouthInnerRadius, ST77XX_BLACK);
}

// Draws a sad face: neat eyes + a downward curving frown mouth
void sadFace() {
  tft.fillScreen(ST77XX_BLACK);
  int screenW = tft.width();
  int screenH = tft.height();

  // ---- Eyes ----
  int eyeWidth   = 40;
  int eyeHeight  = 48;
  int eyeSpacing = 20;
  int cornerRadius = 14;

  int totalWidth = (eyeWidth * 2) + eyeSpacing;
  int eyesStartX = (screenW - totalWidth) / 2;
  int eyesStartY = (screenH / 2) - eyeHeight - 6;

  int leftEyeX  = eyesStartX;
  int rightEyeX = eyesStartX + eyeWidth + eyeSpacing;

  tft.fillRoundRect(leftEyeX, eyesStartY, eyeWidth, eyeHeight, cornerRadius, ST77XX_BLUE);
  tft.fillRoundRect(rightEyeX, eyesStartY, eyeWidth, eyeHeight, cornerRadius, ST77XX_BLUE);

  // ---- Mouth (small frowning arc below the eyes, curving downward) ----
  int mouthCenterX = screenW / 2;
  int mouthCenterY = eyesStartY + eyeHeight + 45;   // pushed a bit lower since arc opens upward
  int mouthRadius  = 22;
  int mouthThickness = 4;

  // Arc from 200° to 340° draws a frowning "∩" shape (opposite of the happy smile arc)
  drawSmoothArc(mouthCenterX, mouthCenterY, mouthRadius, 200, 340, mouthThickness, ST77XX_BLUE);
}
void loveFace() {
  tft.fillScreen(ST77XX_BLACK);

  int screenW = tft.width();
  int screenH = tft.height();

  int eyeY = screenH / 2 - 20;
  int leftEyeX = screenW / 2 - 35;
  int rightEyeX = screenW / 2 + 35;

  // Left heart eye
  tft.fillCircle(leftEyeX - 6, eyeY, 10, ST77XX_RED);
  tft.fillCircle(leftEyeX + 6, eyeY, 10, ST77XX_RED);
  tft.fillTriangle(
    leftEyeX - 16, eyeY + 3,
    leftEyeX + 16, eyeY + 3,
    leftEyeX, eyeY + 22,
    ST77XX_RED
  );

  // Right heart eye
  tft.fillCircle(rightEyeX - 6, eyeY, 10, ST77XX_RED);
  tft.fillCircle(rightEyeX + 6, eyeY, 10, ST77XX_RED);
  tft.fillTriangle(
    rightEyeX - 16, eyeY + 3,
    rightEyeX + 16, eyeY + 3,
    rightEyeX, eyeY + 22,
    ST77XX_RED
  );

  // Smiling mouth
  int mouthCenterX = screenW / 2;
  int mouthCenterY = eyeY + 48;
  drawSmoothArc(
    mouthCenterX, mouthCenterY,
    22, 20, 160, 4, ST77XX_RED
  );
}
void laughingFace() {
  tft.fillScreen(ST77XX_BLACK);

  int screenW = tft.width();
  int screenH = tft.height();

  int eyeY = screenH / 2 - 25;
  int leftEyeX = screenW / 2 - 35;
  int rightEyeX = screenW / 2 + 35;

  // Closed eyes from laughing
  drawSmoothArc(
    leftEyeX, eyeY, 15, 200, 340, 5, ST77XX_YELLOW
  );

  drawSmoothArc(
    rightEyeX, eyeY, 15, 200, 340, 5, ST77XX_YELLOW
  );

  // Tears of laughter
  tft.fillCircle(leftEyeX - 22, eyeY + 12, 5, ST77XX_CYAN);
  tft.fillTriangle(
    leftEyeX - 27, eyeY + 18,
    leftEyeX - 17, eyeY + 18,
    leftEyeX - 22, eyeY + 5,
    ST77XX_CYAN
  );

  tft.fillCircle(rightEyeX + 22, eyeY + 12, 5, ST77XX_CYAN);
  tft.fillTriangle(
    rightEyeX + 17, eyeY + 18,
    rightEyeX + 27, eyeY + 18,
    rightEyeX + 22, eyeY + 5,
    ST77XX_CYAN
  );

   // ---- Big laughing mouth ----
  int mouthCenterX = screenW / 2;
  int mouthCenterY = eyeY + 48;

  // Outer mouth
  tft.fillRoundRect(
    mouthCenterX - 30, mouthCenterY - 5,
    60, 34, 12, ST77XX_WHITE
  );

  // Open mouth interior
  tft.fillRoundRect(
    mouthCenterX - 25, mouthCenterY + 2,
    50, 25, 9, ST77XX_RED
  );

  // Upper teeth
  tft.fillRoundRect(
    mouthCenterX - 20, mouthCenterY,
    40, 9, 4, ST77XX_WHITE
  );

  // Tongue at the bottom
  tft.fillRoundRect(
    mouthCenterX - 12, mouthCenterY + 17,
    24, 8, 4, ST77XX_MAGENTA
  );
}