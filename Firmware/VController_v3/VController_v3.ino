/****************************************************************************
**
** Copyright (C) 2015 Catrinus Feddema
** All rights reserved.
** This file is part of "VController v3" teensy software.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**
** If you use this code in a homebuild project, please support me by sending
** a donation via https://www.paypal.me/sixeight
** If you want to build VControllers, VC-mini's or VC-touches commercially, 
** please contact me on SixEightSoundControl|@gmail.com.
****************************************************************************/

// This page has the following parts:
// Section 1: Hardware selection
// Section 2: Library declaration and main setup() and loop()
// Section 3: VController documentation overview
// Section 4: VController project log

// ********************************* Section 1: Hardware selection ********************************************

// **** Choose the correct hardware below and update the Arduino compiler settings ****
// Current version of TeensyDuino: 1.8.19

// Hardware of VController production model modified for Teensy 4.1
//#include "hardware.h"
// Arduino IDE settings: Board: Teensy 4.1, USB Type: MIDI, CPU speed: 600 MHz, Optimize: Faster, Programmer: -

// Hardware of VC-touch
//#include "hardware_VCtouch.h"
// Arduino IDE settings: Board: Teensy 4.1, USB Type: MIDI, CPU speed: 600 MHz, Optimize: Faster, Programmer: -

// Hardware of VC-mini rev. B (Teensy 4.1)
#include "hardware_VCmini_41.h"
// Arduino IDE settings: Board: Teensy 4.1, USB Type: MIDI, CPU speed: 600 MHz, Optimize: Faster, Programmer: -

// Hardware of VC-mini rev. B (Teensy 3.6)
//#include "hardware_VCmini_b.h"
// Arduino IDE settings: Board: Teensy 3.6, USB Type: MIDI, CPU speed: 180 MHz, Optimize: Fast(!), Programmer: AVRISP mkII

// Hardware of VController V1 model of sixeight
//#include "hardware1.h"
// Arduino IDE settings: Board: Teensy 3.1/3.2, USB Type: MIDI, CPU speed: 96 MHz, Optimize: Smallest code with LTO, Programmer: AVRISP mkII

// Hardware of VController model of Willem Smith
//#include "hardware_WS.h"
//#include "hardware_VCmini_1.h"
// Arduino IDE settings: Board: Teensy 3.6, USB Type: MIDI, CPU speed: 180 MHz, Optimize: Fast, Programmer: AVRISP mkII

// In order for the Serial MIDI ports to receive larger messages the following files have to be edited:
// Win: C:\Program Files (x86)\Arduino\hardware\teensy\avr\cores\teensy3\serial1.c
// Mac: /Applications/Arduino.app/Contents/Java/hardware/teensy/avr/cores/teensy3/serial1.c
// Change the number 64 to 650 on the following line:
// #define SERIAL1_RX_BUFFER_SIZE 650
// Do the same thing for serial2.c and serial3.c

// ********************************* Section 2: Library declaration and main setup() and loop() ********************************************

// Used libraries:
// * https://github.com/sixeight7/ER-TFTM0784-1
// * https://github.com/sixeight7/arduino-goodix
// * https://github.com/sixeight7/extEEPROM
// * https://github.com/sixeight7/MIDI4.3 
// * https://github.com/sixeight7/encoder
// * https://github.com/sixeight7/LiquidCrystal

#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)
#include <i2c_t3.h>
#else
#include <Wire.h>
//#include <i2c_driver.h>
//#include <i2c_driver_wire.h>
#endif

#ifdef IS_VCTOUCH
#include <Goodix.h> // Must be declared on first page, otherwise sketch will not compile
#endif

#define VCONTROLLER_FIRMWARE_VERSION_MAJOR 3
#define VCONTROLLER_FIRMWARE_VERSION_MINOR 12
#define VCONTROLLER_FIRMWARE_VERSION_BUILD 0

#include "debug.h"
#include "globals.h"

