# Version 2


## Code
Code adapted from https://github.com/jgOhYeah/NonOfficial-StuffForCosmicWatchMuonDetectors/tree/master/Arduino%20Code/MuonDetector

What differs from Version 1:
- Accomodate a BMP 180 Altimeter 
- Write to SD card & Display on OLED
- Memory reduction to fit onto Arduino Nano.

Notable details:

Spencer Axani's code for writing to an SD card + displaying on an OLED was too large computationally to push and upload onto the Arduino Nano. A solution was found through jgOhYeah's repository, saving space on the Nano. We then implemented the BMP180 altimeter in the code, which also required us to cut down on even more libraries to accomodate the sensor's library and functionality. 

## Hardware
We found that the initial idea of trying to put a Camera, along with a window and the electronics would be unfeasible for the given weight requirements. Instead, we cut out a new box, and utilized jumper wires to route the OLED, LED, master switch, all to the wall of the box. We removed the camera.

Notably:
 
We first tried an Apeman A77 action camera, but the battery life would not last the ~ 3 hours required for launch. We then attempted to power it through the micro-usb on the side, which was wired to a battery pack. This did not seem to power either. Eventually, we gave up and tried a Raspberry Pi Zero W, with a camera module.

Our Raspberry Pi Zero W would not connect to the Wifi nor SSH, making accessability impossible with it. We also ditched this idea.