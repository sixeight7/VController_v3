#ifndef LIQUID_CRYSTAL_MCP23017_H
#define LIQUID_CRYSTAL_MCP23017_H

// Start of my MCP23017 library - used LiquidCrystal as a starting point...

// Connections to MCP23017
// GPIO B to 8 data bits of three displays
// GPIO A pin 1-4 to switch 1-4
// GPIO A pin 5 to Rs pin of three displays
// GPIO A pin 6,7 and 8 to the E pin of display 1,2 and 3

#include <Print.h>
#include <i2c_t3.h>
#include <LCD.h>
//#include <I2CIO.h>

// Define pins on MCP23017 for the displays
#define D_RS_PIN B00010000 // Pin 5 of GPIO A is the Rs pin
#define DISPLAY1 B10000000 // Pin 8 of GPIO A is the E pin for display 1
#define DISPLAY2 B01000000 // Pin 7 of GPIO A is the E pin for display 2
#define DISPLAY3 B00100000 // Pin 6 of GPIO A is the E pin for display 3
#define DISPLAY_ALL B11100000 // Pin 6-8 for all displays
#define SWITCH_PINS B00001111 // Pin 1-4 are switch pins
#define SWITCH_BOUNCE_DELAY 50 // Set bounce delay for switch in ms

// Port expander registers
#define MCP23017_IODIRA 0x00
#define MCP23017_IPOLA 0x02
#define MCP23017_GPINTENA 0x04
#define MCP23017_DEFVALA 0x06
#define MCP23017_INTCONA 0x08
#define MCP23017_IOCONA 0x0A
#define MCP23017_GPPUA 0x0C
#define MCP23017_INTFA 0x0E
#define MCP23017_INTCAPA 0x10
#define MCP23017_GPIOA 0x12
#define MCP23017_OLATA 0x14

#define MCP23017_IODIRB 0x01
#define MCP23017_IPOLB 0x03
#define MCP23017_GPINTENB 0x05
#define MCP23017_DEFVALB 0x07
#define MCP23017_INTCONB 0x09
#define MCP23017_IOCONB 0x0B
#define MCP23017_GPPUB 0x0D
#define MCP23017_INTFB 0x0F
#define MCP23017_INTCAPB 0x11
#define MCP23017_GPIOB 0x13
#define MCP23017_OLATB 0x15

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

class LiquidCrystal_MCP23017 : public LCD
{
  public:
    // Class constructor
    // lcd_Addr is the i2C address of the MCP23017 on the display board
    // lcd_Number is the number of the display (1, 2 or 3)
    LiquidCrystal_MCP23017 (uint8_t lcd_Addr, uint8_t lcd_Number);

    virtual void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);
    virtual void send(uint8_t value, uint8_t mode);
    bool updateButtonState (); // Updates state of buttons, returns true when updated
    uint8_t readButtonState (); // Returns state of buttons

  private:
    void expanderWrite (const byte reg, const byte data );
    void expanderWriteBoth (const byte reg, const byte dataA, uint8_t dataB );
    uint8_t expanderRead (const byte reg);

    int  init();             // Initialize the LCD class and the MCP23017

    uint8_t _Addr;           // I2C Address of the IO expander
    uint8_t _Number;
    uint8_t _En;             // LCD expander word for enable pin
    uint8_t _Rs;             // LCD expander word for Register Select pin
    uint8_t _Rw;             // LCD expander word for R/W pin (not implemented)
    uint8_t _ButtonState;    // State of buttons (debounced)
    uint8_t _NewButtonState; // Current state of buttons (not debounced)
    uint32_t _BounceDelay;   // Remember delay times
};

LiquidCrystal_MCP23017::LiquidCrystal_MCP23017 (uint8_t lcd_Addr, uint8_t lcd_Number) {
  _Addr = lcd_Addr;
  _Number = lcd_Number;     // LCD number is 1, 2 or 3 for display 1,2 or 3
  _Rs = D_RS_PIN;           // Pin 4 is the default Rs pin
  _En = lcd_Number; // Pin 7 for display 1, pin 6 for display 2 and pin 5 for display 3
  _Rw = 0;                  // RW pin is not implemented
}