void setup() {
  SCO_switch_power_on();
  setup_LED_control(); //Should be first, to reduce startup flash of Neopixel LEDs
  setup_debug();

  // Wire speeds are set in hardware.h
#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, WIRE_SPEED);
#else
  pinMode(18, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);
  Wire.setClock(WIRE_SPEED);
  Wire.begin();
#endif
#ifdef WIRE2_SPEED
  pinMode(24, INPUT_PULLUP);
  pinMode(25, INPUT_PULLUP);
  Wire2.setClock(WIRE2_SPEED);
  Wire2.begin();
#endif

  setup_LCD_control();
  setup_devices();
  setup_eeprom();
  setup_switch_check();
  setup_switch_control();
  setup_page();
  setup_MIDI_common();
  check_all_devices_for_manual_connection();
  DEBUGMAIN("Setup finished");
}

void loop() {
  main_switch_check(); // Check for switches pressed
  main_switch_control(); //If switch is pressed, take the configured action
  main_LED_control(); //Check update of LEDs
  main_LCD_control(); //Check update of displays
  main_MIDI_common(); //Read MIDI ports
  main_page(); // Check update of current page
  main_devices();
  main_eeprom();
  //MIDI_check_USBHMIDI();
}

// ********************************* Section 3: VController documentation overview  ********************************************

/* VController version 3.

  Hardware VController;
  - 16 momentary switches connected to display boards or in keypad matrix
  - 12  neopixel LEDs
  - 12 LCD displays for first 12 switches + main LCD display - all 16x2 character displays
  - Teensy 3.2
  - 3 MIDI input/outputs and MIDI over USB. Support for external Raspberry Pi running VCbridge to add extra ports.
  - Extra EEPROM Flash memory: 24LC512

  Hardware VC-mini
  - 3 - 6 momentary switches connected to Teensy directly
  - 2 rotary encoders with push button
  - 3 or more  neopixel LEDs
  - 1 main LCD display - a 16x2 character displays
  - Teensy 3.6
  - 2 MIDI input/outputs, a USB MIDI host port and MIDI over USB. Support for external Raspberry Pi running VCbridge to add extra ports.
  - Extra EEPROM Flash memory: 24LC512

  Software features:
  - Patch and parameter control for the following devices:
      Boss GP-10, Roland GR-55, Roland VG-99, Boss Katana
      Zoom G3, Zoom MS SERIES
      Line6 M13, Line6 Helix
      AxeFX II (other types implemented but not tested)
      Kemper Profiling Amp
  - Patchnames are read from devices (apart from the Line6 Helix)
  - Parameter states and assigns are read from devices (apart from Line6 Helix)
  - Programmable pages of switches. These can be programmed on the VController of VC-mini or via the external editor

  Software structure:
  - Initialization of LCDs, LEDs and MIDI ports
  - Configuration of the pages and switches is done in the page[] array on the Config page.
  - On startup, detection of devices, page change or patch change the current page is loaded in the SP[] array (void load_page)
  - After the page is loaded, the patchnames and parameter states are read (void Request_first_switch, Request_next_switch and PAGE_request_current_switch)
  - Main loop checks for switches being pressed (Switch_check/Switch Control), midi received and update of LCD and LED states.

*/

// ********************************* Section 4: VController project log  ********************************************

