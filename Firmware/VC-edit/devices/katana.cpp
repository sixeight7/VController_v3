#include "katana.h"
#include "VController/config.h"
#include "VController/leds.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

void KTN_class::init()
{
    device_name = "KATANA";
    full_device_name = "Boss KATANA";
    patch_min = KTN_PATCH_MIN;
    patch_max = KTN_PATCH_MAX;
    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 2; // Default value: red
    MIDI_channel = KTN_MIDI_CHANNEL; // Default value
    bank_number = 0; // Default value
    is_always_on = true; // Default value
    my_device_page1 = KTN_DEFAULT_PAGE1; // Default value
    my_device_page2 = KTN_DEFAULT_PAGE2; // Default value
    my_device_page3 = KTN_DEFAULT_PAGE3; // Default value
    my_device_page4 = KTN_DEFAULT_PAGE4; // Default value

    InitializePatchArea();
}

bool KTN_class::check_command_enabled(uint8_t cmd)
{
    switch (cmd) {
    case PATCH:
    case PARAMETER:
    //case ASSIGN:
    //case MUTE:
    case OPEN_PAGE_DEVICE:
    case OPEN_NEXT_PAGE_OF_DEVICE:
    case TOGGLE_EXP_PEDAL:
    case MASTER_EXP_PEDAL:
    //case SNAPSCENE:
    //case LOOPER:
        return true;
    }
    return false;
}

QString KTN_class::number_format(uint16_t patch_no)
{
    if (patch_no == 0) return "PANEL";
    else if (patch_no < 9) return "CH" + QString::number(patch_no);
    else return "VC" + QString::number((patch_no - 9) / KTN_BANK_SIZE) + "." + QString::number((patch_no - 9) % KTN_BANK_SIZE + 1);
}

// Parameter categories
#define KTN_CAT_NONE 0 // Some parameters cannot be read - should be in a category
#define KTN_CAT_AMP 1
#define KTN_CAT_BOOST 2
#define KTN_CAT_MOD 3
#define KTN_CAT_DLY1 4
#define KTN_CAT_FX 5
#define KTN_CAT_RVB 6
#define KTN_CAT_DLY2 7
#define KTN_CAT_EQ 8
#define KTN_CAT_MISC 9
#define KTN_CAT_GLOBAL 10
#define KTN_CAT_PEDAL 11
#define KTN_NUMBER_OF_FX_CATEGORIES 11

struct KTN_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  //uint16_t Target; // Target of the assign as given in the assignments of the KTN / GR55
  uint16_t Address; // The address of the parameter
  uint8_t NumVals; // The number of values for this parameter
  QString Name; // The name for the label
  uint16_t Sublist; // Which sublist to read for the FX or amp type - 0 if second byte does not contain the type or if there is no sublist +100 Show value from sublist.
  uint8_t Colour; // The colour for this effect.
  uint8_t Category; // The category of this effect
};

#define KTN_FX_COLOUR 255 // Just a colour number to pick the colour from the KTN_FX_types table
#define KTN_MOD_TYPE_COLOUR 254 //Another number for the FX type
#define KTN_FX_TYPE_COLOUR 253 //Another number for the FX type
#define KTN_PEDAL_TYPE_COLOUR 252
#define KTN_FX_BUTTON_COLOUR 251

#define SUBLIST_FROM_BYTE2 32768 //Bit 16 set high means read sublist frm byte 2
#define SHOW_NUMBER 31767 // Special number for showing the number (used for gain/volume/etc) - set in sublist
#define SHOW_TONE_NUMBER 31766 // To show 00-64:-50-+50
#define SHOW_PAN 31765 // Special number for showing the pan- set in sublist
#define SHOW_DELAY_TIME 31764 // To show 1 - 2000 ms
#define SHOW_CUT_BOOST_20DB 31763 // To show -20 dB - + 20 dB
#define SHOW_RVB_TIME 31762
#define SHOW_PITCH_NUMBER 31761 // To show -24 - +24
#define SHOW_MILLIS 31760 // To show msec in steps of 0.5
#define SHOW_NUMBER_PLUS_ONE 31759 // Show number plus one
#define SHOW_CUT_BOOST_12DB 31758 // GEQ in EQ block has a different scale -12dB  - +24dB

