#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "GyverTimer.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     4

#define PREDEL 200
#define BATTERY_MIN 3.2
#define BATTERY_MAX 4.0

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool gasTimerRun = false;
GTimer_ms gasTimer;
bool batteryTimerRun = false;
GTimer_ms batteryTimer;
GTimer_ms graphTimer;

int i = 0;
int arrGraph [SCREEN_WIDTH];
bool batteryCircle = false;
bool gasCircle = false;
int mq4 = 0;
float batteryPowerVolt = BATTERY_MAX;

void setup() {
  
  while(display.begin(SSD1306_SWITCHCAPVCC, 0x3C) == false) {}
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  gasTimer.setInterval(300);
  gasTimer.stop();
  
  batteryTimer.setInterval(1000);
  batteryTimer.stop();
  
  graphTimer.setInterval(1000);

  pinMode(9, OUTPUT);

  for(int i = 0; i < SCREEN_WIDTH; i++) {
    arrGraph[i] = -1;
  }
  
}

void loop() {

  if(graphTimer.isReady()) {
    
    if(i == SCREEN_WIDTH) i = 0;
    
    mq4 = analogRead(A2);
    arrGraph[i++] = mq4;

    batteryPowerVolt = analogRead(A1) * (5.077 / 1023.0) * 2.0;

    if(batteryPowerVolt < BATTERY_MIN) {
      if(batteryTimerRun == false) {
        batteryTimer.reset();
        batteryTimer.start();
        batteryTimerRun = true;
      }
    }
    
    display.clearDisplay();
    display.setCursor(20, 0);
    display.print("C3H6: ");
    display.print(map(mq4, 0, 1023, 0, 100));
    display.print("%");
    drawGraph();
    drawProgressBattary(batteryPowerVolt);
    display.display();
  }

  if (batteryTimer.isReady()){
    batteryCircle = !batteryCircle;
    if(batteryCircle) digitalWrite(9, HIGH);
    else digitalWrite(9, LOW);
    if(batteryPowerVolt > BATTERY_MIN) {
      batteryTimer.stop();
      batteryTimerRun = false;
      digitalWrite(9, LOW);
    }
  }

  if (gasTimer.isReady()){
    gasCircle = !gasCircle;
    if(gasCircle) digitalWrite(9, HIGH);
    else digitalWrite(9, LOW);
    if(mq4 < PREDEL) {
      gasTimer.stop();
      gasTimerRun = false;
      digitalWrite(9, LOW);
    }
  }

  if(mq4 >= PREDEL) {
    if(gasTimerRun == false) {
      gasTimer.reset();
      gasTimer.start();
      gasTimerRun = true;
    }
  }

}

void drawProgressBattary(float batt) {
  int proc = map(batt * 1000, BATTERY_MIN * 1000, BATTERY_MAX * 1000, 0, 100);
  int wProg = 10;
  int hProg = 4;
  int x = SCREEN_WIDTH - wProg - 1;
  int y = 0;
  display.drawFastHLine(x, 0, wProg, SSD1306_WHITE);
  display.drawFastHLine(x, hProg, wProg, SSD1306_WHITE);
  display.drawFastVLine(x, 0, hProg, SSD1306_WHITE);
  display.drawFastVLine(SCREEN_WIDTH-1, 0, hProg+1, SSD1306_WHITE);
  for(int i = 0; i < hProg; i++) {
    display.drawFastHLine(x, i, wProg * proc / 100, SSD1306_WHITE);
  }
  display.setCursor(x - 27, 0);
  display.print(proc);
  display.print("%");
}

void drawGraph() {
  display.drawFastHLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH, SSD1306_WHITE);
  display.drawFastVLine(0, 20, SCREEN_HEIGHT, SSD1306_WHITE);
  for(int x = 0; x < SCREEN_WIDTH; x++) {
    display.drawPixel(x, SCREEN_HEIGHT - map(arrGraph[x], 0, 1023, 0, SCREEN_HEIGHT - 20), SSD1306_WHITE);
  }
}
