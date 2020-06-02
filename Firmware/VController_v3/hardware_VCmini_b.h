// Please read VController_v3.ino for information about the license and authors

#ifndef HARDWARE_VCM_B_H
#define HARDWARE_VCM_B_H

// This file contains all the hardware settings for the VController Production Model version 1.
// Make sure you include this file when updating to new firmware

#define IS_VCMINI // Will give the alternative configuration as defined in Config.ino

// Allowable i2c rates:
// I2C_RATE_100, I2C_RATE_200, I2C_RATE_300, I2C_RATE_400, I2C_RATE_600, I2C_RATE_800, I2C_RATE_1000, I2C_RATE_1200, 
// I2C_RATE_1500, I2C_RATE_1800, I2C_RATE_2000, I2C_RATE_2400, I2C_RATE_2800, I2C_RATE_3000

#define WIRE_SPEED I2C_RATE_1500 // Speed i2c port 1 (pin 17/18)
//#define WIRE1_SPEED I2C_RATE_800 // Speed i2c port 2 (pin 29/30)
//#define WireLQ Wire1 // Sets the i2c port for the main display to port 2 (Wire1)

// *** Pinout of the Teensy
// If a pin is not connected in your hardware, just add a // before it.
//#define POWER_PIN           4  // This pin that switches power on and off - enabling this pin enables the power on/off procedures in the code
//#define POWER_SWITCH_PIN    5  // The pin that goes to the power switch - can be one of the VController top switches or an external switch
//#define POWER_SWITCH_NUMBER 13 // Power switch is same as switch 13: top left switch of VController
//#define INTA_PIN            2  // Digital Pin 2 of the Teensy is connected to INTA of the MCP23017 on the display boards - defining this pin enables the display boards in the code
#define NEOPIXELLEDPIN     17  // The NeoPixel LEDs are connected to pin 17.
//#define BACKLIGHTNEOPIXELPIN 2 // The WS2811 chips for the backlights are connected to pin 2 - defining this pin enables the backlight procedures in the code
#define MAINDISPLAYBACKLIGHTPIN 2

// Pin 15,16, 20, 21, 22, 23, 27 and 28 are used for switches. They are declared below

// *** Switches (internal)
#define NUMBER_OF_SWITCHES 3
#define NUMBER_OF_ENCODERS 2
#define NUMBER_OF_EXTERNAL_SWITCHES 2 // Count 2 switches per jack
#define NUMBER_OF_MIDI_SWITCHES 15 // Switches that only can be controlled thrugh MIDI
#define TOTAL_NUMBER_OF_SWITCHES 24 // NUMBER_OF_SWITCHES + (NUMBER_OF_ENCODERS * 2) + NUMBER_OF_EXTERNAL_SWITCHES + NUMBER_OF_MIDI_SWITCHES

// If switches are directly connected to Teensy, define them here
#define NUMBER_OF_CONNECTED_SWITCHES 6
#define SWITCH1_PIN 16
#define SWITCH2_PIN 15
#define SWITCH3_PIN 14
#define SWITCH4_PIN 26
#define SWITCH5_PIN 27
#define SWITCH6_PIN 28
#define SWITCH_BOUNCE_TIME 100 // in ms

#define CUSTOM_SWITCH_NUMBERS 1, 2, 3, 10, 11, 12

// If we have a switchpad, define it here
/*#define ROWS 4 //number of rows in switchpad
#define COLS 4 //number of columns in switchpad
#define ROWPINS 5, 4, 3, 2
#define COLUMNPINS 13, 12, 11, 6*/

// Encoders
#define ENCODER1_A_PIN 11
#define ENCODER1_B_PIN 12
#define ENCODER1_SWITCH_PIN 24

#define ENCODER2_A_PIN 20
#define ENCODER2_B_PIN 21
#define ENCODER2_SWITCH_PIN 25


