#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>

#define YP A3
#define XM A2
#define YM 9
#define XP 8
#define TS_MINX 109
#define TS_MINY 138
#define TS_MAXX 934
#define TS_MAXY 914
#define MAXPRESSURE 1500
// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
// optional
#define LCD_RESET A4
// Color definitions - in 5:6:5
#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF
#define WATER           0x9FDF 
#define JJCOLOR         0x1CB6
#define JJORNG          0xFD03

#define LINE_COLOR      0x959A
#define BUTTON_COLOR    0x3415
#define BKGD_COLOR      0x2169
#define RECT_CONT       0x02D5


#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSansBold12pt7b.h"
#include "Fonts/FreeSansBold18pt7b.h"
#include "Fonts/FreeSansBold9pt7b.h"
#include "Fonts/Roboto_Medium12pt7b.h"
#include "Fonts/dosis_book8pt7b.h"

#define FREESANS9PT FreeSans9pt7b
#define FREESANSBOLD9PT FreeSansBold9pt7b
#define FREESANSBOLD12PT FreeSansBold12pt7b
#define FREESANSBOLD18PT FreeSansBold18pt7b
#define ROBOTO12PT Roboto_Medium12pt7b
#define DOSIS8pt dosis_book8pt7b

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
int i = 0;
int page = 0;
int blv;

bool pressure;


void drawHomeScreen() {

  tft.reset();

  uint16_t identifier = tft.readID();

  //Force it for Elegoo !?
  identifier = 0x9341;
  Serial.print(F("LCD driver chip: "));
  Serial.println(identifier, HEX);

  tft.begin(identifier);

  tft.setRotation(3);
  tft.fillScreen(BKGD_COLOR);


  //Temp Gauge
  tft.drawRect(4, 45, 91, 15, RECT_CONT);
  tft.fillRect(4, 45, 91, 15, JJCOLOR);

  tft.setFont(&FREESANSBOLD18PT);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);

  //Temp +
  tft.drawRect(4, 71, 91, 36, RECT_CONT);
  tft.fillRect(4, 71, 91, 36, BUTTON_COLOR);
  tft.setCursor(35, 95);
  tft.print("+");

  //Temp -
  tft.drawRect(4, 198, 91, 36, RECT_CONT);
  tft.fillRect(4, 198, 91, 36, BUTTON_COLOR);
  tft.setCursor(35, 225);
  tft.print("-");

  //Hygro Gauge
  tft.drawRect(112, 45, 91, 15, RECT_CONT);
  tft.fillRect(112, 45, 91, 15, JJCOLOR);

  //Hygro +
  tft.drawRect(112, 71, 91, 36, RECT_CONT);
  tft.fillRect(112, 71, 91, 36, BUTTON_COLOR);
  tft.setCursor(150, 95);
  tft.print("+");

  //Hygro -
  tft.drawRect(112, 198, 91, 36, RECT_CONT);
  tft.fillRect(112, 198, 91, 36, BUTTON_COLOR);
  tft.setCursor(150, 225);
  tft.print("-");


  tft.setFont(&ROBOTO12PT);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.setCursor(8, 135);
  tft.print("Temp"); //
  tft.setCursor(116, 135);
  tft.print("Hygro"); //

  //pH Mesure
  tft.drawRect(223, 71, 91, 36, RECT_CONT);
  tft.fillRect(223, 71, 91, 36, BUTTON_COLOR);
  tft.setFont(&FREESANSBOLD9PT);
  tft.setCursor(230, 95);
  tft.print("MESURE");

  tft.fillRect(223, 116, 91, 77, BLACK);
  tft.drawRect(223, 116, 91, 77, RECT_CONT);

  //Lines
  tft.drawLine(102, 0, 102, 240, LINE_COLOR);
  tft.drawLine(213, 0, 213, 240, LINE_COLOR);
  tft.drawLine(213, 42, 320, 42, LINE_COLOR);
}

void drawClock() {

  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= BLINK_INTERVAL_TEMPERATURE) {

    readRTC();
    
    previousMillis = currentMillis;

    String cl = "";
    cl = String(Clock.getDate(), DEC)
         + "/"
         + String(Clock.getMonth(Century), DEC)
         + "/"
         + String(Clock.getYear(), DEC)
         + " "
         + String(Clock.getHour(h12, PM), DEC)
         + ":"
         + String(Clock.getMinute(), DEC);

    tft.fillRect(213, 200, 107, 40, BKGD_COLOR);

    tft.setFont(&DOSIS8pt);
    tft.setTextColor(WHITE);
    tft.setTextSize(1);
    tft.setCursor(214, 220);
    tft.print(cl);
  }

}

void drawPHMesures() {

  tft.fillRect(223, 116, 91, 77, BLACK);
  tft.drawRect(223, 116, 91, 77, RECT_CONT);
}

