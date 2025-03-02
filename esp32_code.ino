#include <RTClib.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <GxEPD2_BW.h>
#include <QRCode_Library.h>
#include <Wire.h>
#include <Preferences.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <TOTP.h>
#include <ArduinoBearSSL.h>
//#include <AES128.h>
#include <Base64.h>  // For Base64 encoding
#include <Preferences.h>
#include <mbedtls/aes.h>
#include <mbedtls/base64.h>
#include <string.h>

// SETUP
String baseUrl = "http://192.168.66.185:5000"; // Dynamic base URL
const char *accountId = "alice123";  // The account ID to be used in the QR code
const char *key = "JBSWY3DPEHPK3PXP";




Preferences preferences;

// Create an instance of the DS3231 RTC
RTC_DS3231 rtc;

// Create an instance of the TinyGPS++ object
TinyGPSPlus gps;

// Define the serial connection for the GPS module
SoftwareSerial gpsSerial(16, 17); // TX, RX

// Define the GPIO pin to control GPS power
#define GPS_POWER_PIN 4 // Change this to the GPIO pin you're using

#define CS_PIN    26
#define DC_PIN    25
#define RST_PIN   33 //ACHTUNG FALSCH BESCHRIFTET!!
#define BUSY_PIN  27

// Define the display type (1.54" b/w)
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(/*CS=*/ CS_PIN, /*DC=*/ DC_PIN, /*RST=*/ RST_PIN, /*BUSY=*/ BUSY_PIN));

void drawQRCode(const char *text);
void displaySettingUpMessage();
void setRTCTimeFromGPS();

const char* spinnerFrames[] = {"|", "/", "-", "\\"};
int currentFrame = 0;

unsigned long setupStartTime = millis();
const unsigned long timeoutDuration = 5 * 60 * 1000; // 5 minutes
const unsigned long animationInterval = 5000; // Update animation every 1 second
unsigned long lastAnimationUpdate = 0;

// enter your hmacKey (10 digits)
uint8_t hmacKey[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x21, 0xde, 0xad, 0xbe, 0xef};
TOTP totp = TOTP(hmacKey, 10);

String generateTOTP();

String totpCode = String("");

// Function to convert DateTime to Unix timestamp
unsigned long dateTimeToUnixTimestamp(const DateTime& dt) {
  tm tmDt;
  tmDt.tm_year = dt.year() - 1900;
  tmDt.tm_mon = dt.month() - 1;
  tmDt.tm_mday = dt.day();
  tmDt.tm_hour = dt.hour();
  tmDt.tm_min = dt.minute();
  tmDt.tm_sec = dt.second();
  tmDt.tm_isdst = 0; // No daylight saving time

  return mktime(&tmDt);
}

