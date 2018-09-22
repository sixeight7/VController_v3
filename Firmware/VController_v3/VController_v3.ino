/****************************************************************************
**
** Copyright (C) 2015 Catrinus Feddema
** All rights reserved.
** This file is part of "VController v2" teensy software.
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
****************************************************************************/

// This page has the following parts:
// Section 1: Main setup() and loop()
// Section 2: VController documentation overview
// Section 3: VController project log

// ********************************* Section 1: Library declaration ********************************************

//#include <Wire.h>
#include <i2c_t3.h>
#include "debug.h"
#include "hardware.h" // Hardware of production model
//#include "hardware1.h" // Hardware of first model of sixeight
//#include "hardware_RM.h" // Hardware of first model of Ryan Muar
#include "globals.h"
#define VCONTROLLER_FIRMWARE_VERSION_MAJOR 3
#define VCONTROLLER_FIRMWARE_VERSION_MINOR 1
#define VCONTROLLER_FIRMWARE_VERSION_BUILD 0

void setup() {
  SCO_switch_power_on();
  setup_LED_control(); //Should be first, to reduce startup flash of Neopixel LEDs
  
  // Wire speeds are set in hardware.h
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, WIRE_SPEED);
  #ifdef WIRE1_SPEED
  Wire1.begin(I2C_MASTER, 0x00, I2C_PINS_29_30, I2C_PULLUP_INT, WIRE1_SPEED);
  #endif
  
  setup_LCD_control();
  setup_debug();
  setup_devices();
  setup_eeprom();
  setup_switch_check();
  setup_switch_control();
  setup_page();
  setup_MIDI_common();
  check_all_devices_for_manual_connection();
}

void loop() {
  main_switch_check(); // Check for switches pressed
  main_switch_control(); //If switch is pressed, take the configured action
  main_LED_control(); //Check update of LEDs
  main_LCD_control(); //Check update of displays
  main_MIDI_common(); //Read MIDI ports
  main_page(); // Check update of current page
  main_devices();
}

// ********************************* Section 2: VController documentation overview  ********************************************

/* VController version 3.

Hardware;
- 16 momentary switches connected to display boards or in keypad matrix
- 12  neopixel LEDs
- 12 LCD displays for first 12 switches + main LCD display - all 16x2 character displays
- Teensy 3.2
- 3 MIDI input/outputs and MIDI over USB. Support for external Raspberry Pi running VCbridge to add extra ports.
- Extra EEPROM Flash memory: 24LC512

Software features:
- Patch and parameter control for Boss GP-10, Roland GR-55, Roland VG-99, Zoom G3 and Zoom MS70-cdr
- Patchnames are read from devices
- Parameter states and assigns are read from devices
- Programmable pages of switches. These can be programmed on the Config page

Software structure:
- Initialization of LCDs, LEDs and MIDI ports
- Configuration of the pages and switches is done in the page[] array on the Config page.
- On startup, detection of devices, page change or patch change the current page is loaded in the SP[] array (void load_page)
- After the page is loaded, the patchnames and parameter states are read (void Request_first_switch, Request_next_switch and PAGE_request_current_switch)
- Main loop checks for switches being pressed (Switch_check/Switch Control), midi received and update of LCD and LED states.

*/

// ********************************* Section 3: VController project log  ********************************************

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
24-06-2017 Added new FX types for ZMS70-cdr version 2.0.0
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
04-11-2017 ZMS70cdr bug not showing patch name fixed. VController now diplaying full version number on powerup. Main display top line behaviour updated.
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
15-09-2018 Alpha testing release 3.1. Fixed minor bugs, improved patch reading for G3/MS70-cdr, GR55 direct select improved, M13 properly switched between EXP1 and 2. Updated fixed configuration for GR55, Katana and direct select
22-09-2018 When switching between pages with different bank sizes, the bank with the current patch is now always shown.
*/


