#include "defines.h"
#include "strings.h"

#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;

#ifdef USE_SCREEN
#include <Wire.h>
#include <U8x8lib.h>
#endif

#ifdef USE_SD_CARD
#include <SD.h>
File myFile;
#endif

#ifdef USE_SCREEN
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);
#endif

unsigned long count = 0;
float sipmVoltage = 0;

unsigned int startTime = 0;
unsigned long totalDeadtime = 0;

#ifdef USE_SD_CARD
bool isSDCard = false;
char separatorChar = ' ';
#endif

// Declare charBuffer globally for reuse across functions
char charBuffer[60] = "";

void setup() {
  analogReference(EXTERNAL);
  ADCSRA &= ~(bit(ADPS0) | bit(ADPS1) | bit(ADPS2));
  ADCSRA |= bit(ADPS0) | bit(ADPS1);
  DDRD |= PIN_LED;
  PORTD |= PIN_LED;

#ifdef USE_SCREEN
  u8x8.begin();
  u8x8.setFlipMode(1);
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
#endif

  bmp.begin(); // Initialize BMP sensor

#if defined USE_SCREEN || defined USE_SERIAL
  strcpy_P(charBuffer, cosmicString);
#ifdef USE_SCREEN
  u8x8.draw2x2String(0, 0, charBuffer);
#endif
#ifdef USE_SERIAL
  Serial.print(charBuffer);
  Serial.print(' ');
#endif

  strcpy_P(charBuffer, watchString);
#ifdef USE_SCREEN
  u8x8.draw2x2String(6, 2, charBuffer);
#endif
#ifdef USE_SERIAL
  Serial.print(charBuffer);
#endif
#endif

  delay(1000);

#ifdef USE_SD_CARD
  if (SD.begin(PIN_SD_CARD)) {
#if defined USE_SCREEN || defined USE_SERIAL
    strcpy_P(charBuffer, sdPresent);
#ifdef USE_SCREEN
    u8x8.clear();
    u8x8.draw1x2String(0, 0, charBuffer);
#endif
    strcpy_P(charBuffer, loggingTo);
#ifdef USE_SCREEN
    u8x8.draw1x2String(0, 2, charBuffer);
#endif
#endif
    isSDCard = true;
    openLogFile(charBuffer);

#ifdef USE_SERIAL
    Serial.println(charBuffer);
#endif
#ifdef USE_SCREEN
    u8x8.draw1x2String(0, 4, charBuffer);
#endif
  } else {
    strcpy_P(charBuffer, noSdCard);
#ifdef USE_SERIAL
    Serial.println(charBuffer);
#endif
#ifdef USE_SCREEN
    u8x8.clear();
    u8x8.draw1x2String(0, 0, charBuffer);
#endif
  }

  if (isSDCard) {
    strcpy_P(charBuffer, cosmicString);
    myFile.print(charBuffer);
    myFile.print(' ');
    strcpy_P(charBuffer, watchString);
    myFile.print(charBuffer);
    strcpy_P(charBuffer, muonString);
    myFile.println(charBuffer);
    strcpy_P(charBuffer, deviceIdentity);
    myFile.print(charBuffer);
    myFile.println();
  }
#endif

  delay(1000);

#ifdef USE_SD_CARD
  if (isSDCard) {
    strncpy_P(charBuffer, txtHeader, 50);
    myFile.print(charBuffer);
    strcpy_P(charBuffer, txtHeader + 50);
    myFile.println(charBuffer);
    myFile.flush();
  }
#endif

  PORTD &= ~PIN_LED;

#ifdef USE_SCREEN
  u8x8.clear();
  strcpy_P(charBuffer, totalCount);
  u8x8.draw1x2String(0, 0, charBuffer);  // Count or other text on left

  // Altitude display, right aligned
  float initialAlt = bmp.readAltitude();
  char altStr[6];
  snprintf(altStr, sizeof(altStr), "%4dm", (int)initialAlt);  // e.g. "1234m"

  byte altLen = strlen(altStr);  // 5 for "1234m"
  byte startCol = 16 - altLen;   // Right align: place so last char at col 15

  u8x8.draw1x2String(startCol, 0, altStr);
#endif

  analogRead(PIN_DETECTOR);
  startTime = millis();
}

void loop() {
  if (analogRead(PIN_DETECTOR) > THRESHOLD_SIGNAL) {
    float initialAlt = bmp.readAltitude();
    int detectionADC = analogRead(PIN_DETECTOR);
    unsigned long timeStamp = micros();
    unsigned long timeDetecting = millis() - startTime;

    PORTD |= PIN_LED;

#if defined USE_SCREEN || defined USE_SERIAL
    float temperatureC = (((analogRead(PIN_TEMPERATURE) * 2) * (1650.0 / 1024)) - TEMPERATURE_SENSOR_OFFSET) / 10.0;
#endif

    count++;
    sipmVoltage = get_sipm_voltage(detectionADC);

    // Buffers for converting values to strings
    char countString[11], timeString[11], adcString[5], sipmVString[7], deadtimeString[11], tempCString[7];

    ultoa(count, countString, 10);
    ultoa(timeDetecting, timeString, 10);
    itoa(detectionADC, adcString, 10);

    byte decimalPlaces = (sipmVoltage >= 100 || sipmVoltage < 0) ? 0 : 1;
    dtostrf(sipmVoltage, 1, decimalPlaces, sipmVString);

    ultoa(totalDeadtime, deadtimeString, 10);
    dtostrf(temperatureC, 1, 1, tempCString);

#ifdef USE_SD_CARD
    if (isSDCard) {
      myFile.print(countString);       myFile.print(separatorChar);
      myFile.print(timeString);        myFile.print(separatorChar);
      myFile.print(adcString);         myFile.print(separatorChar);
      myFile.print(sipmVString);       myFile.print(separatorChar);
      myFile.print(deadtimeString);    myFile.print(separatorChar);
      myFile.print(tempCString);       myFile.print(separatorChar);
      myFile.print(initialAlt);

#ifdef USE_SERIAL_DEBUG
#ifndef USE_SERIAL
      int ramLeft = freeRam();
#endif
      myFile.print(separatorChar);
      myFile.print(ramLeft);
#endif
      myFile.println();

#ifdef SD_CARD_SKIP_WRITES
      if ((count & 0x01) == 0x00) {
        myFile.flush();
      }
#else
      myFile.flush();
#endif
    }
#endif

#ifdef USE_SCREEN
    u8x8.clear();
    strcpy_P(charBuffer, totalCount);
    u8x8.draw1x2String(0, 0, charBuffer);  // Count or other text on left

    char altStr[6];
    snprintf(altStr, sizeof(altStr), "%4dm", (int)initialAlt);  // e.g. "1234m"

    byte altLen = strlen(altStr);  // 5 for "1234m"
    byte startCol = 16 - altLen;   // Right align

    u8x8.draw1x2String(startCol, 0, altStr);
#endif

    PORTD &= ~PIN_LED;

    while (analogRead(PIN_DETECTOR) > THRESHOLD_RESET) {}
    totalDeadtime += (micros() - timeStamp) / 1000;
  }

#ifdef USE_SCREEN
  updateTime();
#endif
}