QVector<KTN_parameter_struct> KTN_parameters = {
    {0x0620, 2, "PEDAL SW", 290 | SUBLIST_FROM_BYTE2, FX_FILTER_TYPE, KTN_CAT_PEDAL }, //00
    {0x1111, 3, "PEDAL TYPE", 290, FX_FILTER_TYPE, KTN_CAT_PEDAL },
    {0xB000, 101, "PDL PAR 01", SHOW_NUMBER, KTN_PEDAL_TYPE_COLOUR, KTN_CAT_PEDAL},
    {0xB001, 101, "PDL PAR 02", SHOW_NUMBER, KTN_PEDAL_TYPE_COLOUR, KTN_CAT_PEDAL},
    {0xB002, 101, "PDL PAR 03", SHOW_NUMBER, KTN_PEDAL_TYPE_COLOUR, KTN_CAT_PEDAL},
    {0xB003, 101, "PDL PAR 04", SHOW_NUMBER, KTN_PEDAL_TYPE_COLOUR, KTN_CAT_PEDAL},
    {0xB004, 101, "PDL PAR 05", SHOW_NUMBER, KTN_PEDAL_TYPE_COLOUR, KTN_CAT_PEDAL},
    {0xB005, 101, "PDL PAR 06", SHOW_NUMBER, KTN_PEDAL_TYPE_COLOUR, KTN_CAT_PEDAL},
    {0x0030, 2, "BOOST", 1 | SUBLIST_FROM_BYTE2, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0031, 22, "BST TP", 1, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0032, 121, "BST DRIVE", SHOW_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},  //10
    {0x0033, 101, "BST BOTTOM", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0034, 101, "BST TONE", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0035, 2, "BST SOLO SW", 0, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0036, 101, "BST SOLO LVL", SHOW_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0037, 101, "BST FX LVL", SHOW_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0038, 101, "BST DIR LVL", SHOW_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0039, 8,   "BST CUST.TP", 276, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x003A, 101, "BST CUST.BTM", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x003B, 101, "BST CUST.TOP", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x003C, 101, "BST CUST.LOW", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST}, //20
    {0x003D, 101, "BST CUST.HI", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x003E, 101, "BST CUST.CHAR", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0140, 2, "MOD", 23 | SUBLIST_FROM_BYTE2, KTN_FX_COLOUR, KTN_CAT_MOD},
    {0x0141, 40, "MOD TP", 23, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8000, 101, "MOD PAR 01", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8001, 101, "MOD PAR 02", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8002, 101, "MOD PAR 03", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8003, 101, "MOD PAR 04", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8004, 101, "MOD PAR 05", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8005, 101, "MOD PAR 06", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD}, // 30
    {0x8006, 101, "MOD PAR 07", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8007, 101, "MOD PAR 08", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8008, 101, "MOD PAR 09", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8009, 101, "MOD PAR 10", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x800A, 101, "MOD PAR 11", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x800B, 101, "MOD PAR 12", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x800C, 101, "MOD PAR 13", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x800D, 101, "MOD PAR 14", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x800E, 101, "MOD PAR 15", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x0051, 28, "AMP TP", 63, FX_AMP_TYPE, KTN_CAT_AMP},              // 40
    {0x0052, 121, "AMP GAIN", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP},
    {0x0054, 101, "AMP BASS", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP},
    {0x0055, 101, "AMP MIDDLE", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP},
    {0x0056, 101, "AMP TREBLE", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP},
    {0x0057, 101, "AMP PRESCENCE", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP},
    {0x0059, 2, "AMP BRIGHT", 0, FX_AMP_TYPE, KTN_CAT_AMP},
    {0x0058, 101, "AMP LEVEL", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP},
    {0x0130, 2, "EQ SW", 0, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0x1104, 2, "EQ TYPE", 288, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0xA000, 101, "EQ PAR 01", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ}, //50
    {0xA001, 101, "EQ PAR 02", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0xA002, 101, "EQ PAR 03", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0xA003, 101, "EQ PAR 04", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0xA004, 101, "EQ PAR 05", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0xA005, 101, "EQ PAR 06", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0xA006, 101, "EQ PAR 07", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0xA007, 101, "EQ PAR 08", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0xA008, 101, "EQ PAR 09", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0xA009, 101, "EQ PAR 10", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0xA00A, 101, "EQ PAR 11", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ}, //60
    {0x0560, 2, "DLY1", 91  | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x0561, 11, "DLY1 TP", 91, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x0562, TIME_2000, "DLY1 TIME", SHOW_DELAY_TIME, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x0564, 101, "DLY1 F.BACK", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x0565, 15, "DLY1 H.CUT", 128, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x0566, 101, "DLY1 FX LVL", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x0567, 101, "DLY1 DIR LVL", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x0573, 101, "DLY1 MOD RATE", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x0574, 101, "DLY1 MOD DPTH", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x034C, 2, "FX", 23  | SUBLIST_FROM_BYTE2, KTN_FX_COLOUR, KTN_CAT_FX}, // 70
    {0x034D, 40, "FX TYPE", 23, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9000, 101, "FX PAR 01", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9001, 101, "FX PAR 02", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9002, 101, "FX PAR 03", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9003, 101, "FX PAR 04", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9004, 101, "FX PAR 05", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9005, 101, "FX PAR 06", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9006, 101, "FX PAR 07", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9007, 101, "FX PAR 08", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9008, 101, "FX PAR 09", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX}, //80
    {0x9009, 101, "FX PAR 10", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x900A, 101, "FX PAR 11", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x900B, 101, "FX PAR 12", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x900C, 101, "FX PAR 13", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x900D, 101, "FX PAR 14", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x900E, 101, "FX PAR 15", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x104E, 2, "DLY2", 91  | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE, KTN_CAT_DLY2},
    {0x104F, 11, "DLY2 TP", 91, FX_DELAY_TYPE, KTN_CAT_DLY2},
    {0x1050, TIME_2000, "DLY2 TIME", SHOW_DELAY_TIME, FX_DELAY_TYPE, KTN_CAT_DLY2},
    {0x1052, 101, "DLY2 F.BACK", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY2}, // 90
    {0x1053, 15, "DLY2 H.CUT", 128, FX_DELAY_TYPE, KTN_CAT_DLY2},
    {0x1054, 101, "DLY2 FX LVL", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY2},
    {0x1055, 101, "DLY2 DIR LVL", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY2},
    {0x1061, 101, "DLY2 MOD RATE", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY2},
    {0x1062, 101, "DLY2 MOD DPTH", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY2},
    {0x0610, 2, "RVB SW", 102  | SUBLIST_FROM_BYTE2, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x0611, 7, "RVB TP", 102, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x0612, 100, "RVB TIME", SHOW_RVB_TIME, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x0613, TIME_500, "RVB PRE", SHOW_DELAY_TIME, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x0615, 18, "RVB L.CUT", 112, FX_REVERB_TYPE, KTN_CAT_RVB}, // 100
    {0x0616, 15, "RVB H.CUT", 128, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x0617, 11, "RVB DENS", SHOW_NUMBER, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x0618, 101, "FX LVL", SHOW_NUMBER, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x0619, 101, "DIR LVL", SHOW_NUMBER, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x061A, 101, "SPRING SENS", SHOW_NUMBER, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x0655, 2, "S/R LOOP", 0, FX_FILTER_TYPE, KTN_CAT_MISC},
    {0x0656, 2, "S/R MODE", 149, FX_FILTER_TYPE, KTN_CAT_MISC},
    {0x0657, 101, "S/R SEND LVL", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_MISC},
    {0x0658, 101, "S/R RET. LVL", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_MISC},
    {0x0663, 2, "N/S SW", 0, FX_DYNAMICS_TYPE, KTN_CAT_MISC}, // 110
    {0x0664, 101, "N/S THRESH", SHOW_NUMBER, FX_DYNAMICS_TYPE, KTN_CAT_MISC},
    {0x0665, 101, "N/S RLEASE", SHOW_NUMBER, FX_DYNAMICS_TYPE, KTN_CAT_MISC},
    {0x121F, 9, "EXP ASGN", 293, FX_FILTER_TYPE, KTN_CAT_MISC },
    {0x1220, 9, "GAFC EXP1 ASGN", 293, FX_FILTER_TYPE, KTN_CAT_MISC },
    {0x1221, 9, "GAFC EXP2 ASGN", 293, FX_FILTER_TYPE, KTN_CAT_MISC },
    {0x0633, 101, "FOOT VOL", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_MISC},
    {0x1210, 3, "BOOST COLOR", 109, KTN_FX_BUTTON_COLOUR, KTN_CAT_MISC},
    {0x1211, 3, "MOD COLOR", 109, KTN_FX_BUTTON_COLOUR, KTN_CAT_MISC},
    {0x1212, 3, "DLY1 COLOR", 109, KTN_FX_BUTTON_COLOUR, KTN_CAT_MISC},
    {0x1213, 3, "FX COLOR", 109, KTN_FX_BUTTON_COLOUR, KTN_CAT_MISC},  // 120
    {0x1214, 3, "RVB COLOR", 109, KTN_FX_BUTTON_COLOUR, KTN_CAT_MISC},
    {0x7430, 3, "L/OUT AIR", 268, FX_AMP_TYPE, KTN_CAT_GLOBAL}, // Address 0x7xxx is translated to 0x00000xxx
    {0x7431, 3, "CAB RESO", 271, FX_AMP_TYPE, KTN_CAT_GLOBAL},
    {0x7432, 2, "GLBL EQ SW", 0, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x743E, 2, "GL EQ POS", 273, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x7433, 18, "GEQ LOW CUT", 112, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x7434, 41, "GEQ LOW LVL", SHOW_CUT_BOOST_20DB, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x7435, 26, "GEQ L-M F", 113, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x7436, 6, "GEQ L-M Q", 143, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x7437, 41, "GEQ L-M LVL", SHOW_CUT_BOOST_20DB, FX_FILTER_TYPE, KTN_CAT_GLOBAL}, // 130
    {0x7438, 26, "GEQ H-M F", 113, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x7439, 6, "GEQ H-M Q", 143, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x743A, 41, "GEQ H-M LVL", SHOW_CUT_BOOST_20DB, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x743B, 15, "GEQ HI CUT", 128, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x743C, 41, "GEQ HI LVL", SHOW_CUT_BOOST_20DB, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x743D, 101, "GEQ LEVEL", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
};

const uint16_t KTN_NUMBER_OF_PARAMETERS = KTN_parameters.size();

QStringList KTN_sublists = {
      // Sublist 1 - 22: Booster types
      "MILD B", "CLEAN B", "TREBLE B", "CRUNCH", "NAT OD", "WARM OD", "FAT DS", "LEAD DS", "METAL DS", "OCT.FUZZ",
      "BLUES OD", "OD-1", "TUBESCRM", "TURBO OD", "DIST", "RAT", "GUVNR DS", "DST+", "METAL ZN", "60s FUZZ",
      "MUFF FZ", "CUSTOM",

      // Sublist 23 - 62: MOD/FX types
      "TOUCHWAH", "AUTO WAH", "SUB WAH", "COMPRSOR", "LIMITER", "DIST", "GRAPH EQ", "PARAM EQ", "TONE MOD", "GTR SIM",
      "SLOW GR", "DEFRET", "WAV SNTH", "SITARSIM", "OCTAVE", "PITCH SH", "HARMONST", "SND HOLD", "AC. PROC", "PHASER",
      "FLANGER", "TREMOLO", "ROTARY 1", "UNI-V", "PAN", "SLICER", "VIBRATO", "RING MOD", "HUMANIZR", "2x2 CHOR",
      "SUB DLY", "AcGtrSim", "ROTARY 2", "TeraEcho", "OVERTONE", "PHAS 90E", "FLGR117E", "WAH 95E", "DC30", "HEAVYOCT", // Tera Echo and Overtone do not work for the MOD FX

      // Sublist 63 - 90 : Amp types
      "NAT CLN", "ACOUSTIC", "COMBO C", "STACK C", "HiGAIN", "POWER DR", "EXTREME", "CORE MTL", "CLEAN!", "CLN TWIN",
      "PRO CR", "CRUNCH!", "DELUXE 2", "VO DRIVE", "VO LEAD", "MATCH DR", "BG LEAD", "BG DRIVE", "MS1959I", "MS1959II",
      "RFIER V", "RFIER M", "T-AMP L", "BROWN!", "LEAD!", "CUSTOM", "BGNR UBR", "ORNGE RR",

      // Sublist 91 - 101 : Delay types
      "DIGITAL", "PAN", "STEREO", "DUAL SER", "DUAL PAR", "DUAL L/R", "REVERSE", "ANALOG", "TAPE ECO", "MODULATE",
      "SDE-3000",

      // Sublist 102 - 108 : Reverb types
      "AMBIENCE", "ROOM", "HALL 1", "HALL 2", "PLATE", "SPRING", "MODULATE",

      // Sublist 109 - 111 : FX color types
      "GREEN", "RED", "YELLOW",

      // Sublist 112 - 142: Frequencies
      // Low: start at 112 -
      // Mid - start at 113
      // High - start at 128
      "FLAT", "20.0Hz", "25.0Hz", "31.5Hz", "40.0Hz", "50.0Hz", "63.0Hz", "80.0Hz", "100Hz", "125Hz",
      "160Hz", "200Hz", "250Hz", "315Hz", "400Hz", "500Hz", "630Hz", "800Hz", "1.00kHz", "1.25kHz",
      "1.60kHz", "2.0kHz", "2.50kHz", "3.15kHz", "4.00kHz", "5.00kHz", "6.30kHz", "8.00kHz", "10.0kHz", "12.5kHz",
      "FLAT",

      // Sublist 143 - 148 - Mid Q
      "0,5", "1", "2", "4", "8", "16",

      // Sublist 149 - 150 - S/R mode
      "SERIES", "PARALLEL",

      // Sublist 151 - 152: Wah Mode
      "LPF", "BPF",

      // Sublist 153 - 154 T.Wah polar
      "DOWN", "UP",

      // Sublist 155 - 160 Wah types
      "CRY WAH", "VO WAH", "FAT WAH", "LITE WAH", "7STR.WAH", "RESO WAH",

      // Sublist 161 - 168 Compressor types
      "BOSS CMP", "HI-BAND", "LIGHT", "D-COMP", "ORANGE", "FAT", "MILD", "ST COMP",

      // Sublist 169 - 171: Limiter types
      "BOSS LIM",  "RACK160D", "VTGRACKU",

      // Sublist 172 - 179: Tone MOD types
      "FAT", "PRESENCE", "MILD", "TIGHT", "ENHANCE", "RESO1", "RESO2", "RESO3",

      // Sublist 180 - 187: Guitar sim types
      "S->H", "H->S", "H->HF", "S->HLW", "H->HLW", "S->AC", "H->AC", "P->AC",

      // Sublist 188 - 189: Wave synth WAVE types
      "SAW", "SQUARE",

      // Sublist 190 - 193: Octave ranges
      "1: B1-E6", "2:B1-E5", "3:B1-E4", "4:B1-E3",

      // Sublist 194 - 195: Pitch shift voices
      "1-VOICE", "2-MONO",

      // Sublist 196 - 199: Pitch shift modes
      "FAST", "MEDIUM", "SLOW", "MONO",

      // Sublist 200 - 229
      "-2oct", "-14th", "-13th", "-12th", "-11th", "-10th", "-9th",
      "-1oct", "-7th", "-6th", "-5th", "-4th", "-3rd", "-2nd", "TONIC",
      "+2nd", "+3rd", "+4th", "+5th", "+6th", "+7th", "+1oct", "+9th", "+10th", "+11th",
      "+12th", "+13th", "+14th", "+2oct", "USER",

      // Sublist 230 - 233: Ac processor types
      "SMALL", "MEDIUM", "BRIGHT", "POWER",

      // Sublist 234 - 237: Phaser types
      "4STAGE", "8STAGE", "12STAGE", "BiPHASE",

      // Sublist 238 - 239: Rotary speed
      "SLOW", "FAST",

      // Sublist 240 - 241 PAN types
      "AUTO", "MANUAL",

      // Sublist 242 -243 RING MOD modes
      "NORMAL", "INTLGNT",

      // Sublist 244 -245 HUMANIZER modes
      "PICKING", "AUTO",

      // Sublist 246 -250 HUMANIZER vowels
      "a", "e", "i", "o", "u",

      // 251 - 252 SUB DELAY types
      "MONO", "PAN",

      // 253 - 255 TERA ECHO MODES
      "MONO", "ST1", "ST2",

      // Sublist 256 - 267 for key
      "C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B",

      // Sublist 268 - 270 for Line Out Live Feel
      "REC", "LIVE", "BLEND",

      // Sublist 271 - 273 for Cabinet Resonance
      "VINTAGE", "MODERN", "DEEP",

      // Sublist 274 - 275 for Global eq position
      "INPUT", "OUTPUT",

      // Sublist 276 - 283 for Boost Custom types
      "OD-1", "OD-2", "CRUNCH", "DS-1", "DS-2", "METAL-1", "METAL-2", "FUZZ",

      // Sublist 284 - 285 for DC30 selector
      "CHORUS", "ECHO",

      // Sublist 286 - 287 for DC30 output select
      "D+E", "D/E",

      // Sublist 288 - 289 for EQ type
      "PARAMTRC", "GRAPHIC",

      // Sublist 290 - 292 for pedal type
      "WAH", "PDL BEND", "WAH 95E",
};

const uint16_t KTN_SIZE_OF_SUBLIST = KTN_sublists.size();

QString KTN_class::read_parameter_name(uint16_t par_no)
{
    if (par_no < number_of_parameters())  return KTN_parameters[par_no].Name;
      else return "?";
}

QString KTN_class::read_parameter_state(uint16_t par_no, uint8_t value)
{
    QString Output = "";
    if (par_no < number_of_parameters())  {
        uint16_t my_sublist = KTN_parameters[par_no].Sublist;
        if ((my_sublist > 0) && !(my_sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
          switch (my_sublist) {
            case SHOW_NUMBER:
            case SHOW_DELAY_TIME:
              Output += QString::number(value);
              break;
            case SHOW_TONE_NUMBER:
              if (value > 50) Output += "+";
              Output += QString::number(value - 50);
              break;
            case SHOW_PAN:
              if (value < 64) Output += "L" + QString::number(50 - value);
              if (value == 64) Output += "C";
              if (value > 64) Output += "R" + QString::number(value - 50);
              break;
            case SHOW_CUT_BOOST_20DB:
              if (value > 20)  Output += "+";
              Output += QString::number(value - 20);
              Output += "dB";
              break;
            case SHOW_CUT_BOOST_12DB:
              if (value > 24)  Output += "+";
              Output += QString::number((value - 24) / 2);
              if ((value - 24) % 2 == 0) Output += ".0";
              else Output += ".5";
              Output += "dB";
              break;
            case SHOW_RVB_TIME:
              value++;
              Output += QString::number(value / 10);
              Output += ".";
              Output += QString::number(value % 10);
              Output += "sec";
              break;
            case SHOW_PITCH_NUMBER:
              if (value > 24)  Output += "+";
              Output += QString::number(value - 24);
              break;
            case SHOW_MILLIS:
              Output += QString::number(value / 2);
              Output += ".";
              Output += QString::number((value & 1) * 5);
              Output += "ms";
              break;
            case SHOW_NUMBER_PLUS_ONE:
              Output += QString::number(value + 1);
              break;
            default:
              Output += KTN_sublists[my_sublist + value - 1];
              break;
          }
        }
        else if (value == 1) Output += "ON";
        else Output += "OFF";
      }
      else Output += "?";
    return Output;
}

uint16_t KTN_class::number_of_parameters()
{
    return KTN_NUMBER_OF_PARAMETERS;
}

uint8_t KTN_class::max_value(uint16_t par_no)
{
    if (par_no >= KTN_NUMBER_OF_PARAMETERS) return 0;
    uint8_t Max = KTN_parameters[par_no].NumVals;
    if (Max == TIME_2000) return 0; // Not properly implemented yet to set larger numbers in commands
    if (Max == TIME_1000) return 0; // Use tap tempo to set the tempo...
    if (Max == TIME_500) return 0;
    if (Max == TIME_300) return 0;
    return Max - 1;
}

// Patch memory structure on VController
#define KTN_PATCH_NAME_INDEX     0  // Patch name (16 parameters)
#define KTN_BOOST_INDEX         16  // Boost FX (15 parameters)
#define KTN_AMP_INDEX           31  // Amp index (9 parameters)
#define KTN_EQ_SW_INDEX         40  // Eq (1 parameters)
#define KTN_EQ_GEQ_INDEX        41  // Eq or GEQ (11 parameters)
#define KTN_MOD_BASE_INDEX      52  // Mod base (2 parameters: ON/OFF and type)
#define KTN_MOD_INDEX           54  // MOD effect settings for active effect (max 15 parameters)
#define KTN_FX_BASE_INDEX       69  // FX base (2 parameters: ON/OFF and type)
#define KTN_FX_INDEX            71  // FX effect settings for active effect (max 15 parameters)
#define KTN_DELAY1_INDEX        86  // Delay 1 (18 parameters)
#define KTN_DELAY1_SDE_INDEX   104  // Delay 1 SDE-3000 settings (5)
#define KTN_DELAY2_INDEX       109  // Delay 2 (18 parameters)
#define KTN_DELAY2_SDE_INDEX   127  // Delay 2 SDE-3000 settings (5)// 074 - 084 Reverb (11 parameters)
#define KTN_REVERB_INDEX       132  // Reverb (11 parameters)
#define KTN_FOOT_VOL_INDEX     143  // Foot volume (4 parameters)
#define KTN_S_R_LOOP_INDEX     147  // S/R loop (4 parameters)
#define KTN_NOISE_GATE_INDEX   151  // Noise gate (4 parameters)
#define KTN_MASTER_KEY_INDEX   155  // Master key
#define KTN_FX_CHAIN_INDEX     156  // FX chain (20 parameters)
#define KTN_EQ_TYPE_INDEX      176  // GEQ (1 parameter)
#define KTN_PEDAL_SW_INDEX     177  // Pedal sw  (1 parameters: ON/OFF)
#define KTN_PEDAL_TYPE_INDEX   178  // Pedal type  (1 parameter)
#define KTN_PEDAL_INDEX        179  // Wah type (6 parameters)
#define KTN_EXP_ASSIGNS_INDEX  185  // Assignments for the expression pedals (3 bytes)
#define KTN_SPARE_BYTES        188  // Four bytes left

// 192 Total memory size

struct KTN_patch_memory_struct { // Address map for the essential patch data that will be stored
  uint16_t Address; // The start address of the data on the Katana
  uint8_t Length;   // The Length of the data
  uint8_t Index;    // The index in the KTN_patch_buffer[] array
  QString Name;     // The name for the json file
};

// Copy from VController_v3/MD_KTN.ino - Added the 15 parameters for MOD and FX!
QVector<KTN_patch_memory_struct> KTN_patch_memory = {
  { 0x0720, 20, KTN_FX_CHAIN_INDEX, "CHAIN"     },  // FX chain
  { 0x0051, 9, KTN_AMP_INDEX, "AMP"             },  // Amp
  { 0x0030, 15, KTN_BOOST_INDEX, "BOOST"        },  // Boost effect
  { 0x0130, 1 + 11, KTN_EQ_SW_INDEX, "EQ"       },  // Eq + data
  { 0x1104, 1, KTN_EQ_TYPE_INDEX, "EQ_TYPE"  },  // eq type
  { 0x0140, 2 + 15, KTN_MOD_BASE_INDEX, "MOD"   },  // Mod base
  { 0x034C, 2 + 15, KTN_FX_BASE_INDEX, "FX"     },  // FX base
  { 0x0630, 4, KTN_FOOT_VOL_INDEX, "FOOTVOL"    },  // Foot volume
  { 0x0655, 4, KTN_S_R_LOOP_INDEX, "S_R_LOOP"   },  // S/R loop
  { 0x0560, 18, KTN_DELAY1_INDEX, "DELAY1"      },  // Delay 1
  { 0x0610, 11, KTN_REVERB_INDEX, "REVERB"      },  // Reverb
  { 0x104E, 18, KTN_DELAY2_INDEX, "DELAY2"      },  // Delay 2
  { 0x0663, 4, KTN_NOISE_GATE_INDEX, "N_GATE"   },  // Noise gate
  { 0x0718, 1, KTN_MASTER_KEY_INDEX, "M_KEY"    },  // Master key
  { 0x1049, 5, KTN_DELAY1_SDE_INDEX, "DLY1_SDE" },  // Delay 1 - SDE3000 settings
  { 0x1049, 5, KTN_DELAY2_SDE_INDEX, "DLY2_SDE" },  // Delay 2 - SDE3000 settings
  { 0x0620, 1, KTN_PEDAL_SW_INDEX, "PEDAL SW"   },  // Pedal SW
  { 0x1111, 1 + 6, KTN_PEDAL_TYPE_INDEX, "PEDAL TP" },  // Pedal type
  { 0x0626, 1, KTN_EXP_ASSIGNS_INDEX, "EXP ASGN"},  // Assigns for expression pedals
  { 0x0000, 16, KTN_PATCH_NAME_INDEX, "P_NAME"  },  // Patch name (1)
};

const uint8_t KTN_NUMBER_OF_PATCH_MESSAGES = KTN_patch_memory.size();

QByteArray KTN_class::ReadPatch(int number)
{
    if (number >= KTN_MAX_NUMBER_OF_KATANA_PRESETS) return 0;
    QByteArray patch;
    for (int i = 0; i < KTN_PATCH_SIZE; i++) {
        patch.append(KTN_patches[number][i]);
    }
    return patch;
}

QString KTN_class::ReadPatchName(int number)
{
    QString line = "";
    for (int c = 0; c < 16; c++) {
        line.append(static_cast<char>(KTN_patches[number][c]));
    }
    return line.trimmed();
}

QString KTN_class::ReadPatchStringForListWidget(int number)
{
    QString line = "";
    line.append(number_format(number + 9));
    line.append(":\t");
    for (int c = 0; c < 16; c++) {
        line.append(static_cast<char>(KTN_patches[number][c]));
    }
    line.append("   \t( Amp: ");
    int amp_type = KTN_patches[number][KTN_AMP_INDEX];
    line.append(KTN_sublists[59 + amp_type]);
    line.append(",  Mod:");
    int mod_type = KTN_patches[number][KTN_MOD_BASE_INDEX + 1];
    line.append(KTN_sublists[22 + mod_type]);
    line.append(",  FX:");
    int fx_type = KTN_patches[number][KTN_FX_BASE_INDEX + 1];
    line.append(KTN_sublists[22 + fx_type]);
    line.append(" )");
    return line;
}

void KTN_class::WritePatch(int number, QByteArray patch)
{
    if (number >= KTN_MAX_NUMBER_OF_KATANA_PRESETS) return;
    if (patch.size() > KTN_PATCH_SIZE) return;
    for (int i = 0; i < KTN_PATCH_SIZE; i++) {
        KTN_patches[number][i] = patch[i];
    }
}

void KTN_class::WritePatchName(int number, QString name)
{
    int len = name.length();
    if (len > 16) len = 16;
    for (int c = 0; c < len; c++) {
        KTN_patches[number][c] = name.at(c).toLatin1();
    }
    for (int c = len; c < 16; c++) {
        KTN_patches[number][c] = 32; //space
    }
}

void KTN_class::InitializePatchArea()
{
    for (int p = 0; p < KTN_MAX_NUMBER_OF_KATANA_PRESETS; p++) {
        for (int i = 0; i < KTN_PATCH_SIZE; i++) {
            KTN_patches[p][i] = KTN_default_patch[i];
        }
    }
}

void KTN_class::readAll(const QJsonObject &json)
{
    QJsonObject allPatches = json["KTN Patches"].toObject();
    for (int p = 0; p < KTN_MAX_NUMBER_OF_KATANA_PRESETS; p++) {
        QJsonObject patchObject = allPatches["KTN_Patch_" + QString::number(p)].toObject();
        if (!patchObject.isEmpty()) {
            readPatchData(p, patchObject);
        }
    }
}

void KTN_class::readPatchData(int patch_no, const QJsonObject &json)
{
    for (int m = 0; m < KTN_NUMBER_OF_PATCH_MESSAGES; m++) {
        QString blockName = KTN_patch_memory[m].Name;
        QJsonObject patchBlock = json[blockName].toObject();
        if (!patchBlock.isEmpty()) {
            for (int i = 0; i < KTN_patch_memory[m].Length; i++) {
                int value = patchBlock["Data"+ QString::number(i)].toInt();
                KTN_patches[patch_no][KTN_patch_memory[m].Index + i] = value;
            }
        }
    }
}

void KTN_class::writeAll(QJsonObject &json) const
{
    QJsonObject allPatches;
    for (int p = 0; p < KTN_MAX_NUMBER_OF_KATANA_PRESETS; p++) {
        QJsonObject patchObject;
        writePatchData(p, patchObject);
        allPatches["KTN_Patch_" + QString::number(p)] = patchObject;
        //patchArray.append(patchObject);
    }
    json["KTN Patches"] = allPatches;
}

void KTN_class::writePatchData(int patch_no, QJsonObject &json) const
{
    for (int m = 0; m < KTN_NUMBER_OF_PATCH_MESSAGES; m++) {
        QJsonObject patchBlock;
        for (int i = 0; i < KTN_patch_memory[m].Length; i++) {
            int value = KTN_patches[patch_no][KTN_patch_memory[m].Index + i];
            patchBlock["Data"+ QString::number(i)] = value;
        }
        json[KTN_patch_memory[m].Name] = patchBlock;
    }
}

void KTN_class::swapPatch(int patch_no1, int patch_no2)
{
    for (int i = 0; i < KTN_PATCH_SIZE; i++) {
        int temp = KTN_patches[patch_no1][i];
        KTN_patches[patch_no1][i] = KTN_patches[patch_no2][i];
        KTN_patches[patch_no2][i] = temp;
    }
}

void KTN_class::movePatch(int source_patch, int dest_patch)
{
    if (dest_patch > source_patch) {
        for (int i = source_patch; i < dest_patch; i++) {
            swapPatch(i, i + 1);
        }
    }
    if (dest_patch < source_patch) {
        for (int i = source_patch; i --> dest_patch;) {
            swapPatch(i, i + 1);
        }
    }
}

void KTN_class::pastePatch(int number)
{
    if (!copyBufferFilled) return;
    for (int i = 0; i < KTN_PATCH_SIZE; i++) {
       KTN_patches[number][i] = KTN_copy_buffer[i];
    }
    qDebug() << "Pasted Katana patch" << number;
}

void KTN_class::copyPatch(int number)
{
    for (int i = 0; i < KTN_PATCH_SIZE; i++) {
        KTN_copy_buffer[i] = KTN_patches[number][i];
    }
    copyBufferFilled = true;
    qDebug() << "Pasted Katana patch" << number;
}
