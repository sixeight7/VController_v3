// Please read VController_v2.ino for information about the license and authors

#ifndef LCDLIB1_H
#define LCDLIB1_H

// Start of my PCF8745 library - an adapted copy from FMathilda's LiquidCrystal_i2c library.
// Wrote this to avoid having to edit a library. It makes the sketch easier to transfer.

#include <Print.h>
#include <i2c_t3.h>
#include <LCD.h>
//#include "debug.h"

#ifndef WireLQ
#define WireLQ Wire // Default i2c port is port 1
#endif

class LiquidCrystal_PCF8745 : public LCD
{
  public:
    // Class constructor
    // lcd_Addr is the i2C address of the MCP23017 on the display board
    // lcd_Number is the number of the display (1, 2 or 3)
    LiquidCrystal_PCF8745 (uint8_t lcd_Addr, uint8_t En, uint8_t Rw,
                           uint8_t Rs, uint8_t d4, uint8_t d5,
                           uint8_t d6, uint8_t d7 );
    virtual void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);
    virtual void send(uint8_t value, uint8_t mode);
    virtual void setBacklightPin ( uint8_t value, t_backlighPol pol );
    virtual void setBacklight( uint8_t value );

  private:
    int digitalWrite (byte data );
    int  init();             // Initialize the LCD class and the MCP23017
    void write4bits ( uint8_t value, uint8_t mode ) ;

    uint8_t _Addr;           // I2C Address of the IO expander
    bool   _initialised;     // Initialised PCF8745 chip
    uint8_t _Number;
    uint8_t _En;             // LCD expander word for enable pin
    uint8_t _Rs;             // LCD expander word for Register Select pin
    uint8_t _Rw;             // LCD expander word for R/W pin (not implemented)
    uint8_t _backlightPinMask; // Backlight IO pin mask
    uint8_t _backlightStsMask; // Backlight status mask
    uint8_t _data_pins[4];     // LCD data lines
};

#define LCD_NOBACKLIGHT 0x00
#define LCD_BACKLIGHT   0xFF

LiquidCrystal_PCF8745::LiquidCrystal_PCF8745 (uint8_t lcd_Addr, uint8_t En, uint8_t Rw,
    uint8_t Rs, uint8_t d4, uint8_t d5,
    uint8_t d6, uint8_t d7 )
{
  _Addr = lcd_Addr;

  _backlightPinMask = 0;
  _backlightStsMask = LCD_NOBACKLIGHT;
  _polarity = POSITIVE;

  _En = ( 1 << En );
  _Rw = ( 1 << Rw );
  _Rs = ( 1 << Rs );

  // Initialise pin mapping
  _data_pins[0] = ( 1 << d4 );
  _data_pins[1] = ( 1 << d5 );
  _data_pins[2] = ( 1 << d6 );
  _data_pins[3] = ( 1 << d7 );
}

// Initialization of i2c, MCP23017 ports and display
int LiquidCrystal_PCF8745::init()
{

  // initialize the backpack IO expander
  // and display functions.
  // ------------------------------------------------------------------------

  _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

  _initialised = WireLQ.requestFrom ( _Addr, (uint8_t)1 );
  //_shadow = WireLQ.read ();
  if (_initialised) digitalWrite(0);  // Set the entire port to LOW

  return _initialised;
}

void LiquidCrystal_PCF8745::begin(uint8_t cols, uint8_t lines, uint8_t dotsize)
{

  init();     // Initialise the I2C expander interface
  LCD::begin ( cols, lines, dotsize );
}

// Display low-level stuff
// send is called from the base LCD library
void LiquidCrystal_PCF8745::send(uint8_t value, uint8_t mode)
{
  if ( mode == FOUR_BITS )
  {
    write4bits( (value & 0x0F), COMMAND );
  }
  else
  {
    write4bits( (value >> 4), mode );
    write4bits( (value & 0x0F), mode);
  }
}

void LiquidCrystal_PCF8745::setBacklightPin ( uint8_t value, t_backlighPol pol = POSITIVE )
{
  _backlightPinMask = ( 1 << value );
  _polarity = pol;
  setBacklight(BACKLIGHT_OFF);
}

void LiquidCrystal_PCF8745::setBacklight( uint8_t value )
{
  if ( _backlightPinMask != 0x0 )
  {
    // Check for polarity to configure mask accordingly
    // ----------------------------------------------------------
    if  (((_polarity == POSITIVE) && (value > 0)) ||
         ((_polarity == NEGATIVE ) && ( value == 0 )))
    {
      _backlightStsMask = _backlightPinMask & LCD_BACKLIGHT;
    }
    else
    {
      _backlightStsMask = _backlightPinMask & LCD_NOBACKLIGHT;
    }
    digitalWrite( _backlightStsMask );
  }
}

void LiquidCrystal_PCF8745::write4bits ( uint8_t value, uint8_t mode )
{
  uint8_t pinMapValue = 0;

  // Map the value to LCD pin mapping
  // --------------------------------
  for ( uint8_t i = 0; i < 4; i++ )
  {
    if ( ( value & 0x1 ) == 1 )
    {
      pinMapValue |= _data_pins[i];
    }
    value = ( value >> 1 );
  }

  // Is it a command or data
  // -----------------------
  if ( mode == DATA )
  {
    mode = _Rs;
  }

  pinMapValue |= mode | _backlightStsMask;

  // Now we toggle the enable bit(s) high and vrite the data in the first write cycle
  digitalWrite (pinMapValue | _En);   // En HIGH
  digitalWrite (pinMapValue & ~_En);  // En LOW
}

// **** PCF8745 expander writing and reading
// write a byte to a ports of the expander
int LiquidCrystal_PCF8745::digitalWrite (uint8_t value )
{
  int status = 0;
  if (_initialised) {
    WireLQ.beginTransmission ( _Addr );
#if (ARDUINO <  100)
    WireLQ.send ( value );
#else
    WireLQ.write ( value );
#endif
    status = WireLQ.endTransmission ();
  }
  return ( (status == 0) );
}

#endif
