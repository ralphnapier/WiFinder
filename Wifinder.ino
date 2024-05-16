#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 //OLED display width, in pixels
#define SCREEN_HEIGHT 64 //OLED display height, in pixels
#define BATTERY_PIN A0   //ADC pin for battery voltage monitoring

#define OLED_RESET    -1 //Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///<See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

//Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Lock icon bitmap (8x8)
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

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing WiFi...");
  WiFi.mode(WIFI_STA);

  //Initialize OLED display with I2C address 0x3C
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  pinMode(BATTERY_PIN, INPUT); //Setup the battery monitoring pin A0
  display.clearDisplay();
  display.setTextSize(1);      //Normal 1:1 pixel scale
  display.setTextColor(WHITE); //Draw white text
  display.setCursor(0, 0);     //Start at top-left corner
  display.println(F("Setup done!"));
  display.display();
}

void loop() {
  int adcValue = analogRead(BATTERY_PIN);
  float voltage = adcValue * (9.0 / 1023.0); //Convert ADC value to voltage (scaled for 9V)

  //Display Scanning..., WiFinder, battery information, and designer information
  display.clearDisplay();

  //Display "Scanning..." in normal font
  display.setTextSize(1); // Set text size to normal
  String scanningText = "Scanning...";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(scanningText, 0, 0, &x1, &y1, &w, &h);
  int centeredX = (SCREEN_WIDTH - w) / 2;
  display.setCursor(centeredX, 0);
  display.println(scanningText);

  //Display "WiFinder" in large font
  display.setTextSize(2); // Set text size to 2 for larger font
  String title = "WiFinder";
  display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  centeredX = (SCREEN_WIDTH - w) / 2;
  display.setCursor(centeredX, 20); // Position with as much space as possible below "Scanning..."
  display.println(title);

  //Display battery information in normal font
  display.setTextSize(1); // Reset text size to normal
  String batteryInfo = "Battery: " + String(voltage, 2) + " V";
  display.getTextBounds(batteryInfo, 0, 0, &x1, &y1, &w, &h);
  centeredX = (SCREEN_WIDTH - w) / 2;
  display.setCursor(centeredX, 40); //Position below "WiFinder"
  display.println(batteryInfo);

  //Display designer information in normal font
  String designerInfo = "Designer: R Napier";
  display.getTextBounds(designerInfo, 0, 0, &x1, &y1, &w, &h);
  centeredX = (SCREEN_WIDTH - w) / 2;
  display.setCursor(centeredX, SCREEN_HEIGHT - h); //Position at the bottom
  display.println(designerInfo);

  display.display();

  Serial.print("Battery: ");
  Serial.print(voltage, 2);
  Serial.println(" V");

  //Delay to allow the user to read the battery information
  delay(2000);

  //Scan for WiFi networks
  int n = WiFi.scanNetworks();
  Serial.println("Scan done!");

  //Clear the display and prepare to show WiFi networks
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
    display.getTextBounds(networksFound, 0, 0, &x1, &y1, &w, &h);
    centeredX = (SCREEN_WIDTH - w) / 2;

    display.setCursor(centeredX, 0);
    display.println(networksFound);
    Serial.println(networksFound);

    int lineHeight = 8; //Height of each line of text in pixels
    int linesPerPage = min(networksPerPage, n - currentPage * networksPerPage); //Calculate how many lines fit on the screen

    //Add a vertical space of about 2 pixels
    display.setCursor(0, lineHeight + 2);

    for (int i = 0; i < linesPerPage; ++i) {
      int networkIndex = currentPage * networksPerPage + i;
      display.setCursor(0, (i + 1) * lineHeight + 2); //Adjust cursor position for each network
      display.print(networkIndex + 1);
      display.print(": ");
      if (WiFi.encryptionType(networkIndex) != AUTH_OPEN) {
        display.drawBitmap(display.getCursorX(), display.getCursorY() - 1, key_bitmap, 8, 8, WHITE);
        display.setCursor(display.getCursorX() + 10, display.getCursorY()); //Move cursor to the right after the key icon
      }
      String ssid = WiFi.SSID(networkIndex);
      if (ssid.length() > 15) {
        ssid = ssid.substring(0, 15); //Truncate SSID to 15 characters, Prevents wrapping
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
    delay(5000); //Delay between pages
    currentPage++;
  }

  delay(5000); //Delay before rescanning
}
