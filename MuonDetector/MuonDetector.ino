#include "defines.h"
#include "strings.h"

#include <Adafruit_BMP085.h>
#include <Wire.h>
#include <U8x8lib.h>
#include <SD.h>

Adafruit_BMP085 bmp;
File myFile;

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

unsigned long count = 0;
float sipmVoltage = 0;

unsigned int startTime = 0;
unsigned long totalDeadtime = 0;

bool isSDCard = false;
char separatorChar = ' ';

// Declare charBuffer globally for reuse across functions
char charBuffer[60] = "";

const long double cal[] = {-9.085681659276021e-27, 4.6790804314609205e-23, -1.0317125207013292e-19,
  1.2741066484319192e-16, -9.684460759517656e-14, 4.6937937442284284e-11, -1.4553498837275352e-08,
   2.8216624998078298e-06, -0.000323032620672037, 0.019538631135788468, -0.3774384056850066, 12.324891083404246};
   
   
float get_sipm_voltage(float adc_value) {
  float v = 0;
  for (int i = 0; i < 12; i++) v += cal[i] * pow(adc_value, 11 - i);
  return v;
}

void setup() {
  analogReference(EXTERNAL);
  ADCSRA &= ~(bit(ADPS0) | bit(ADPS1) | bit(ADPS2));
  ADCSRA |= bit(ADPS0) | bit(ADPS1);
  pinMode(3, OUTPUT);

  Serial.begin(9600);

  u8x8.begin();
  u8x8.setFlipMode(1);
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  bmp.begin(); // Initialize BMP sensor

  strcpy_P(charBuffer, cosmicString);
  u8x8.draw2x2String(0, 0, charBuffer);
  strcpy_P(charBuffer, watchString);
  u8x8.draw2x2String(6, 2, charBuffer);

  if (SD.begin(PIN_SD_CARD)) {
    strcpy_P(charBuffer, sdPresent);
    u8x8.clear();
    u8x8.draw1x2String(0, 0, charBuffer);
    strcpy_P(charBuffer, loggingTo);
    u8x8.draw1x2String(0, 2, charBuffer);

    isSDCard = true;
    openLogFile(charBuffer);
    u8x8.draw1x2String(0, 4, charBuffer);
  } else {
    strcpy_P(charBuffer, noSdCard);
    u8x8.clear();
    u8x8.draw1x2String(0, 0, charBuffer);
  }

  if (isSDCard) {
    //myFile.print("细微探深");
    myFile.println();
  }


  delay(2000);

  if (isSDCard) {
    strncpy_P(charBuffer, txtHeader, 50);
    myFile.print(charBuffer);
    strcpy_P(charBuffer, txtHeader + 50);
    myFile.println(charBuffer);
    myFile.flush();
  }

  PORTD &= ~PIN_LED;

  u8x8.clear();
  strcpy_P(charBuffer, totalCount);
  u8x8.draw1x2String(0, 0, charBuffer);  // Count or other text on left

  float initialAlt = bmp.readAltitude();
  char altStr[6];
  snprintf(altStr, sizeof(altStr), "%4dm", (int)initialAlt);

  byte altLen = strlen(altStr);
  byte startCol = 16 - altLen;

  u8x8.draw1x2String(startCol, 0, altStr);

  analogRead(A0);
  startTime = millis();
}

void loop() {
  if (analogRead(A0) > THRESHOLD_SIGNAL) {
    int detectionADC = analogRead(A0);
    
    float initialAlt = bmp.readAltitude();
    

    Serial.println(detectionADC); // Print only the ADC value

    unsigned long timeStamp = micros();
    unsigned long timeDetecting = millis() - startTime;

    PORTD |= PIN_LED;

    float temperatureC = (((analogRead(PIN_TEMPERATURE) * 2) * (1650.0 / 1024)) - TEMPERATURE_SENSOR_OFFSET) / 10.0;

    count++;
    sipmVoltage = get_sipm_voltage(detectionADC);

    char countString[11], timeString[11], adcString[5], sipmVString[7], deadtimeString[11], tempCString[7];

    ultoa(count, countString, 10);
    ultoa(timeDetecting, timeString, 10);
    itoa(detectionADC, adcString, 10);

    byte decimalPlaces = (sipmVoltage >= 100 || sipmVoltage < 0) ? 0 : 1;
    dtostrf(sipmVoltage, 1, decimalPlaces, sipmVString);

    ultoa(totalDeadtime, deadtimeString, 10);
    dtostrf(temperatureC, 1, 1, tempCString);

    if (isSDCard) {
      myFile.print(countString);      myFile.print(separatorChar);
      myFile.print(timeString);       myFile.print(separatorChar);
      myFile.print(sipmVString);      myFile.print(separatorChar);
      myFile.print(tempCString);      myFile.print(separatorChar);
      myFile.print(initialAlt);
      myFile.println();
      myFile.flush();
    }

    //drawing
    u8x8.clear();
    strcpy_P(charBuffer, totalCount);
    u8x8.draw1x2String(0, 0, charBuffer);  // Count or other text on left

    char altStr[7];
    snprintf(altStr, sizeof(altStr), "%4dm", (int)initialAlt);
    byte startCol = 16 - strlen(altStr);
    u8x8.draw1x2String(startCol, 0, altStr);

    strcpy_P(charBuffer, rateString);
    u8x8.draw1x2String(0, 2, "Time:");
    u8x8.draw1x2String(0, 6, charBuffer);

    for (byte i = 0; i < 15; i++) {
      memcpy_P(charBuffer, barScaleB + i * 8, 8);
      u8x8.drawTile(i + 1, 5, 1, charBuffer);
    }    
    //end drawing

    PORTD &= ~PIN_LED;

    while (analogRead(A0) > THRESHOLD_RESET) {continue;}
    totalDeadtime += (micros() - timeStamp) / 1000.;
  }

  updateTime();
}


