#include "defines.h"
#include "strings.h"

#ifdef USE_SCREEN
//LCD Libraries
#include <Wire.h>
#include <U8x8lib.h>
#endif
//SD card
#ifdef USE_SD_CARD
#include <SD.h>
File myFile;
#endif
#ifdef USE_SCREEN
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);
#endif
#include <EEPROM.h>
unsigned long count = 0;
float sipmVoltage = 0;

//Timing
unsigned int startTime = 0;
unsigned long totalDeadtime = 0;
#ifdef USE_SD_CARD
//SD Card stuff
bool isSDCard = false;
char separatorChar = ' ';
#endif

#ifdef USE_SERIAL_DEBUG
  #define DEBUG_RAM() debugRam(__LINE__)
#else
  #define DEBUG_RAM()
#endif

void setup() {
  //Set up ADC
  analogReference(EXTERNAL);
  ADCSRA &= ~(bit(ADPS0) | bit(ADPS1) | bit(ADPS2));  // clear prescaler bits
  ADCSRA |= bit(ADPS0) | bit(ADPS1);                  // Set prescaler to 8
  DDRD = DDRD | PIN_LED; //Set the led to be an output
  PORTD = PORTD | PIN_LED; //Turn the led on
#ifdef USE_SCREEN
  //Start the display
  u8x8.begin();
  u8x8.setFlipMode(1);
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setContrast(EEPROM.read(EEPROM_CONSTRAST));
#endif
#ifdef USE_SERIAL
  //Set up serial
  Serial.begin(9600);
  Serial.setTimeout(3000);
#endif
  DEBUG_RAM();
  char charBuffer[60] = "";
  DEBUG_RAM();
#if defined USE_SCREEN || defined USE_SERIAL
  strcpy_P(charBuffer, cosmicString);
#ifdef USE_SCREEN
  u8x8.draw2x2String(0, 0, charBuffer);
#endif
#ifdef USE_SERIAL
  Serial.print(charBuffer);
  Serial.print(' ');
  DEBUG_RAM();
#endif
  strcpy_P(charBuffer, watchString);
#ifdef USE_SERIAL
  Serial.print(charBuffer);
#endif
#ifdef USE_SCREEN
  u8x8.draw2x2String(6, 2, charBuffer);
  getEepromString(EEPROM_ID1,16,charBuffer);
  u8x8.draw1x2String(0,4,charBuffer);
  byte secondRowLength = getEepromString(EEPROM_ID2,16,charBuffer);
  u8x8.draw1x2String(17 - secondRowLength,6,charBuffer);
#endif
#ifdef USE_SERIAL
  strcpy_P(charBuffer, muonString);
  Serial.println(charBuffer);
  strcpy_P(charBuffer, deviceIdentity);
  Serial.print(charBuffer);
  getEepromString(EEPROM_ID1,EEPROM_STRING_LENGTH,charBuffer);
  Serial.print(charBuffer);
  getEepromString(EEPROM_ID2,EEPROM_STRING_LENGTH,charBuffer);
  Serial.println(charBuffer);
  strcpy_P(charBuffer, firmwareString);
  Serial.println(charBuffer);
  strcpy_P(charBuffer, compiledString);
  Serial.println(charBuffer);
  strcpy_P(charBuffer, aboutString1);
  Serial.println(charBuffer);
  strcpy_P(charBuffer, aboutString2);
  Serial.print(charBuffer);
  strncpy_P(charBuffer, aboutString3,50);
  Serial.print(charBuffer);
  strcpy_P(charBuffer, aboutString3+50);
  Serial.println(charBuffer);
#ifdef USE_SERIAL_SETTINGS
  strcpy_P(charBuffer, settingsInstructions);
  Serial.print(charBuffer);
  strcpy_P(charBuffer, settingsAndNL);
  Serial.print(charBuffer);
  strcpy_P(charBuffer, settingsInstructions2);
  Serial.println(charBuffer);
  DEBUG_RAM();
#endif
  Serial.println();
#endif
#endif
  delay(1000);
#ifdef USE_SD_CARD
  if (SD.begin(PIN_SD_CARD)) {
#if defined USE_SCREEN || defined USE_SERIAL
    strcpy_P(charBuffer, sdPresent);
#ifdef USE_SERIAL
    Serial.println(charBuffer);
#endif
    u8x8.clear();
#ifdef USE_SCREEN
    u8x8.draw1x2String(0, 0, charBuffer);
#endif
    strcpy_P(charBuffer, loggingTo);
#ifdef USE_SCREEN
    u8x8.draw1x2String(0, 2, charBuffer);
#endif
#ifdef USE_SERIAL
    Serial.print(charBuffer);
#endif
    DEBUG_RAM();
#endif
    isSDCard = true;
    openLogFile(charBuffer);
    DEBUG_RAM();
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
#ifdef USE_SD_CARD
  DEBUG_RAM();
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
    getEepromString(EEPROM_ID1,EEPROM_STRING_LENGTH,charBuffer);
    myFile.print(charBuffer);
    getEepromString(EEPROM_ID2,EEPROM_STRING_LENGTH,charBuffer);
    myFile.println(charBuffer);
    strcpy_P(charBuffer, firmwareString);
    myFile.println(charBuffer);
    strcpy_P(charBuffer, compiledString);
    myFile.println(charBuffer);
    strcpy_P(charBuffer, aboutString1);
    myFile.println(charBuffer);
    strcpy_P(charBuffer, aboutString3+50);
    myFile.println(charBuffer);
    myFile.println();
  }
#endif
#endif
  DEBUG_RAM();
  delay(1000);
#ifdef USE_SERIAL
  strncpy_P(charBuffer, txtHeader,50);
  Serial.print(charBuffer);
  strcpy_P(charBuffer, txtHeader+50);
  Serial.println(charBuffer);
#endif
#ifdef USE_SD_CARD
  if (isSDCard) {
    if(separatorChar == ',') {
      strncpy_P(charBuffer, csvHeader,50);
      myFile.print(charBuffer);
      strcpy_P(charBuffer, csvHeader+50);
      myFile.println(charBuffer);
    } else {
      strncpy_P(charBuffer, txtHeader,50);
      myFile.print(charBuffer);
      strcpy_P(charBuffer, txtHeader+50);
      myFile.println(charBuffer);
    }
    myFile.flush();
  }
  DEBUG_RAM();
#endif
  PORTD = PORTD & (~PIN_LED); //Make the led pin low
#ifdef USE_SCREEN
  u8x8.clear();
  strcpy_P(charBuffer, totalCount);
  u8x8.draw1x2String(0, 0, charBuffer);
  strcpy_P(charBuffer, upTime);
  u8x8.draw1x2String(0, 2, charBuffer);
  memcpy_P(charBuffer,offsetColon1,8);
  u8x8.drawTile(5,0,1,charBuffer);
  u8x8.drawTile(4,2,1,charBuffer);
  u8x8.drawTile(4,6,1,charBuffer);
  memcpy_P(charBuffer,offsetColon2,8);
  u8x8.drawTile(5,1,1,charBuffer);
  u8x8.drawTile(4,3,1,charBuffer);
  u8x8.drawTile(4,7,1,charBuffer);
  strcpy_P(charBuffer, rateString);
  u8x8.draw1x2String(0, 6, charBuffer);
  for(byte i = 0;i<15;i++) {
    memcpy_P(charBuffer,barScaleB+i*8,8);
    u8x8.drawTile(i+1,5,1,charBuffer);
  }
  DEBUG_RAM();
#endif
#if defined USE_SERIAL && defined USE_SERIAL_SETTINGS
  enterSettings();
#endif
  DEBUG_RAM();
  analogRead(PIN_DETECTOR);
  startTime = millis();
}

