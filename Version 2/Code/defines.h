/*defines.h
 * Code adapted from Spencer Axiani's original software and designs by Jotham Gates
 * This file allows the software to be customised without having to scroll through many lines of code.
 * See MuonDetector.ino for more information
 */
#define FIRMWARE_VERSION "2.3.2"
//Pins
#define PIN_TEMPERATURE A3
#define PIN_LED B00001000 //Bitmap of pin D3 - Does not use digitalWrite to save time and memory
#define PIN_SD_CARD 10
#define PIN_COINCIDENCE B01000000 //Bitmap of pin D6
#define OLED_RESET 10

//Interfaces to use. Comment out to disable non required ones. This will speed the detector up a little bit.
//If both serial and the sd card are not required, the arduino will not bother to take readings from the temperature sensor or format the values as a string.
//If the screen is used, it will be updated once per second.
#define USE_SCREEN //Whether to include the library and code for driving the oled display or not
#define USE_SD_CARD //Whether to include the library and code for writing to the sd card
#define THRESHOLD_SIGNAL 50
#define THRESHOLD_RESET 25
#define THRESHOLD_LARGE 180


//General
#define NUMBER_FILES_TO_DELETE 5 //If the sd card if full, start at the beginning and delete 5 more files ahead
#define FILENAME_NUMBER_START_POS 5 //Position of the hundreds char in the filename
#define MAX_FILES 200 //200 temp, 10 for testingMaximum file number before rolls back to 0 Needs to be less then 255 - NUMBER_FILES_TO_DELETE (in this case 250)
#define SCREEN_UPDATE_INTERVAL 1000 //ms between updating the screen
#define TEMPERATURE_SENSOR_OFFSET 500 //The voltage (in mV) produced by the temperature sensor when the temperature is 0.

