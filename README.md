# VController_v3
Production model of the VController

The V-Controller is a MIDI footcontroller with 16 buttons, 12 LCDs and 12 LEDs.
It can be fully programmed both onboard and via an external editor for Mac and PC called VC-edit.

# Device support
It support a number of guitar modelers and symths, where it actively read patch names and parameter states for the following devices:
* Fractal Audio Axe-fx 2 (other models may work too, but have not been tested yet)
* Boss GP-10
* Roland GR-55
* Roland VG-99
* Boss Katana (50/100/Artist)
* Kemper Profiling Amp
* Line 6 Helix (limited)
* Line 6 M13
* Zoom G3
* Zoom MS70-cdr

# General MIDI
The V-Controller is also able to send MIDI PC, CC and Note on/off messages to control any device that is able to receive these messages.

# Programmability
The V-Controller supports up to 200 user pages, where a total of almost 3000 commands can be programmed. Any button can execute up to 50 commands.

# Connectivity
* There are two MIDI in/out conenctors and MIDI 2 doubles as an RRC2 connection for the VG-99.
* Four external expression pedals or eight external switches can be connected.
* One USB slave connector (also used for programming the V-Controller)
* An internal or external Raspberry Pi can add four USB host ports to the V-Controller.

# Hardware
This repository contains the building plans (enclosure, PCB and schematics) for the VController and the optional external Raspberry Pi. There is a building guide included which can be followed.

# Firmware
This repository also contains the source code for the V-Controller and for VC-edit. The source code for VC-bridge (the code for the raspberry pi) is in a separate repository called VC-bridge. There are also ready made hex files for the V-Controllers Teensy. There is also special firmware that can check the VControllers hardware.

For VC-edit there is a ready to run zip file for windows and dmg-file for Mac. Also the source code of VC-edit is provided.

# Support
Please visit https://www.vguitarforums.com/smf/index.php?board=277.0 for additional information or for any questions you have regarding this project. Please post any questions on the board, where several V-Controller users and builders can respond and help.

I currently do not build any more V-Controllers to sell, so if you would like one you have to build it yourself. Visit vguitarforums.com to connect to other people who are building the same project.

I have invested thousands of hours in this project. You can support me here: https://www.paypal.me/sixeight


