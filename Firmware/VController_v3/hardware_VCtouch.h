// Please read VController_v3.ino for information about the license and authors

#ifndef HARDWARE_VCT_H
#define HARDWARE_VCT_H

// This file contains all the hardware settings for the VController Production Model version 1.
// Make sure you include this file when updating to new firmware

#define VC_NAME "VC-touch"

// Choose which configuration for the switches is chosen in Config.ino
//#define CONFIG_VCONTROLLER
//#define CONFIG_VCMINI
#define CONFIG_VCTOUCH
//#define CONFIG_CUSTOM
#define MENU_ON_INDIVIDUAL_DISPLAYS

#define IS_VCTOUCH
//#define USE_SPI_MEMORY_CHIP

// Allowable i2c rates:
// I2C_RATE_100, I2C_RATE_200, I2C_RATE_300, I2C_RATE_400, I2C_RATE_600, I2C_RATE_800, I2C_RATE_1000, I2C_RATE_1200, 
// I2C_RATE_1500, I2C_RATE_1800, I2C_RATE_2000, I2C_RATE_2400, I2C_RATE_2800, I2C_RATE_3000

//#define WIRE_SPEED 1000000 // Speed i2c port 1 (pin 17/18)
#define WIRE_SPEED 400000 // Settling at 400 kHz. 1 MHz gives too many errors.
//#define WIRE1_SPEED I2C_RATE_800 // Speed i2c port 2 (pin 29/30)
//#define WireLQ Wire1 // Sets the i2c port for the main display to port 2 (Wire1)

// *** Pinout of the Teensy
// If a pin is not connected in your hardware, just add a // before it.
#define POWER_PIN           2  // This pin that switches power on and off - enabling this pin enables the power on/off procedures in the code
#define POWER_SWITCH_PIN    3  // The pin that goes to the power switch - can be one of the VController top switches or an external switch
#define POWER_SWITCH_NUMBER 11 // Power switch is same as switch 11: top left switch of VController
//#define INTA_PIN            2  // Digital Pin 2 of the Teensy is connected to INTA of the MCP23017 on the display boards - defining this pin enables the display boards in the code
#define NEOPIXELLEDPIN     31  // The NeoPixel LEDs are connected to pin 31
//#define BACKLIGHTNEOPIXELPIN 3 // The WS2811 chips for the backlights are connected to pin 3 - defining this pin enables the backlight procedures in the code

// Pin 15,16, 20, 21, 22, 23, 27 and 28 are used for switches. They are declared below

// *** Switches (internal)
#define NUMBER_OF_SWITCHES 15 // Number of foot switches
#define NUMBER_OF_TOUCH_SWITCHES 15
#define NUMBER_OF_ENCODERS 0
#define NUMBER_OF_EXTERNAL_SWITCHES 8
#define NUMBER_OF_MIDI_SWITCHES 1
#define TOTAL_NUMBER_OF_SWITCHES 24 // NUMBER_OF_SWITCHES + (NUMBER_OF_ENCODERS * 2) + NUMBER_OF_EXTERNAL_SWITCHES + NUMBER_OF_MIDI_SWITCHES

#define SWITCH_BOUNCE_TIME 100 // in ms

// If we have a switchpad, define it here
#define ROWS 3 //number of rows in switchpad
#define COLS 5 //number of columns in switchpad
#define ROWPINS 6, 5, 4
#define COLUMNPINS 36, 35, 34, 33, 32

// *** Switches and expression pedals (external)
#define NUMBER_OF_CTL_JACKS 3 // Number of jacks on the VController for connecting an expression pedal or two switches 

// Pin numbers in the Teensy (digital tip, digital ring, analog tip, analog ring, max_ring_for_exp_pedal) - make sure you write the analog pin numbers as A0 and not just 0
#define JACK1_PINS 38, 39, A14, A15, 1000
#define JACK2_PINS 40, 41, A16, A17, 1000
#define JACK3_PINS 22, 23, A8, A9, 1000

#define JACK_PORT_PULLUP false // There are already pull-up resistors externally

#define MENU_KEY_COMBINATION 0x220 // Switches 6 and 10

// *** Neopixel LEDs
#define NUMBER_OF_LEDS     15 // Number of neopixel LEDs connected
#define LED_ORDER          0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 // The order in which the LEDs are connected

#define NUMBER_OF_BACKLIGHTS     0 // Number of WS2811 chips for backlight connected
#define BACKLIGHT_ORDER          0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8, 11 // The order in which the chips are connected


// *** Display settings of the main display - so far it is connected to a regular display board
//#define PRIMARY_MAIN_DISPLAY_ADDRESS 0x3F // If we have a PCF8745AT chip in the i2c expander
//#define SECONDARY_MAIN_DISPLAY_ADDRESS 0x27 // If we have a PCF8745T chip in the i2c expander
#define MAIN_TFT_DISPLAY
#define MAIN_TFT_BACKLIGHT_PIN 24
#define MAIN_LCD_DISPLAY_SIZE 16
#define USE_TFT_USER_FONT

// *** Individual displays above switches
//#define NUMBER_OF_DISPLAY_BOARDS 4
#define NUMBER_OF_DISPLAYS 15
#define LCD_DISPLAY_SIZE 16