void drawTemperature(float Temp) {

  char c[5];
  //Clear
  tft.fillRect(4, 5, 95, 33, BKGD_COLOR);
  tft.setFont(&FREESANSBOLD18PT);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.setCursor(4, 35);
  dtostrf(Temp, 4, 1, c);
  strcat(c, "c");
  tft.print(c); //
}

void drawTempWarm() {
  tft.drawRect(4, 45, 91, 15, RECT_CONT);
  tft.fillRect(4, 45, 91, 15, RED);
}

void drawTempCold() {
  tft.drawRect(4, 45, 91, 15, RECT_CONT);
  tft.fillRect(4, 45, 91, 15, CYAN);
}

void drawTempNull() {
  //Temp Gauge
  tft.drawRect(4, 45, 91, 15, RECT_CONT);
  tft.fillRect(4, 45, 91, 15, JJCOLOR);
}

void drawHygroOn() {
  tft.drawRect(112, 45, 91, 15, RECT_CONT);
  tft.fillRect(112, 45, 91, 15, WATER);
}
void drawHygroOff() {
  tft.drawRect(112, 45, 91, 15, RECT_CONT);
  tft.fillRect(112, 45, 91, 15, JJCOLOR);
}

void drawHygrometry(float Hygro) {

  char c[5];
  //Clear
  tft.fillRect(112, 5, 95, 33, BKGD_COLOR);
  tft.setFont(&FREESANSBOLD18PT);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.setCursor(112, 35);
  dtostrf(Hygro, 4, 1, c);
  strcat(c, "%");
  tft.print(c); //
}

void drawTargetTemperature(int Temp) {

  char c[5];
  //Clear
  tft.fillRect(20, 145, 60, 25, BKGD_COLOR);
  tft.setFont(&ROBOTO12PT);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.setCursor(20, 170);
  itoa(Temp, c, 10);
  strcat(c, "c");
  tft.print(c); //
}


void drawTargetHygrometry(int Hygro) {

  char c[5];
  //Clear
  tft.fillRect(128, 145, 60, 25, BKGD_COLOR);
  tft.setFont(&ROBOTO12PT);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.setCursor(128, 170);
  itoa(Hygro, c, 10);
  strcat(c, "%");
  tft.print(c); //
}


void drawCO2(float co2, float tvoc) {

  char c[15] = "CO2:";
  char t[15] = "TVOC:";
  char msg_co2[9] = "";
  char msg_tvoc[9] = "";

  dtostrf(ccs.geteCO2(), 5, 0, msg_co2);
  dtostrf(ccs.getTVOC(), 5, 0, msg_tvoc);
  strcat(c, msg_co2);
  strcat(c, "ppm");

  strcat(t, msg_tvoc);
  strcat(t, "ppb");

  tft.fillRect(225, 4, 95, 37, BKGD_COLOR);

  tft.setFont(&DOSIS8pt);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);

  tft.setCursor(225, 18);
  tft.print(c);

  tft.setCursor(225, 38);
  tft.print(t);

}


void drawPH(float ph) {

  char c[9] = "pH:";
  char msg_ph[5] = "";
  dtostrf(ph, 5, 1, msg_ph);

  strcat(c, msg_ph);

  tft.fillRect(225, 43, 95, 30, BKGD_COLOR);

  tft.setFont(&FREESANSBOLD12PT);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);

  tft.setCursor(225, 65);
  tft.print("--------");
  delay(500);
  tft.fillRect(225, 43, 95, 30, BKGD_COLOR);
  tft.setCursor(225, 65);
  tft.print(c);
}


// TOUCH

void touchDetect() {
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  int px, py;

  if (p.z > ts.pressureThreshhold && p.z < MAXPRESSURE) {
    py = map(p.x, TS_MINX, TS_MAXX, 0, tft.height());
    px = map(p.y, TS_MINY, TS_MAXY, 0, tft.width());
    pressure = true;
    Serial.print("X = "); Serial.print(px);
    Serial.print("\tY = "); Serial.print(py);
    Serial.print("\tPressure = "); Serial.println(p.z);
  } else {
    pressure = false;
  }


  if (pressure == true) {

    //TEMP
    if (px > 4 && px < 110 && py > 70 && py < 110) {
      config.temperature++;
      drawTargetTemperature(config.temperature);
      saveConfiguration("/config.ini", config);
    }


    if (px > 4 && px < 110 && py > 195 && py < 240) {
      config.temperature--;
      drawTargetTemperature(config.temperature);
      saveConfiguration("/config.ini", config);
    }

    //HYGRO
    if (px > 110 && px < 220 && py > 70 && py < 110) {
      config.humidity++;
      drawTargetHygrometry(config.humidity);
      saveConfiguration("/config.ini", config);
    }


    if (px > 110 && px < 220 && py > 195 && py < 240) {
      config.humidity--;
      drawTargetHygrometry(config.humidity);
      saveConfiguration("/config.ini", config);
    }

    //PH

    if (px > 220 && px < 320 && py > 70 && py < 110) {
      float ph = getPhMeterSensorValue();
    }

  }

  pressure = false;

}
