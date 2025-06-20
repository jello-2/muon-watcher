/*oled.ino
 * Code adapted from Spencer Axiani's original software and designs by Jotham Gates
 * This file contains functions relating to the display
 * See MuonDetector.ino for more information
 */
#ifdef USE_SCREEN
void runningTime(char * timeSinceStart) {
  unsigned long milliSeconds = millis() - startTime;
  byte seconds = milliSeconds / 1000 % 60;
  byte minutes = milliSeconds / 60000 % 60;
  byte hours = milliSeconds / 3600000 % 24;
  // byte days = milliSeconds / (removed calculation for days); // Removed this line

  char number[7]; // This buffer size is still sufficient for HH:MM:SS, but could be smaller if only H:M:S is needed
  
  // Removed conversion and concatenation for 'days'
  
  strcpy(timeSinceStart, ""); // Initialize timeSinceStart as empty

  //Add a 0 to keep places
  if (hours < 10) {
    strcat_P(timeSinceStart, zero);
  }
  itoa(hours, number, 10);
  strcat(timeSinceStart, number);
  strcat_P(timeSinceStart, colon);
  //Add a 0 to keep places
  if (minutes < 10) {
    strcat_P(timeSinceStart, zero);
  }
  itoa(minutes, number, 10);
  strcat(timeSinceStart, number);
  strcat_P(timeSinceStart, colon);
  if (seconds < 10) {
    strcat_P(timeSinceStart, zero);
  }
  itoa(seconds, number, 10);
  strcat(timeSinceStart, number);
}
#endif
#ifdef USE_SCREEN
void updateScreen() {
  //Only update the count and pulse size part of the display when required
  static unsigned long lastCount = 90; //Some value other than 0
  if (count != lastCount) {
    //unsigned long writeTime = micros(); //Check the screen update time. Around 10.5ms for this version, 32.0ms for the old hyphen graph
    lastCount = count;
    unsigned int sipmIntV = (unsigned int)(sipmVoltage); //Hopefully make processing faster.
    char charBuffer[16];
    ultoa(count, charBuffer, 10);
    u8x8.draw1x2String(6, 0, charBuffer);
    //Draw a line indicating the size of the pulse on the u8x8 display
    if (count == 0) {
      strcpy_P(charBuffer, detectorReady);
      u8x8.setInverseFont(1);
      u8x8.drawString(1, 4, charBuffer); //These overwrite the entire top line of the graph, so don't need to clear it beforehand.
      u8x8.setInverseFont(0);
    } else if (sipmIntV > THRESHOLD_LARGE) {
      strcpy_P(charBuffer, sipmWow);
      u8x8.setInverseFont(1);
      u8x8.drawString(1, 4, charBuffer);
      u8x8.setInverseFont(0);
    } else { //Bar graph part
      byte tempTile[120];
      memcpy_P(tempTile,barScaleT,120);
      //Serial.print(F("\tBAR TEST: Voltage F: ")); //Runs out of ram about here with this left in
      //Serial.print(sipmVoltage);
      //Serial.print(F(", Voltage I: "));
      //Serial.print(sipmIntV);
      //Number of pixels = V/180*119 sort of 120/180*V = V*6/9 = V*2/3
      unsigned int barLength = (sipmIntV * 2) / 3; //<<1 should be a fast multiply by 2 on an unsigned int
      //Serial.print(F(", Bar Length Px: "));
      //Serial.println(barLength);

      for(byte i = 2; i < barLength && i < 118; i++) { //Invert the bar graph to the rught length. LEaving the ends as they are white.
        tempTile[i] ^= CHAR_BAR_GRAPH; //Make the columns with pixels displaying the bar graph white.
      }
      u8x8.drawTile(1,4,15,tempTile);
      
      /*byte i = 0;
      for (; (i < (sipmVoltage + 10) / 10) && (i < 15); i++) {
        charBuffer[i] = CHAR_BAR_CHART;
      }
      //Fill the rest up with spaces to cover the previous graph
      for (; i < 15; i++) {
        charBuffer[i] = ' ';
      }
      //Null terminate the array
      charBuffer[i] = 0;*/
    }
    /*writeTime = micros() - writeTime;
    Serial.print(F("\tLCD Update time: "));
    Serial.println(writeTime);*/
  }
}
#endif

#ifdef USE_SCREEN
void updateTime() {
  // Refresh display every SCREEN_UPDATE_INTERVAL ms (typically 1000 ms)
  static unsigned long nextTime = 0;
  if (millis() >= nextTime) {
    unsigned long now = millis();
    unsigned long elapsed = now - startTime - totalDeadtime;

    // Format time as hh:mm:ss
    char timeBuffer[10];
    unsigned long totalSeconds = elapsed / 1000;
    unsigned int h = totalSeconds / 3600;
    unsigned int m = (totalSeconds % 3600) / 60;
    unsigned int s = totalSeconds % 60;
    snprintf(timeBuffer, sizeof(timeBuffer), "%02u:%02u:%02u", h, m, s);
    u8x8.draw1x2String(5, 2, timeBuffer); // Display right of "Time:"

    // Compute count rate and standard deviation
    float countRate = 0.0, countStd = 0.0;
    if (count > 0 && elapsed > 0) {
      float t = elapsed / 1000.0;
      countRate = count / t;
      countStd = sqrt(count) / t;
    }

    char charBuffer[12];
    byte decimalPlaces = (countRate < 10) ? 3 : 2;
    dtostrf(countRate, 5, decimalPlaces, charBuffer);
    u8x8.draw1x2String(5, 6, charBuffer);

    u8x8.drawGlyph(10, 6, '+');
    u8x8.drawGlyph(10, 7, '-');

    decimalPlaces = (countStd < 10) ? 3 : 2;
    dtostrf(countStd, 5, decimalPlaces, charBuffer);
    u8x8.draw1x2String(11, 6, charBuffer);

    updateScreen(); // May be a placeholder for additional rendering logic

    // Track time lost to updating screen
    unsigned long updateEnd = millis();
    totalDeadtime += updateEnd - now;

    nextTime = now + SCREEN_UPDATE_INTERVAL;
  }
}

#endif