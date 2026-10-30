#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// TFT pin connections (change to match your wiring)
#define TFT_CS   5
#define TFT_RST  4
#define TFT_DC   2

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);

  tft.initR(INITR_BLACKTAB);   // try INITR_GREENTAB if colors/offset look wrong
  tft.setRotation(1);          // landscape: 160 wide x 128 high (better for a face)
  tft.fillScreen(ST77XX_BLACK);

}

void loop() {
  happyFace();
  delay(1000);
  angryFace();
  delay(1000);
  surprisedFace();
  delay(1000);
  sadFace();
  delay(1000);
  sleepyFace();
  delay(1000);
  // happyFace() called once in setup()
}

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
  tft.fillRoundRect(rightEyeX, eyesStartY, eyeWidth, eyeHeight, cornerRadius, ST77XX_RED);

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