void setup() {
  Serial.begin(115200);

  // Initialize the GPS power control pin
  pinMode(GPS_POWER_PIN, OUTPUT);
  digitalWrite(GPS_POWER_PIN, HIGH); // Turn on the GPS module

  // Initialize preferences
  preferences.begin("totp", false); // Open NVS namespace "totp" in read/write mode

  // Initialize totpCode from NVS (or set to empty string if not found)
  totpCode = preferences.getString("totpCode", "");

  
  gpsSerial.begin(9600); // Start the GPS serial connection
  SPI.begin(18, -1, 23, -1);  // SCK=18, MISO not used (-1), MOSI=23, CS=-1
  display.init(115200, false, 10, false); // Initialize the e-ink display

  // Initialize the RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    display.clearScreen();
    display.setCursor(10, 30);
    display.print("RTC not found. Halting.");
    while (1); // Halt if RTC is not found
  }

  // Check the reset reason
  esp_reset_reason_t resetReason = esp_reset_reason();

  // Only set the RTC time from GPS on power-up or software reset
  if (resetReason == ESP_RST_POWERON || resetReason == ESP_RST_SW) {
    preferences.putString("totpCode", ""); // Clear the TOTP code in NVS

    displaySettingUpMessage();
    
    // Wait for GPS to get a fix or timeout
    bool gpsFixAcquired = false;
    bool gpsLocationValid = false;
    while (millis() - setupStartTime < timeoutDuration) {
      // Check for GPS data
      while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
      }

      // Check if GPS has a valid fix (time and date)
      if (gps.time.isValid() && gps.date.isValid()) {
        gpsFixAcquired = true;

        // Check if GPS has valid location data
        if (gps.location.isValid()) {
          gpsLocationValid = true;
          break; // Exit the loop if GPS fix and location are acquired
        }
      }

      // Update the spinner animation every second
      if (millis() - lastAnimationUpdate >= animationInterval) {
        updateSpinner(currentFrame);
        currentFrame = (currentFrame + 1) % 4; // Cycle through frames
        lastAnimationUpdate = millis();
      }
    }
    
    // Check if the GPS fix and location were acquired
    if (gpsFixAcquired && gpsLocationValid) {
      // Set the RTC time from the GPS
      rtc.adjust(DateTime(gps.date.year(), gps.date.month(), gps.date.day(),
                 gps.time.hour(), gps.time.minute(), gps.time.second()));
      Serial.println("RTC time set from GPS");

      // Calculate the delay to align with the next 30-second interval
      int currentSecond = gps.time.second(); // Get the current second from GPS
      int delaySeconds = (30 - (currentSecond % 30)) % 30; // Calculate remaining seconds

      // Turn off the GPS module to save power
      digitalWrite(GPS_POWER_PIN, LOW);

      Serial.println("GPS module turned off.");
      
      // Add the delay
      if (delaySeconds > 0) {
          Serial.print("Delaying for ");
          Serial.print(delaySeconds);
          Serial.println(" seconds to align with the next 30-second interval...");
          delay(delaySeconds * 1000); // Convert seconds to milliseconds
      }
                
      // Read the TOTP code from NVS
      totpCode = preferences.getString("totpCode", "");
      
      // Print the saved TOTP code (for debugging)
      Serial.print("Saved TOTP Code: ");
      Serial.println(totpCode);
      //preferences.putString("totpCode", totpCode);
      
    } else {
      // Timeout or invalid location occurred
      display.clearScreen();
      display.setCursor(10, 30);
      if (!gpsFixAcquired) {
        display.print("GPS signal not found.");
      } else if (!gpsLocationValid) {
        display.print("GPS location invalid.");
      }
      display.display(true); // Full refresh
      Serial.println("GPS signal or location not found within timeout.");
    }
  }

  // Hibernate the display to save power
  display.hibernate();
}

void loop() {
  // Get the current time from the RTC
  DateTime currentTime = rtc.now();

  // Check if the RTC time is valid
  if (!currentTime.isValid()) {
    Serial.println("RTC time is invalid!");
    return;
  }

  // Format and print the time
  char str[20];   // Declare a string as an array of chars
  sprintf(str, "%d/%d/%d %d:%d:%d",     // %d allows to print an integer to the string
          currentTime.year(),   // Get year method
          currentTime.month(),  // Get month method
          currentTime.day(),    // Get day method
          currentTime.hour(),   // Get hour method
          currentTime.minute(), // Get minute method
          currentTime.second()  // Get second method
         );

  // Serial.println(str); // Print the string to the serial port

  // Convert DateTime to Unix timestamp
  unsigned long unixTime = dateTimeToUnixTimestamp(currentTime);
  // Generate TOTP code
  String newCode = String(totp.getCode(unixTime));

  if (totpCode != newCode) {
    totpCode = String(newCode);
    Serial.print("TOTP code: ");
    Serial.println(newCode);

    // Save the TOTP code to NVS
    preferences.putString("totpCode", totpCode);

    const char *plain_text = totpCode.c_str();
    char output[256];
    size_t output_len = sizeof(output);

    int ret = encrypt_totp(key, plain_text, output, &output_len);

    // Generate the URL with the encrypted TOTP code
    String url = baseUrl + "/validate?account_id=" + String(accountId) + "&totp_enc=" + String(output);
    
    // Print the URL to the serial monitor for debugging
    Serial.println("Generated URL: " + url);

    // Convert the URL to a char array for the QR code library
    char urlBuffer[150];  // Ensure this buffer is large enough
    url.toCharArray(urlBuffer, sizeof(urlBuffer));

    // Draw the QR code on the display
    drawQRCode(urlBuffer);

    // Put the display into low-power mode
    display.hibernate();

    // Enter deep sleep
    Serial.println("Entering deep sleep...");
    esp_deep_sleep(27e6); // Sleep for 27 seconds (27e6 microseconds)
  }
}

void pad_data(unsigned char *data, size_t *data_len) {
    size_t block_size = 16;
    size_t padding_len = block_size - (*data_len % block_size);
    memset(data + *data_len, padding_len, padding_len);
    *data_len += padding_len;
}