// Define the addresses and numbers of the displays using the VController display boards based on an MCP23017. Maximum 15 displays can be defined here. 
// Make sure the number of displays here matches the NUMBER_OF_DIPLAYS variable.
// You cannot declare displays both here and in the next section.

/*#define DISPLAY_01_ADDRESS_AND_NUMBER 0x20, DISPLAY1
#define DISPLAY_02_ADDRESS_AND_NUMBER 0x21, DISPLAY1
#define DISPLAY_03_ADDRESS_AND_NUMBER 0x22, DISPLAY1
#define DISPLAY_04_ADDRESS_AND_NUMBER 0x23, DISPLAY1
#define DISPLAY_05_ADDRESS_AND_NUMBER 0x20, DISPLAY2
#define DISPLAY_06_ADDRESS_AND_NUMBER 0x21, DISPLAY2
#define DISPLAY_07_ADDRESS_AND_NUMBER 0x22, DISPLAY2
#define DISPLAY_08_ADDRESS_AND_NUMBER 0x23, DISPLAY2
#define DISPLAY_09_ADDRESS_AND_NUMBER 0x20, DISPLAY3
#define DISPLAY_10_ADDRESS_AND_NUMBER 0x21, DISPLAY3
#define DISPLAY_11_ADDRESS_AND_NUMBER 0x22, DISPLAY3
#define DISPLAY_12_ADDRESS_AND_NUMBER 0x23, DISPLAY3
*/

// Define the addresses of the displays using regular i2C backpacks here. Maximum 15 displays can be defined here. 
// Make sure the number of displays here matches the NUMBER_OF_DIPLAYS variable
/*
#define DISPLAY_01_ADDRESS 0x21
#define DISPLAY_02_ADDRESS 0x22
#define DISPLAY_03_ADDRESS 0x23
#define DISPLAY_04_ADDRESS 0x24
#define DISPLAY_05_ADDRESS 0x25
#define DISPLAY_06_ADDRESS 0x26
#define DISPLAY_07_ADDRESS 0x39
#define DISPLAY_08_ADDRESS 0x3A
#define DISPLAY_09_ADDRESS 0x3B
#define DISPLAY_10_ADDRESS 0x3D
#define DISPLAY_11_ADDRESS 0x20
#define DISPLAY_12_ADDRESS 0x3E
*/

// Pinout for ER_TFTM0784 display
// SPI pins for the RA8876 TFT controller
#define SPI_SPEED 42000000
#define SDI_PIN 11
#define SDO_PIN 12
#define SCLK_PIN 13
#define CS_PIN 10
#define RST_SER_PIN 9

// SPI pins for the 2828 LCD panel driver
#define CS_2828_PIN 25
#define RST_2828_PIN 28
#define SDI_2828_PIN 26
#define SCLK_2828_PIN 27

// I2C pins for GT911 touch controller
#define INT_911_PIN    30
#define RST_911_PIN    29


// Pinout on PCF8745 i2c expanders
#define BACKLIGHT_PIN     3
#define EN_PIN  2
#define RW_PIN  1
#define RS_PIN  0
#define D4_PIN  4
#define D5_PIN  5
#define D6_PIN  6
#define D7_PIN  7

// Expansion board Raspberry Pi running VCbridge
#define LINE_SELECT_CC_NUMBER 119
#define VCONTROLLER_MIDI_CHANNEL 16

// Set baudrate for MIDI3 - can be changed to allow for high speed connection to raspberry pi
#define NUMBER_OF_MIDI_PORTS 7
//#define MIDI3_BAUD_RATE 115200
//#define MIDI3_BAUD_RATE 38400
#define MIDI3_ENABLED
#define MIDI4_ENABLED
#define MIDI5_ENABLED
#define MIDI5_SPEED 500000
#define MIDI5_HANDSHAKE_PORT 37
#define MIDI_T36_HOST_PORT_ENABLED
#define RECEIVE_SERIAL_BUFFER_SIZE 512 // Stabilize serial MIDI communication with buffers
#define TRANSMIT_SERIAL_BUFFER_SIZE 256

// Names and numbers of ports
#define PORT1_NAME "USB MIDI"
#define PORT1_TYPE USBMIDI_PORT

#define PORT2_NAME "USB MIDI HOST"
#define PORT2_TYPE USBHMIDI_PORT

#define PORT3_NAME "MIDI 1"
#define PORT3_TYPE MIDI1_PORT

#define PORT4_NAME "MIDI 2"
#define PORT4_TYPE MIDI2_PORT

#define PORT5_NAME "MIDI 3"
#define PORT5_TYPE MIDI3_PORT

#define PORT6_NAME "MIDI 4"
#define PORT6_TYPE MIDI4_PORT

#define PORT7_NAME "WIRELESS MIDI"
#define PORT7_TYPE MIDI5_PORT

#define PORT8_NAME "ALL MIDI PORTS"
#define PORT8_TYPE ALL_MIDI_PORTS

#define PORT9_NAME ""
#define PORT9_TYPE USBMIDI_PORT
#endif