int LiquidCrystal_MCP23017::init()
{
  int status = 0;

  // initialize the MCP23017 expander
  // and display functions.
  // ------------------------------------------------------------------------
  Wire.begin();

  if ( Wire.requestFrom ( _Addr, (uint8_t)1 ) == 1 )
  {
    // MCP23017 has four switches connected to pin 1-4 of Port A
    // Port 5-8 of Port A and all pins of Port B are connected to the displays

    // MCP PortA pin 1-4 input and pin 5-8 output
    // MCP PortB all output
    expanderWriteBoth(MCP23017_IODIRA, SWITCH_PINS, 0x00);

    // Setup for INTA port to be triggered by switch change
    expanderWriteBoth (MCP23017_IOCONA, 0b01100000, 0b01100000); // mirror interrupts, disable sequential mode
    expanderWriteBoth (MCP23017_GPPUA, SWITCH_PINS, 0x00);   // pull-up resistor for switch pins
    expanderWriteBoth (MCP23017_IPOLA, SWITCH_PINS, 0x00);  // invert polarity of signal for switch pins
    expanderWriteBoth (MCP23017_GPINTENA, SWITCH_PINS, 0x00); // Enable interrupts for switch pins

    // read from interrupt capture ports to clear them
    expanderRead (MCP23017_INTCAPA);
    //expanderRead (MCP23017_INTCAPB);

    // setup port 1 D7 = E; D6 = RS
    expanderWrite(MCP23017_GPIOA, _Rw);

    _displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
    status = 1;
  }
  return ( status );
}

void LiquidCrystal_MCP23017::begin(uint8_t cols, uint8_t lines, uint8_t dotsize)
{

  init();     // Initialise the I2C expander interface
  LCD::begin ( cols, lines, dotsize );
}

// Display low-level stuff
void LiquidCrystal_MCP23017::send(uint8_t value, uint8_t mode)
{
  // First write the value to port Bv and keep port A all 0:
  expanderWriteBoth(MCP23017_GPIOA, 0, value);

  // Is it a command or data
  // -----------------------
  if ( mode == DATA )
  {
    mode = _Rs;
  }

  // Now we toggle the enable bit(s) high
  expanderWrite(MCP23017_GPIOA, (mode | _En)); // Enable bit high
  expanderWrite(MCP23017_GPIOA, (mode));       // Enable bit low
}

// update button state
bool LiquidCrystal_MCP23017::updateButtonState () {
  bool updated = false;
  _NewButtonState = expanderRead(MCP23017_INTCAPA) & SWITCH_PINS;
  if (millis() > _BounceDelay + SWITCH_BOUNCE_DELAY) {
    _BounceDelay = millis(); // Reset timer
    if (_NewButtonState != _ButtonState) {
      if ((_NewButtonState <= 2) || (_NewButtonState == 4) || (_NewButtonState == 8)) { // Only accept 0, 1, 2, 4 or 8 - no double presses allowed
        _ButtonState = _NewButtonState;
        updated = true;
      }
    }
  }
  return updated;
}

// Read button state
// We check for the highest pressed button, in case we accidently touch the switch below
uint8_t LiquidCrystal_MCP23017::readButtonState () {
  uint8_t HighestPressedButton = _ButtonState; // State is correct for 0,1 and 2
  if (_ButtonState & 4) HighestPressedButton = 3;
  if (_ButtonState & 8) HighestPressedButton = 4;
  return HighestPressedButton;
}

// MCP23017 expander writing and reading
void LiquidCrystal_MCP23017::expanderWrite (uint8_t reg, uint8_t data ) // write data byte to both registers
{
  Wire.beginTransmission (_Addr);
  Wire.send (reg);
  Wire.send (data);
  Wire.endTransmission ();
}

void LiquidCrystal_MCP23017::expanderWriteBoth (uint8_t reg, uint8_t dataA, uint8_t dataB ) // write data byte to both registers
{
  Wire.beginTransmission (_Addr);
  Wire.send (reg);
  Wire.send (dataA);  // port A
  Wire.send (dataB);  // port B
  Wire.endTransmission ();
}

// read a byte from the expander
uint8_t LiquidCrystal_MCP23017::expanderRead (uint8_t reg)
{
  Wire.beginTransmission (_Addr);
  Wire.send (reg);
  Wire.endTransmission ();
  Wire.requestFrom (_Addr, (uint8_t) 1);
  return Wire.read();
}
// End of library


#endif