int encrypt_totp(const char *key, const char *plain_text, char *output, size_t *output_len) {
    mbedtls_aes_context aes;
    unsigned char key_bytes[32];
    unsigned char plaintext[256];
    unsigned char ciphertext[256];
    size_t plaintext_len = strlen(plain_text);
    size_t ciphertext_len;
    size_t base64_len;
    int ret;

    // Ensure the key is 32 bytes long
    memset(key_bytes, 0, sizeof(key_bytes));
    size_t key_len = strlen(key);
    if (key_len > 32) key_len = 32;
    memcpy(key_bytes, key, key_len);

    // Copy plaintext and pad it
    memcpy(plaintext, plain_text, plaintext_len);
    pad_data(plaintext, &plaintext_len);

    // Initialize AES context
    mbedtls_aes_init(&aes);
    ret = mbedtls_aes_setkey_enc(&aes, key_bytes, 256);
    if (ret != 0) {
        mbedtls_aes_free(&aes);
        return ret;
    }

    // Encrypt the plaintext
    ret = mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, plaintext, ciphertext);
    if (ret != 0) {
        mbedtls_aes_free(&aes);
        return ret;
    }

    // Base64 encode the ciphertext
    ciphertext_len = plaintext_len;  // AES ECB mode preserves length
    ret = mbedtls_base64_encode((unsigned char *)output, *output_len, &base64_len, ciphertext, ciphertext_len);
    if (ret != 0) {
        mbedtls_aes_free(&aes);
        return ret;
    }

    // Make the base64 output URL-safe
    for (size_t i = 0; i < base64_len; i++) {
        if (output[i] == '+') output[i] = '-';
        if (output[i] == '/') output[i] = '_';
    }

    *output_len = base64_len;
    mbedtls_aes_free(&aes);
    return 0;
}

void updateSpinner(int currentFrame) {
  // Define the offset to move the spinner down (e.g., 20 pixels)
  int yOffset = 20;

  // Adjust the partial window position
  display.setPartialWindow(50, 50 + yOffset, 20, 20); // Move the window down by yOffset

  // Display the current frame
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE); // Clear the spinner area
    display.setCursor(50, 65 + yOffset); // Move the cursor down by yOffset
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.print(spinnerFrames[currentFrame]); // Print the current frame
  } while (display.nextPage());
}
void drawQRCode(const char *text)
{
  // Create the QR code
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(5)];
  qrcode_initText(&qrcode, qrcodeData, 5, 0, text);
  Serial.println("printing qr");
  // Calculate the size of each QR code module (pixel) to fill the screen
  uint16_t screenWidth = display.width();
  uint16_t screenHeight = display.height();
  uint16_t qrSize = qrcode.size;
  uint16_t moduleSize = min(screenWidth, screenHeight) / qrSize;

  // Center the QR code on the screen
  uint16_t xOffset = (screenWidth - (qrSize * moduleSize)) / 2;
  uint16_t yOffset = (screenHeight - (qrSize * moduleSize)) / 2;

  // Draw the QR code
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    for (uint8_t y = 0; y < qrSize; y++)
    {
      for (uint8_t x = 0; x < qrSize; x++)
      {
        if (qrcode_getModule(&qrcode, x, y))
        {
          display.fillRect(xOffset + x * moduleSize, yOffset + y * moduleSize, moduleSize, moduleSize, GxEPD_BLACK);
        }
      }
    }
  }
  while (display.nextPage());
}

void displaySettingUpMessage() {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(10, 30);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.println("Searching for GPS signal...");
  } while (display.nextPage());
}

void checkTimeout() {
  if (millis() - setupStartTime > timeoutDuration) {
    // Timeout occurred
    display.clearScreen();
    display.setCursor(10, 30);
    display.print("GPS signal not found.");
    while (true); // Halt or enter low-power mode
  }
}

void setRTCTimeFromGPS() {
  Serial.println("Waiting for GPS to get a fix...");
  while (!gps.time.isValid() || !gps.date.isValid()) {
    while (gpsSerial.available() > 0) {
      gps.encode(gpsSerial.read());
    }
    delay(100);
  }

  // Set the RTC time from the GPS time
  rtc.adjust(DateTime(gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second()));
  Serial.println("RTC time set from GPS");
}