/*
  23-06-2015 First VController design ready
  14-07-2015 v.1.0.0 First basic version of VController ready.
  17-07-2015 Added global tuner
  04-08-2015 Added assigns for VG99
  11-08-2015 Added US20 emulation
  16-08-2015 Added global tempo
  10-11-2015 v.2.0.0 Start programming v2. Developed structure for VController
  19-11-2015 Finished GP10 patch selection mode
  20-12-2015 Patch change mode for GR55 and VG99 as well.
  22-12-2015 US20 emulation done
  24-12-2015 Wrote PATCH modes and fixed Bank up/down problems
  24-12-2015 Problem with occasional reboot in VG99 patch change. Previous version did not seem to have this problem...
  26-12-2015 Started writing assign mode for GP-10. Does contains bugs.
  27-12-2015 Assign mode finished. Bug was a string written out of bounds..
  29-12-2015 Programmed TRI/FOUR/(FIVE)STATE options in GP10
  30-12-2015 Parameter and assign_mode for GR55 written
  31-12-2015 Allow for multiple commands / added tap tempo and bass mode from version 1
  01-01-2016 Parameter and assign mode for VG-99
  02-01-2016 Added FC300 CTL1-8 for VG-99
  17-01-2016 Worked on individual display messages
  2?-03-2016 Updated firmware to work with the new hardware
  30-03-2016 Simplified updating individual displays
  02-04-2016 Added EEPROM.ino with global read/store and on/standbye mode for the VController
  13-06-2016 Started adding page for Zoom G3
  11-07-2016 Changed LCD_set_patch_number_and_name() so the main display can deal with the Zoom G3 and more devices as well
  12-07-2016 Added virtual LEDs (LED state in display)
  16-07-2016 Changed GP10/GR55/VG99/ZG3_detected into connect/offline system.
  26-07-2016 Updated inconsistent display update system
  27-07-2016 Consistent naming scheme for all functions and voids
  28-07-2016 Added support for expression pedals - they send fixed cc messages now
  30-12-2016 v.3.0.0 Started rewriting code into class structure
  11-02-2017 Changed structure of pages to new style with unlimited commands
  13-02-2017 Implemented default page for commands - this makes programming much more flexible...
  14-02-2017 Added bass mode for GR-55
  23-02-2017 Added support for multiple hardware - moving all hardware settings to hardware.h. Added compatibility for the new production model
  20-03-2017 Added support for VCbridge - virtual ports on s Raspberry pi
  27-03-2017 The commands array is now written to the extermal chip and read back into the memory
  17-04-2017 Added STEP type parameter
  27-05-2017 New parameter view for GP10/GR55 and VG99 - added PAR_BANK
  29-05-2017 Updated VG99 parameter list. Added EXP1,2 + EP SW1,2 assigns
  30-05-2017 Added switch hold
  01-06-2017 Added UPDOWN latch type.
  07-06-2017 Created global menu amd the menu structure.
  14-06-2017 Added support for expression pedals for VG990 parameters and assigns
  22-06-2017 Added menu command structure. Updated button respons as responded poorly on menus
  28-06-2017 Added new indexes for commands on EEPROM. Improved reading keypresses over i2c
  31-06-2017 Faster reading of new indexes. Tested and fixed executing multiple commands under one button
  08-06-2017 Increased the speed of the i2c connections. Use 1500 kHz for MCP23017 and memory and 800 kHz for PCF8745 expander chips. The production model uses both i2c ports now.
  11-06-2017 First version of command edit implemented - still kind of buggy...
  24-06-2017 Added EEPROM checks for changed data before writing to preserve EEPROM life.
  24-06-2017 Added new FX types for ZMS-cdr version 2.0.0
  27-06-2017 Adding new commands in edit mode finished.
  29-06-2017 Adding new page in edit mode finished
  30-06-2017 Added virtual keyboard for changing page names and labels
  27-07-2017 Added deleting of commands and names. Included purging of commands.
  28-07-2017 Added MIDI_CC messages including the CC ledger (memory for 25 CC values)
  29-07-2017 Added support for VCbridge on every port.
  29-08-2017 Made FX colours programmable from the menu
  30-08-2017 Split fixed and programmable commands
  01-09-2017 Re-arranged menu structure
  08-09-2017 Added PC ledger to have LEDs showing the current "patch" for GM PC. As long as channel and port match the selected PC will light up
  25-09-2017 v3.0.1 Fixed PAGE_UP and PAGE_DOWN not working properly
  04-10-2017 v3.0.2 Updown could not be selected when programming menus
  29-10-2017 New algorithm for bass mode
  04-11-2017 ZMScdr bug not showing patch name fixed. VController now diplaying full version number on powerup. Main display top line behaviour updated.
  05-11-2017 Connect of first new device automatically selects its device page. v3.0.2 published
  11-11-2017 v3.0.3 Support Line6 M13 Patch change. Reading the patch names works, but is too slow for practical use. Tap tempo and global tuner work as well
  13-11-2017 Added basic support for effects and looper control
  19-11-2017 Added full support of Line6 M13 - scenes are read after detection and stored in memory.
  22-11-2017 Added support for control of M13 expression pedals. Also disabled expression pedal triggering other functions.
  01-03-2018 Added support for Line6 Helix - basic control of patch/parameters/snapshots and looper. Also changed the connection system to allow the Helix to connect while it is not detectable.
  17-04-2018 Started with adding some support for the AxeFX2 for Ryan Muar.
  23-04-2018 Started with adding support for Boss Katana
  28-04-2018 Changed LCDs.ino, with better centring of titles and labels for the individual displays. Removed all printf statements
  29-04-2018 Improved direct select - it now shows patchnames and allows bank up/down during selection.
  15-05-2018 Patch selection, IA and XY switches, tap tempo and tuner support for the AxeFX working and tested.
  19-05-2018 Devices can now have up to four pages associated to the device. Selecting the next device will move to the page that was last selected for a specific device.
  20-05-2018 Added master expression pedal. You can edit the menu and command settings with an expression pedal!!!
  23-05-2018 Added calibration menu and implemented the calibration in the code.
  03-06-2018 Added master expression pedal support for GR55 (limited), VG99 and Katana
  04-06-2018 Even faster LCD support. LCD contents are copied in memory and only the changes are written to the displays
  06-06-2018 Added faster LCD support for main display as well.
  13-06-2018 Added looper support with moving ledbar for M13 and Helix
  14-06-2018 Added looper support for the AxeFX
  16-06-2018 AxeFX parameter bank now only shows active effects (bypass and XY states)
  17-06-2018 VG99 parameters are now split in categories
  july 2018  Added editor support for the VController
  29-08-2018 Added port forwarding for Katana editor - works partially - editor gives error on patch sync, but parameters can be edited!
  30-08-2018 Added Katana edit mode, added large part of the parameters, but all the MOD and FX parameters will be too much data. Allowed UPDOWN and RANGE to support large numbers for delay times.
  10-09-2018 Katana can now store 80 extra patches.
  14-09-2018 Made the code for the Katana patch storage more readable. Also the extra parameters of the SDE-3000 are stored on the Katana now.
  15-09-2018 Alpha testing release 3.1. Fixed minor bugs, improved patch reading for G3/MS SERIES, GR55 direct select improved, M13 properly switched between EXP1 and 2. Updated fixed configuration for GR55, Katana and direct select
  22-09-2018 When switching between pages with different bank sizes, the bank with the current patch is now always shown.
  22-09-2018 Release of firmware 3.1
  27-09-2018 Started with KPA support
  20-10-2018 Changed the layout of the PARAMETER and PAR_BANK displays. Took out the device name. Updated this for all devices
  30-10-2018 Fixed bug in UPDOWN behaviour in PAR_BANK. Increaded speed of switch holding
  01-04-2019 Added support for Teensy 3.6 and the USB host port - make sure you use Arduino 1.8.9 and Teensyduino 1.46 to compile for proper Roland device support.GP-10 does not work yet.
  20-04-2019 VController looper now responds to CC looper messages from Helix. Also added the Helix data forwarding mode (from VC-edit rev. A code)
  21-04-2019 Added CC control of all switches and expression pedals.
  22-04-2019 Updated internal editor for MIDI switch support
  28-04-2019 Added VC-edit support for MIDI switches
  30-04-2019 Started VCmini support from main code. Added VCmini mode for main display.
  01-05-2019 Added ON_RELEASE and ON_LONG_PRESS options to make full use of only three switches. Also added the option to press switch 1+2, 2+3 and 1+3 for the VC-mini. VC-edit still must be updated with these changes.
  31-05-2019 Finished updating menu structure for VC-mini.
  03-06-2019 Added Katana v3.0 FX and new KPA reverbs. Fixed several bugs (repressing switch on different page error and MEP parameter control)
  15-06-2019 Added USB hub support for max eight connections and fixed GP-10 sysex communication (needs cable 1)
  17-06-2019 Added Katana editing via encoders for the VC-mini
  24-06-2019 Added double and triple snapshot selection options for snapshots/scenes
  13-07-2019 Fixed looper bar error for longer loops
  16-07-2019 Helix PC/CC forwarding now stores and checks the current setlist. This ensures no faulty messages are sent when changing setlist on the Helix. Fixed wrong label shown on large VController for looper bar
  23-07-2019 Changed format of PAGE, PATCH and ASSIGN commands - also updated the onboard editing and fixed a bug in making new commands.
  09-09-2019 Uploaded V3.3.0 firmware to Github.
  29-09-2019 Implemented new library for encoders that is more stable and supports encoder acceleration - when the encoder is rotated faster, the encoders value will increment faster.
  05-10-2019 Memory versions of external memory are stored on the external EEPROM instead of internal. This is easier when swapping memory chips.
  30-10-2019 v.3.3.3 Solved an issue with changing devices from an external MIDI foot controller. Some MIDI messages were not received, because writing i2c is blocking midi data reception. Writing the current page and device is now delayed.
  02-11-2019 v.3.3.4 Added high string priority. Also added basic support for the Strymon Volante.
  18-11-2019 v.3.3.5 Added custom effect types of the MS-50G and th MS-60B to the MS SERIES. Thanks to Dr. Michael Cvachovec for supplying the custom firmware packages for this.
  30-12-2019 Added Katana patch dump from and to editor
  03-12-2019 Added MIDI clock receive.
  27-12-2019 Added MIDI clock transmit. Also the tap tempo LED turns blue whenever it is synced through MIDI clock. The tap tempo LED is now updated from the MIDI clock timer.
  03-01-2020 Found a workaround for the VG-99 displaying sysex receive messages that freeze the user interface. Solved by sending back a PC MIDI message for the current patch. That supresses the messages
  03-01-2020 Fixed UPDOWN function for the VC-mini.
  11-01-2020 Added support for Buydisplay RGB backlight colour scheme.
  11-01-2020 Changed read/write of Katana patches to allow storage of pedal block parameters. EQ and GEQ parameters are now stored in same location to find the space.
  12-01-2020 Added EQ/GEQ and PEDAL block parameters to the edit page of the VController.
  18-01-2020 Fixed the menu on VController and in VC-edit to work with the new parameters. Fixed some bugs in showing the proper parameter values in VC-edit.
  20-01-2020 v.3.4.0 release version. Is backwards compatible with older versions of the Katana. Version is detected at startup.
  27-01-2020 Fixed some bugs: 1) Sysex errors on sending clock via USB host port - temporary workaround: clock is not sent on this port!
                              2) Changing settings will reset them??? Looks like the labels are disappearing on the VC-mini - labels now always show for page select and menu.
                              3) Last MIDI switch can not be edited - fixed on VController and VC-edit
  17-03-2020 Started developing the SY1000 compatibility remotely with Dave LaBrosse
  28-03-2020 EEPROM is now automatically initialized for new values in settings on firmware upgrades
  29-03-2020 SY1000 code in Alpha - waiting on Dave to finish testing
  06-04-2020 Started developing the TC electronics GMajor-2 compatibility remotely with AndyE of vguitarforums.com
  27-04-2020 Cannot read long sysex messages via serial port - therefore patchnames are not read beforehand on GMajor.
  30-04-2020 USB MIDI name has been renamed from Teensy MIDI to VC MIDI
  01-05-2020 GMajor code is now in Alpha - waiting for AndyE to finish testing
  02-05-2020 External pedals (expression and switches) are now hot pluggable
  03-05-2020 Enabled optional SW 4-6 in the firmware of the VC-mini
  08-05-2020 Added the option to show the tempo instead of the device name at the top right of the main menu.
  09-05-2020 Added CURNUM option in Global Menu with option for Previous patch, tap tempo, tuner and direct select.
  22-05-2020 Fixed Helix/VG99 issue with forwarding PC messages. Fix of 03-01-2020 was causing trouble since firmware 2.9 of Helix
  20-07-2020 v.3.5.0 release Added support for the SY1000 and the GMajor 2. Added hot pluggable expression pedals and CURNUM action.
  22-08-2020 Started support for the NUX MG300.
  23-08-2020 Updated the USBHost_t36 library to support longer sysex messages as that is the only way to communicate with the MG300
  30-08-2020 MS SERIES: toggling effect on the VController now selects that effect on the MS SERIES.
  30-08-2020 NUX MG300 effect control implemented
  02-11-2020 Start implementing Katana MK2
  03-11-2020 Fixed HI-LEVEL of EQ not working properly for Katana. It was swapped with Hi-CUT.
  09-11-2020 Added GEQ support for Global Eq Katana.
  14-11-2020 Fixed delay mod parameters were not properly saved
  16-11-2020 Changed behaviour of encoders. Almost all encoder turns use update_encoder_value(). This will make the encoder stop at minimum and maximum values. After one second you can cross from min to max or vise versa
  18-11-2020 Improved the speed of the encoder for parameters with large ranges (like delay time: 0 - 2000 ms)
  30-11-2020 v.3.6.1 release Added support for the NUX MG300 and the Katana MK2.
  07-12-2020 Started with the MIDI CC sequencer for Helix
  01-01-2021 SY-1000 now works through USB Host port. There is some very weird stuff going on here, with data being sent double. But it has been resolved. Also fixed occasional patch name read errors
  03-01-2021 Fixed a lot of bugs in the US20 emulation mode, as I use that for my setup now. Helix will now go to tuner mode to mute it.
  10-01-2021 SY-1000: added option to control all the ctl, num, bank up/down, manual and gk assigns from the VController.
  11-01-2021 SY-1000: all ctl, num, bank and gk ctl functions will also control the regular asdigns via CC #95
  14-01-2021 SY-1000: Expression pedals now also control assigns. There are changed to CC #94 to control
  23-01-2021 SY-1000: Added scene mode.
  29-03-2021 SY-1000: weeks of debugging and working around the quirks of the SY1000. Added scene assigns, use the SY1000 switches for scene selection with VC-mini and much more.
  04-04-2021 SY-1000: Added harmony mode.
  10-04-2021 SY-1000: Added "change to all scenes" to SY1000 menu - to allow updating of changed parameter in all scenes!
  10-04-2021 Changed the memory division: now 150 patches can be stored. The number of commands is now 1737.
  12-04-2021 Helix MIDI CC sequencer can be programmed from editor
  13-04-2021 Release of firmware 3.7.0
  19-04-2021 Added MIDI learn mode for connecting external controllers. It will detect CC_MOMENTARY, CC_RANGE and PC mnessages from external MIDI controllers.
  20-04-2021 FW3.7.1: Changed defaults for MIDI switch and fixed an issue where MIDI channel could be set to zero, resulting in the external MIDI pedal not responding.
  04-05-2021 FW3.7.2: Started with VC-touch implementation
  24-05-2021 FW3.7.3: SY-1000 - Added editor data forwarding, so BTS for SY-1000 can connect through the VC-mini. Selecting scenes on the VC-mini is now properly updated in BTS.
  14-06-2021 Added MIDI forwarding. You can make three "connections" between a srouce and a destination port to forward MIDI data. Forwarded data includes MIDI note on/off, pitch bend, pc, cc and sysex.
  05-07-2021 SY-1000: Added per scene option to mute the inputs while the scene is loaded. This will remove any pops or burts of sound during scene change, but will introduce a minor gap.
  19-07-2021 VC-touch: added on-screen keyboard
  22-07-2021 VC-touch: started with Bluetooth and Wifi support via ESP32 DevKit v1.
  23-07-2021 Improved writing speed of patches. Data of empty patches is no longer sent. Also fixed a bug in writing patch data to EEPROM
  20-09-2021 Improved tap tempo. LED timing is better and max bpm (250) is now easier to tap.
  27-09-2021 Added option to detect different midi in and out port.
  11-10-2021 Added buffers to seral midi ports. This improves serial midi communications and avoids dropouts or sysex errors.
  25-10-2021 Port names and types are made device specific and are set from hardware.h
  08-11-2021 MG-300: Added support for firmware 3.10.13 with the new STAGEMAN amp model.
  13-12-2021 VC-touch: optimized backlight brightnes control
  02-01-2021 VC-edit: made one editor for all three VC versions
  05-01-2022 VC-touch and VC-mini: fixed patch selection on G3 and MS70CDR via USB host port not working.
  06-01-2022 VC-touch: improved switchpad routine for multipress switches, improved edit screen for VG99 and Katana and added colours to the looper progress bar
  21-01-2022 SY-1000: fewer pops in scene change by always first turning parameters off or down before turning parameters on or up.
  31-01-2022 Added automatic tempo following. The tempo is extracted from the Guitar2midi MIDI note information.
  22-02-2022 Katana MK2: GEQ data was not stored and recalled properly. Midi timing has also been tweaked to make sure all data is written to the Katana.
  23-02-2022 Katana: added Global Option to select Katana type. Now CH5-6 will be skipped when using Katana 50W (4CH) 
  24-02-2022 Katana: fixed bug where FX chain would not be updated occasionally when moving from Katana channel patch to VC patch.
  28-02-2022 VC-touch: Titles and labels of switches now contain 16 characters. Text is compressed slightly by eating one pixel of the width of each character.
  01-03-2022 A VC-device can now be extended with another VC-device, by connecting them together. Patches, scenes, tempo, tuner and looper status are synced between the devices. (work in progress)
  12-03-2022 Release of firmware 3.9.0
  14-03-2022 Added menu item firmware/sync patches to pull all patches from another VC device that is connected bi-directionally 
  21-03-2022 Made new hardware file for the VC-mini based upon the Teensy 4.1
  11-04-2022 KPA: fixed not connecting to firmware 3.9.0
  16-04-2022 KPA: fixed tap tempo and allowed for larger rig names on VC-touch.
  29-04-2022 VC-touch: fixed wireless module not connecting to WIDI-master. Also implemented slower connection to editor, so VC-edit can connect wirelessly.
  02-05-2022 MIDI PC: Added PREV, NEXT and BANK select options for it.
  09-05-2022 VC-touch: fixed expression pedal freezing the switches occasionally.
  14-05-2022 MIDI PC/CC/NOTE ON/OFF: New port numbering system was not properly implemented, leading to errors. This has been fixed.
  16-05-2022 VC-touch: centered text is now perfectly centered on display.
  27-05-2022 VC-touch: added menu icon to touch screen
  28-05-2022 KPA: slots are now controlled via snapscenes. Support for morphing through snapscenes. Also performance names are stored in EEPROM for easy browsing.
  13-06-2022 Setlist/song: command structure built for both
  21-09-2022 Added SONG mode, PAGE mode and DEVICE mode to make it clearer that the VC can be used in three ways.
  29-07-2022 Setlist/song: further implemented for VC-touch.
  17-08-2022 Setlist/song: implementation for VC-mini
  02-09-2022 Fixed bug in MENU - PROGRAM SWITCHES where PATCH SELECT was not showing the correct number of patches for certain devices (Katana) both on the unit and in VC-edit.
  03-09-2022 Fixed incorrect cursor position in text entry on VC-mini
  05-09-2022 VC-touch now uses both memory chips (24LC512) and can store 3225 commands, 300 patches and 64 sequences.
  05-09-2022 Added MIDI more command with option to send MIDI start and stop or toggle them.
  19-09-2022 Added option to Block device detect messages. Every second the VC device will send some messages to look for external devices. Enabling this option will block these messages
             Some external devices that are not supported get confused when these messages appear. Be careful using this option, as automatic device detection no longer works on ports where this is enabled.
  21-10-2022 SY-1000: Fixed the normal input not restoring properly from scenes when Quiet scene change was active. Thanks Brad for pointing it out.
  24-10-2022 Release of firmware 3.10.0
  19-11-2022 Updated to Arduino 1.8.19. This has an updated bootloader for the Teensy 4.1. Hopefully these will no longer lose their programming now.
  19-11-2022 VC-mini: fixed menu operation through the switches (menu_prev and menu_next)
  20-12-2022 Added type/mode setting for devices.
  20-12-2022 Helix: type/mode can now be set for Helix, HX-stomp and HX-effects, also taking the two numbering schemes into account.
  20-12-2022 Katana: the Katana 100 / Katana 50 setting has been moved to Device Settings.
  05-12-2022 VC-edit: fixed multiple commands in "on page select" box not selecting properly, making editing hard.
  05-12-2022 Fixed bug that under certain conditions hangs the VController, VC-mini or VC-touch. Doing a String.substring of a string that is too small hangs the sketch!
  05-12-2022 Release of firmware 3.10.2
  04-01-2023 VC-touch: Added pong game - can be controlled by expression pedal or guitar-to-midi!
  06-01-2023 GR-55: added scene mode
  09-01-2023 GR-55: added ctl pedal assigns and removed the regular cc assigns. The CTL and EXP SW function now fully work and also execute associated assigns.
  07-03-2023 VC-mini: fixed MIDI PC labels not showing on BANKSELECT.
  11-03-2023 VC-touch: fixed an issue where the switches would "hang" occasionaly if many switches were pressed at the same time.
  13-03-2023 Added support for switch_holding for SONG, SETLIST, PAGE, ASSIGN, MIDI_PC for NEXT, PREV and bank up/down.
  13-03-2023 GR55: VController no longer has space for the stored patch names of the lead, rhythm and other bank. Currently at 98% of FLASH memory. These patches can be selected, but the names are blank.
  13-03-2023 VC-edit: fixed crash when opening parameter command for current device. Fixed some other minor bugs.
  18-03-2023 VC-edit: fixed bug where user commands would be loaded double right after changing VC device in preferences.
  20-03-2023 Release of firmware 3.11.0
  01-04-2023 Fixed bug where parameters that were not in the first slot were not executed.
  27-04-2023 Added support for MS50G and MS60B (MS60B untested)
  22-08-2023 Added support for user devices
  25-10-2023 VController now runs on Teensy 4.1 - firmware for Teensy 3.2 no longer supported
  26-10-2023 Labels are read from the default page, if there are no commands for the switch on the current page and there is a command on the default page.
  26-10-2023 VC-touch: pressing the device picture will take you to the mode select page.
  26-10-2023 You can set or hide non essential popup messages with a new setting.
  26-10-2023 VC-edit: fixed a number of errors mcopying and deleting items from and to the On Page Select field. Also fixed crash for swapping patches.
  06-11-2023 Setlist select - double pressing a setlist will select the first item of it.
  20-11-2023 VC-touch: Updated ER_TFTM0784 library for support of changed hardware of the display
  */
