The purpose of this project is to test the limits of embedded systems computation by attempting to implement a custom, functional operating system simulator on an ESP32 dev board module with custom applications, peripherals, and user interface. In addition to the technical challenge, this project is motivated by a desire to actualize an acute appreciation for retro-futurism, espionage-spyware and hacker aesthetics.
# System Architecture
<img width="741" height="791" alt="Prototype&#39;85_System_Architecture drawio" src="https://github.com/user-attachments/assets/62e7c06a-1b26-4efe-88b3-74e160f81c39" />
# Pinout
## Main
| ESP32-WROOM-32 | Modules                                                                         |
| -------------- | ------------------------------------------------------------------------------- |
|                | **OLED-1** / **OLED-2** / **1602 LCD (I$^2$C Adapter)**/**MCP23017 (Keyboard)** |
| GPIO22         | SDA                                                                             |
| GPIO21         | SCL                                                                             |
| GND            | GND                                                                             |
| 5V             | VCC                                                                             |
|                | **TFT LCD**                                                                     |
| GPIO19         | MISO                                                                            |
| GPIO23         | MOSI                                                                            |
| GPIO18         | SCLK                                                                            |
| GPIO15         | TFT_CS                                                                          |
| GPIO02         | DC                                                                              |
| GPIO04         | RST                                                                             |
| GND            | GND                                                                             |
| 5V             | VCC                                                                             |
|                | **Joystick**                                                                    |
| GPIO32         | X                                                                               |
| GPIO33         | Y                                                                               |
| 5V             | VCC                                                                             |
| GND            | GND                                                                             |
|                | **Linear Potentiometer**                                                        |
| GPIO14         | Pin                                                                             |
| GND            | GND                                                                             |
| VCC            | VCC                                                                             |
|                | **Flip Switches**                                                               |
| GPIO34         | SW1                                                                             |
| GPIO35         | SW2                                                                             |
| GND            | GND                                                                             |
|                | **MAX98357A**                                                                   |
| GPIO27         | DIN                                                                             |
| GPIO26         | BCLK                                                                            |
| GPIO25         | LRC                                                                             |
| GND            | GAIN                                                                            |
| GND            | GND                                                                             |
| 3V3            | VIN                                                                             |
|                | **R307 Fingerprint Sensor**                                                     |
| GPIO01 (TX0)   | TX                                                                              |

## Keyboard
const byte ROWS = 4;   // Number of rows
  const byte COLS = 10;  // Number of columns
  char keysUpper[ROWS][COLS] = {
    { 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'O', 'P', '<' },
    { 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '\n' },
    { 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '.' },
    { '1', '2', '3', ' ', '5', '6', 'I', '8', ' ', '0' }
  };
  
  byte rowPins[ROWS] = { 12, 15, 14, 13 };
  byte colPins[COLS] = { 0, 1, 2, 3, 4, 5, 7, 6, 9, 8 }; 
| GPIO03 (RX0)   | RX                                                                              |
| GND            | GND                                                                             |
| 5V             | VCC                                                                             |
