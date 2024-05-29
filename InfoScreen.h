#ifndef INFOSCREEN_H
#define INFOSCREEN_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Config.h" // Include the common header file

extern Adafruit_SSD1306 display; // Declare the display object

void displayInfoScreen(float voltage) {
  display.clearDisplay();

  // Display "Scanning..." in normal font
  display.setTextSize(1);
  String scanningText = "Scanning...";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(scanningText, 0, 0, &x1, &y1, &w, &h);
  int centeredX = (SCREEN_WIDTH - w) / 2;
  display.setCursor(centeredX, 0);
  display.println(scanningText);

  // Display "WiFinder" in large font
  display.setTextSize(2);
  String title = "WiFinder";
  display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  centeredX = (SCREEN_WIDTH - w) / 2;
  display.setCursor(centeredX, 20);
  display.println(title);

  // Display battery information in normal font
  display.setTextSize(1);
  String batteryInfo = "Battery: " + String(voltage, 2) + " V";
  display.getTextBounds(batteryInfo, 0, 0, &x1, &y1, &w, &h);
  centeredX = (SCREEN_WIDTH - w) / 2;
  display.setCursor(centeredX, 40);
  display.println(batteryInfo);

  // Display designer information in normal font
  String designerInfo = "Designer: R Napier";
  display.getTextBounds(designerInfo, 0, 0, &x1, &y1, &w, &h);
  centeredX = (SCREEN_WIDTH - w) / 2;
  display.setCursor(centeredX, SCREEN_HEIGHT - h);
  display.println(designerInfo);

  display.display();

  Serial.print("Battery: ");
  Serial.print(voltage, 2);
  Serial.println(" V");

  delay(2000);
}

#endif
