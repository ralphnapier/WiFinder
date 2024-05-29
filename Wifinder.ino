#include <SPI.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Config.h" // Include the common header file
#include "InfoScreen.h" // Include the InfoScreen header file

#define BATTERY_PIN A0   // ADC pin for battery voltage monitoring
#define SD_CS_PIN D8     // CS pin for the microSD card module

#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// Initialize the OLED display with the custom SDA and SCL pins
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Key icon bitmap (8x8)
const unsigned char key_bitmap [] PROGMEM = {
  0b00111100,
  0b01000010,
  0b01000010,
  0b11111111,
  0b10000001,
  0b10010001,
  0b10000001,
  0b11111111
};

// Function declarations
float readBatteryVoltage();
void logScanResults(float voltage);
void displayNetworks();
void enterLightSleep(uint32_t sleepTimeSeconds);

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing WiFi...");
  WiFi.mode(WIFI_STA);

  // Initialize I2C with custom SDA and SCL pins
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialize OLED display with I2C address 0x3C
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  pinMode(BATTERY_PIN, INPUT); // Setup the battery monitoring pin
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.println(F("Setup done!"));
  display.display();
}

void loop() {
  float voltage = readBatteryVoltage();

  displayInfoScreen(voltage);

  logScanResults(voltage);

  displayNetworks();

  enterLightSleep(120); // Enter light sleep for 120 seconds

  delay(1000); // Delay before rescanning
}

float readBatteryVoltage() {
  int adcValue = analogRead(BATTERY_PIN);
  return adcValue * (9.0 / 1023.0); // Convert ADC value to voltage (scaled for 9V)
}

void logScanResults(float voltage) {
  // Scan for WiFi networks
  int n = WiFi.scanNetworks();
  Serial.println("Scan done!");
/*
  // Log the findings to SD card
  File dataFile = SD.open("wifi_log.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Scan results:");
    dataFile.println("Battery: " + String(voltage, 2) + " V");
    for (int i = 0; i < n; ++i) {
      String ssid = WiFi.SSID(i);
      if (ssid.length() > 15) {
        ssid = ssid.substring(0, 15); // Truncate SSID to 15 characters
      }
      dataFile.print(i + 1);
      dataFile.print(": ");
      dataFile.print(ssid);
      if (WiFi.encryptionType(i) != AUTH_OPEN) {
        dataFile.print(" (locked)");
      }
      dataFile.println();
    }
    dataFile.close();
    Serial.println("Data written to SD card.");
  } else {
    Serial.println("error opening wifi_log.txt");
  }
*/
}

void displayNetworks() {
  int n = WiFi.scanNetworks();
  int currentPage = 0;
  int networksPerPage = 6;
  int totalPages = (n + networksPerPage - 1) / networksPerPage; // Round up to the next whole number

  while (currentPage < totalPages) {
    display.clearDisplay();

    String networksFound;
    if (n == 1) {
      networksFound = "1 network found";
    } else {
      networksFound = String(n) + " networks found";
    }
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(networksFound, 0, 0, &x1, &y1, &w, &h);
    int centeredX = (SCREEN_WIDTH - w) / 2;

    display.setCursor(centeredX, 0);
    display.println(networksFound);
    Serial.println(networksFound);

    int lineHeight = 8; // Height of each line of text in pixels
    int linesPerPage = min(networksPerPage, n - currentPage * networksPerPage); // Calculate how many lines fit on the screen

    // Add a vertical space of about 2 pixels
    display.setCursor(0, lineHeight + 2);

    for (int i = 0; i < linesPerPage; ++i) {
      int networkIndex = currentPage * networksPerPage + i;
      display.setCursor(0, (i + 1) * lineHeight + 2); // Adjust cursor position for each network
      display.print(networkIndex + 1);
      display.print(": ");
      if (WiFi.encryptionType(networkIndex) != AUTH_OPEN) {
        display.drawBitmap(display.getCursorX(), display.getCursorY() - 1, key_bitmap, 8, 8, WHITE);
        display.setCursor(display.getCursorX() + 10, display.getCursorY()); // Move cursor to the right after the key icon
      }
      String ssid = WiFi.SSID(networkIndex);
      if (ssid.length() > 15) {
        ssid = ssid.substring(0, 15); // Truncate SSID to 15 characters
      }
      display.print(ssid);
      display.println();

      Serial.print(networkIndex + 1);
      Serial.print(": ");
      if (WiFi.encryptionType(networkIndex) != AUTH_OPEN) {
        Serial.print("(locked) ");
      }
      Serial.print(ssid);
      Serial.println();

      delay(10);
    }

    display.display();
    delay(5000); // Delay between pages
    currentPage++;
  }
}

void enterLightSleep(uint32_t sleepTimeSeconds) {
  Serial.println("Entering light sleep...");

  // Disable WiFi
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1); // Short delay to ensure WiFi is off

  // Sleep for the specified duration
  uint32_t sleepTimeMs = sleepTimeSeconds * 1000; // Convert seconds to milliseconds
  delay(sleepTimeMs);

  // Re-enable WiFi
  WiFi.forceSleepWake();
  delay(1); // Short delay to ensure WiFi is back on
  WiFi.mode(WIFI_STA);

  Serial.println("Waking up from light sleep...");
}
