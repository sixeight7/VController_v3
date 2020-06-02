# VController_v3
Production model of the VController and the VC-mini

![VController]()

The V-Controller is a MIDI footcontroller with 16 buttons, 12 LCDs and 12 LEDs.
It can be fully programmed both onboard and via an external editor for Mac and PC called VC-edit for VController.

The VC-mini is a small version of the V-Controller with 3 buttons, 3 LEDs, 2 encoders and 1 LCD display.
Both the V-Controller and the VC-mini share the same code for the Teensy firmware and the editor (Qt)

# Device support
A number of guitar modelers and synths are supported, where it actively read patch names and parameter states for the following devices:
* Fractal Audio Axe-fx 2 (other models may work too, but have not been tested yet)
* Boss GP-10
* Roland GR-55
* Roland VG-99
* Boss Katana (50/100/Artist)
* Boss SY-1000
* Kemper Profiling Amp
* Line 6 Helix (limited)
* Line 6 M13
* Zoom G3
* Zoom MS70-cdr
* Strymon Volante
* TC Electronics G-Major

# General MIDI
The V-Controller and VC-mini are also able to send MIDI PC, CC and Note on/off messages to control any device that is able to receive these messages.

# Programmability
The V-Controller and VC-mini support up to 200 user pages, where a total of almost 3000 commands can be programmed. Any button can execute up to 50 commands.

# Connectivity of the V-Controller
* There are two MIDI in/out conenctors and MIDI 2 doubles as an RRC2 connection for the VG-99.
* Four external expression pedals or eight external switches can be connected.
* One USB slave connector (also used for programming the V-Controller)
* An internal or external Raspberry Pi can add four USB host ports to the V-Controller.

# Connectivity of the VC-mini
* Two 7 pin MIDI connectors with MIDI in/out and power in one connector. MIDI1 has a seperate 7 pin connector with MIDI in/out reversed. The 7 pin connectors are compatible with regular 5 pin MIDI connectors.
* One external expression pedal or two external switches can be connected via a 3.5 mm jack plug (may need an adapter to regular jack.)
* One USB slave connector (also used for programming the V-Controller)
* One USB host port

# Hardware
This repository contains the building plans (enclosure, PCB and schematics) for the VC-mini, the VController and the optional external Raspberry Pi. Check out the building guides for both projects.

# Firmware
This repository also contains the source code for the V-Controller, the VC-mini and for VC-edit. The source code for VC-bridge (the code for the raspberry pi) is in a separate repository called VC-bridge. There are also ready made hex files for the V-Controllers Teensy. There is also special firmware to check hardware of the V-Controller or the VC-mini.

For VC-edit there is a ready to run zip file for windows and dmg-file for Mac. Also the source code of VC-edit is provided.

# Support
Please visit https://www.vguitarforums.com/smf/index.php?board=277.0 for additional information or for any questions you have regarding this project. Please post any questions on the board, where several V-Controller/VC-mini users and builders can respond and help.

I currently do not build any more V-Controllers to sell, so if you would like one you have to build it yourself. Visit vguitarforums.com to connect to other people who are building the same project.

VC-mini's are built in small numbers. Also the PCB can be optained from me. Send me an e-mail at SixEightSoundControl@gmail.com to get more information on availability and pricing.

I have invested thousands of hours in this project. You can support me here: https://www.paypal.me/sixeight


