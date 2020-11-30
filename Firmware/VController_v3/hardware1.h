// Please read VController_v3.ino for information about the license and authors

#ifndef HARDWARE1_H
#define HARDWARE1_H

// This file contains all the hardware settings for the VController first demo model owner by sixeight.
// Make sure you restore this file when updating to new firmware

// Choose which configuration for the switches is chosen in Config.ino
#define CONFIG_VCONTROLLER
//#define CONFIG_VCMINI
//#define CONFIG_CUSTOM
#define MENU_ON_INDIVIDUAL_DISPLAYS

// Allowable i2c rates:
// I2C_RATE_100, I2C_RATE_200, I2C_RATE_300, I2C_RATE_400, I2C_RATE_600, I2C_RATE_800, I2C_RATE_1000, I2C_RATE_1200, 
// I2C_RATE_1500, I2C_RATE_1800, I2C_RATE_2000, I2C_RATE_2400, I2C_RATE_2800, I2C_RATE_3000

#define WIRE_SPEED I2C_RATE_800 // Speed i2c port 1 (pin 17/18)
//#define WIRE1_SPEED I2C_RATE_100 // Speed i2c port 2 (pin 29/30)
#define WireLQ Wire // Sets the i2c port for the main display to port 1 (Wire)

// *** Pinout of the Teensy
//#define POWER_PIN           4
//#define POWER_SWITCH_PIN    5
#define POWER_SWITCH_NUMBER 13 // Power switch is is switch 13.
//#define INTA_PIN            2 // Digital Pin 2 of the Teensy is connected to INTA of the MCP23017 on the display boards

// *** Neopixel LEDs
#define NEOPIXELLEDPIN     17 // The NeoPixel LEDs are connected to pin 17.
#define NUMBER_OF_LEDS     16 // Number of neopixel LEDs connected
#define LED_ORDER          0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8, 11, 15, 14, 13, 12 // The order in which the LEDs are connected

//#define BACKLIGHTNEOPIXELPIN 3
//#define NUMBER_OF_BACKLIGHTS     0 // Number of neopixel LEDs connected
//#define BACKLIGHT_ORDER          0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8, 11 // The order in which the LEDs are connected

// Pin 15,16, 20, 21, 22, 23, 27 and 28 are used for switches. They are declared below
// *** Switches (internal)
#define NUMBER_OF_SWITCHES 16 // Number of internal switches
#define NUMBER_OF_ENCODERS 0
#define NUMBER_OF_EXTERNAL_SWITCHES 8
#define NUMBER_OF_MIDI_SWITCHES 0
#define TOTAL_NUMBER_OF_SWITCHES 24 // NUMBER_OF_SWITCHES + (NUMBER_OF_ENCODERS * 2) + NUMBER_OF_EXTERNAL_SWITCHES + NUMBER_OF_MIDI_SWITCHES

#define SWITCH_BOUNCE_TIME 100 // in ms

// If we have a switchpad, define it here
#define ROWS 4 //number of rows in switchpad
#define COLS 4 //number of columns in switchpad
#define ROWPINS 5, 4, 3, 2
#define COLUMNPINS 13, 12, 11, 6

// *** Switches and expression pedals (external)
#define NUMBER_OF_CTL_JACKS 4 // Number of jacks on the VController for connecting an expression pedal or two switches 

// Pin numbers in the Teensy (digital tip, digital ring, analog tip, analog ring, max_ring_for_exp_pedal) - make sure you write the analog pin numbers as A0 and not just 0
#define JACK1_PINS 28, 27, A17, A13, 800
#define JACK2_PINS 16, 15, A2, A1, 800
#define JACK3_PINS 23, 22, A9, A8, 800
#define JACK4_PINS 21, 20, A7, A6, 800

#define JACK_PORT_PULLUP true // No pull-up resistors externally, so need the internal ones

// *** Display settings of the main display - so far it is connected to a regular display board
#define PRIMARY_MAIN_DISPLAY_ADDRESS 0x27 // If we have a PCF8745AT chip in the i2c expander
#define SECONDARY_MAIN_DISPLAY_ADDRESS 0x3F // If we have a PCF8745T chip in the i2c expander
#define MAIN_LCD_DISPLAY_SIZE 16

// *** Individual displays above switches
#define NUMBER_OF_DISPLAYS 12
#define LCD_DISPLAY_SIZE 16

// Define the addresses and numbers of the displays using the VController display boards based on an MCP23017. Maximum 15 displays can be defined here. 
// Make sure the number of displays here matches the NUMBER_OF_DIPLAYS variable.
// You cannot declare displays both here and in the next section.
/*
#define DISPLAY_01_ADDRESS_AND_NUMBER 0x20, DISPLAY1
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
//#define MIDI3_BAUD_RATE 31250
#define MIDI3_ENABLED

#endif