// *** Switches and expression pedals (external)
#define NUMBER_OF_CTL_JACKS 1 // Number of jacks on the VController for connecting an expression pedal or two switches 

// Pin numbers in the Teensy (digital tip, digital ring, analog tip, analog ring, max_ring_for_exp_pedal) - make sure you write the analog pin numbers as A0 and not just 0
#define JACK1_PINS 23, 22, A9, A8, 900
//#define JACK1_PINS 31, 32, A12, A13, 1000
//#define JACK3_PINS 20, 21, A6, A7, 1000
//#define JACK4_PINS 22, 23, A8, A9, 1000

#define JACK_PORT_PULLUP false // There are already pull-up resistors externally

// *** Neopixel LEDs
#define NUMBER_OF_LEDS     3 // Number of neopixel LEDs connected
#define LED_ORDER          0, 1, 2 // The order in which the LEDs are connected

#define NUMBER_OF_BACKLIGHTS     0 // Number of neopixel LEDs connected
#define BACKLIGHT_ORDER          0 // The order in which the LEDs are connected


// *** Display settings of the main display - so far it is connected to a regular display board
//#define PRIMARY_MAIN_DISPLAY_ADDRESS 0x3F // If we have a PCF8745AT chip in the i2c expander
//#define SECONDARY_MAIN_DISPLAY_ADDRESS 0x27 // If we have a PCF8745T chip in the i2c expander
#define MAIN_DISPLAY_TEENSY_PINS 3, 4, 5, 6, 7, 8 

// *** Individual displays above switches
//#define NUMBER_OF_DISPLAY_BOARDS 4
#define NUMBER_OF_DISPLAYS 3
#define VCMINI_DISPLAYS

// Define the addresses and numbers of the displays using the VController display boards based on an MCP23017. Maximum 15 displays can be defined here. 
// Make sure the number of displays here matches the NUMBER_OF_DIPLAYS variable.
// You cannot declare displays both here and in the next section.

//#define DISPLAY_01_ADDRESS_AND_NUMBER 0x20, DISPLAY1
//#define DISPLAY_02_ADDRESS_AND_NUMBER 0x21, DISPLAY1
//#define DISPLAY_03_ADDRESS_AND_NUMBER 0x22, DISPLAY1
//#define DISPLAY_04_ADDRESS_AND_NUMBER 0x23, DISPLAY1
//#define DISPLAY_05_ADDRESS_AND_NUMBER 0x20, DISPLAY2
//#define DISPLAY_06_ADDRESS_AND_NUMBER 0x21, DISPLAY2
//#define DISPLAY_07_ADDRESS_AND_NUMBER 0x22, DISPLAY2
//#define DISPLAY_08_ADDRESS_AND_NUMBER 0x23, DISPLAY2
//#define DISPLAY_09_ADDRESS_AND_NUMBER 0x20, DISPLAY3
//#define DISPLAY_10_ADDRESS_AND_NUMBER 0x21, DISPLAY3
//#define DISPLAY_11_ADDRESS_AND_NUMBER 0x22, DISPLAY3
//#define DISPLAY_12_ADDRESS_AND_NUMBER 0x23, DISPLAY3


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

// Pinout on PCF8745 i2c expanders
//#define BACKLIGHT_PIN     3
//#define EN_PIN  2
//#define RW_PIN  1
//#define RS_PIN  0
//#define D4_PIN  4
//#define D5_PIN  5
//#define D6_PIN  6
//#define D7_PIN  7

// Expansion board Raspberry Pi running VCbridge
#define LINE_SELECT_CC_NUMBER 119
#define VCONTROLLER_MIDI_CHANNEL 16

// Set baudrate for MIDI3 - can be changed to allow for high speed connection to raspberry pi
//#define MIDI3_BAUD_RATE 115200
//#define MIDI3_BAUD_RATE 38400
//#define MIDI3_ENABLED
#define MIDI_T36_HOST_PORT_ENABLED

#endif