void loop() {
  if (analogRead(PIN_DETECTOR) > THRESHOLD_SIGNAL) {
    int detectionADC = analogRead(PIN_DETECTOR);
    unsigned long timeStamp = micros();
    unsigned long timeDetecting = millis() - startTime;
    PORTD = PORTD | PIN_LED; //Make the led pin high
#if defined USE_SCREEN || defined USE_SERIAL
    analogRead(PIN_TEMPERATURE);
    float temperatureC = (((analogRead(PIN_TEMPERATURE) + analogRead(PIN_TEMPERATURE)) * (1650. / 1024)) - TEMPERATURE_SENSOR_OFFSET) / 10.;
#endif
    count++;
    sipmVoltage = get_sipm_voltage(detectionADC);
    char countString[11];
    char timeString[11];
    char adcString[5];
    char sipmVString[7];
    char deadtimeString[11];
    char tempCString[7];
    ultoa(count, countString, 10);
    ultoa(timeDetecting, timeString, 10);
    itoa(detectionADC, adcString, 10);
    byte decimalPlaces = 1;
    if (sipmVoltage >= 100 || sipmVoltage < 0) {
      decimalPlaces = 0;
    }
    dtostrf(sipmVoltage, 1, decimalPlaces, sipmVString);
    ultoa(totalDeadtime, deadtimeString, 10);
    dtostrf(temperatureC, 1, 1, tempCString);
#ifdef USE_SERIAL
    Serial.print(countString);
    Serial.print(' ');
    Serial.print(timeString);
    Serial.print(' ');
    Serial.print(adcString);
    Serial.print(' ');
    Serial.print(sipmVString);
    Serial.print(' ');
    Serial.print(deadtimeString);
    Serial.print(' ');
    Serial.print(tempCString);
#ifdef USE_SERIAL_DEBUG
    Serial.print(' ');
    int ramLeft = freeRam();
    Serial.print(ramLeft);
#endif
    Serial.println();
#endif
#ifdef USE_SD_CARD
    if (isSDCard) {
      myFile.print(countString);
      myFile.print(separatorChar);
      myFile.print(timeString);
      myFile.print(separatorChar);
      myFile.print(adcString);
      myFile.print(separatorChar);
      myFile.print(sipmVString);
      myFile.print(separatorChar);
      myFile.print(deadtimeString);
      myFile.print(separatorChar);
      myFile.print(tempCString);
#ifdef USE_SERIAL_DEBUG
#ifndef USE_SERIAL
      int ramLeft = freeRam();
#endif
      myFile.print(separatorChar);
      myFile.print(ramLeft);
#endif
      myFile.println();
#ifdef SD_CARD_SKIP_WRITES
      if((count & 0x01) == 0x00) {
        myFile.flush();
      }
#else
      myFile.flush();
#endif
    }
#endif
    PORTD = PORTD & (~PIN_LED); //Make the led pin low
    while (analogRead(PIN_DETECTOR) > THRESHOLD_RESET) {
    }
    totalDeadtime += (micros() - timeStamp) / 1000;
  }
#ifdef USE_SCREEN
  updateTime();
#endif
}