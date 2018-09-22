#include "katana.h"
#include "VController/config.h"
#include "VController/leds.h"

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
    my_device_page1 = PAGE_KTN_PATCH_BANK; // Default value
    my_device_page2 = PAGE_KTN_FX; // Default value
    my_device_page3 = 0; // Default value
    my_device_page4 = 0; // Default value
}

bool KTN_class::check_command_enabled(uint8_t cmd)
{
    switch (cmd) {
    case PATCH_SEL:
    case PARAMETER:
    //case ASSIGN:
    case PATCH_BANK:
    case BANK_UP:
    case BANK_DOWN:
    case NEXT_PATCH:
    case PREV_PATCH:
    //case MUTE:
    case OPEN_PAGE_DEVICE:
    case OPEN_NEXT_PAGE_OF_DEVICE:
    case TOGGLE_EXP_PEDAL:
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
    else return "VC" + QString::number((patch_no - 9) / 8) + "." + QString::number((patch_no - 9) % 8 + 1);
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
#define KTN_NUMBER_OF_FX_CATEGORIES 10

struct KTN_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  //uint16_t Target; // Target of the assign as given in the assignments of the KTN / GR55
  uint16_t Address; // The address of the parameter
  uint8_t NumVals; // The number of values for this parameter
  QString Name; // The name for the label
  uint16_t Sublist; // Which sublist to read for the FX or amp type - 0 if second byte does not contain the type or if there is no sublist +100 Show value from sublist.
  uint8_t Colour; // The colour for this effect.
  uint8_t Category; // The category of this effect
};

#define KTN_FX_COLOUR 255 // Just a colour number to pick the colour from the KTN_FX_colours table
#define KTN_MOD_TYPE_COLOUR 254 //Another number for the FX type
#define KTN_FX_TYPE_COLOUR 253 //Another number for the FX type
#define KTN_FX_BUTTON_COLOUR 253

#define SUBLIST_FROM_BYTE2 32768 //Bit 16 set high means read sublist frm byte 2
#define SHOW_NUMBER 31767 // Special number for showing the number (used for gain/volume/etc) - set in sublist
#define SHOW_TONE_NUMBER 31766 // To show 00-64:-50-+50
#define SHOW_PAN 31765 // Special number for showing the pan- set in sublist
#define SHOW_DELAY_TIME 31764 // To show 1 - 2000 ms
#define SHOW_CUT_BOOST 31763 // To show -20 dB - + 20 dB
#define SHOW_RVB_TIME 31762
#define SHOW_PITCH_NUMBER 31761 // To show -24 - +24
#define SHOW_MILLIS 31760 // To show msec in steps of 0.5
#define SHOW_NUMBER_PLUS_ONE 31759 // Show number plus one

QVector<KTN_parameter_struct> KTN_parameters = {
    {0x0030, 2, "BOOST", 1 | SUBLIST_FROM_BYTE2, FX_DIST_TYPE, KTN_CAT_BOOST}, //00
    {0x0031, 22, "BST TP", 1, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0032, 121, "BST DRIVE", SHOW_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0033, 101, "BST BOTTOM", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0034, 101, "BST TONE", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0035, 2, "BST SOLO SW", 0, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0036, 101, "BST SOLO LVL", SHOW_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0037, 101, "BST FX LVL", SHOW_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0038, 101, "BST DIR LVL", SHOW_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0039, 8, "CUST.TP", 273, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x003A, 101, "CUST.BOTTOM", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST}, // 10
    {0x003B, 101, "CUST.TOP", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x003C, 101, "CUST.LOW", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x003D, 101, "CUST.HIGH", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x003E, 101, "CUST.CHAR", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST},
    {0x0140, 2, "MOD", 23 | SUBLIST_FROM_BYTE2, KTN_FX_COLOUR, KTN_CAT_MOD},
    {0x0141, 37, "MOD TP", 23, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8000, 101, "MOD PAR 01", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8001, 101, "MOD PAR 02", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8002, 101, "MOD PAR 03", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8003, 101, "MOD PAR 04", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD}, // 20
    {0x8004, 101, "MOD PAR 05", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8005, 101, "MOD PAR 06", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8006, 101, "MOD PAR 07", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8007, 101, "MOD PAR 08", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8008, 101, "MOD PAR 09", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x8009, 101, "MOD PAR 10", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x800A, 101, "MOD PAR 11", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x800B, 101, "MOD PAR 12", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x800C, 101, "MOD PAR 13", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x800D, 101, "MOD PAR 14", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD}, // 30
    {0x800E, 101, "MOD PAR 15", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD},
    {0x0051, 28, "AMP TP", 60, FX_AMP_TYPE, KTN_CAT_AMP},
    {0x0052, 121, "AMP GAIN", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP},
    {0x0054, 101, "AMP BASS", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP},
    {0x0055, 101, "AMP MIDDLE", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP},
    {0x0056, 101, "AMP TREBLE", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP},
    {0x0057, 101, "AMP PRESCENCE", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP},
    {0x0059, 2, "AMP BRIGHT", 0, FX_AMP_TYPE, KTN_CAT_AMP},
    {0x0058, 101, "AMP LEVEL", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP},
    {0x0130, 2, "EQ SW", 0, FX_FILTER_TYPE, KTN_CAT_EQ}, // 40
    {0x0131, 18, "EQ LOW CUT", 109, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0x0132, 41, "EQ LOW LVL", SHOW_CUT_BOOST, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0x0133, 26, "EQ L-M FRQ", 110, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0x0134, 6, "EQ L-M Q", 140, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0x0135, 41, "EQ L-M LVL", SHOW_CUT_BOOST, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0x0136, 26, "EQ H-M FRQ", 110, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0x0137, 6, "EQ H-M Q", 140, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0x0138, 41, "EQ H-M LVL", SHOW_CUT_BOOST, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0x0139, 15, "EQ HI CUT", 125, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0x013A, 41, "EQ HI LVL", SHOW_CUT_BOOST, FX_FILTER_TYPE, KTN_CAT_EQ}, // 50
    {0x013B, 101, "EQ LEVEL", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ},
    {0x0560, 2, "DELAY1", 88  | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x0561, 11, "DLY1 TP", 88, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x0562, TIME_2000, "DLY1 TIME", SHOW_DELAY_TIME, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x0564, 101, "DLY1 F.BACK", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x0565, 15, "DLY1 H.CUT", 125, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x0566, 101, "DLY1 FX LVL", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x0567, 101, "DLY1 DIR LVL", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x0573, 101, "DLY1 MOD RATE", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x0574, 101, "DLY1 MOD DPTH", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY1}, // 60
    {0x034C, 2, "FX SW", 23  | SUBLIST_FROM_BYTE2, KTN_FX_COLOUR, KTN_CAT_FX},
    {0x034D, 37, "FX TYPE", 23, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9000, 101, "FX PAR 01", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9001, 101, "FX PAR 02", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9002, 101, "FX PAR 03", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9003, 101, "FX PAR 04", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9004, 101, "FX PAR 05", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9005, 101, "FX PAR 06", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9006, 101, "FX PAR 07", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9007, 101, "FX PAR 08", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX}, // 70
    {0x9008, 101, "FX PAR 09", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x9009, 101, "FX PAR 10", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x900A, 101, "FX PAR 11", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x900B, 101, "FX PAR 12", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x900C, 101, "FX PAR 13", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x900D, 101, "FX PAR 14", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x900E, 101, "FX PAR 15", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX},
    {0x104E, 2, "DELAY2", 88  | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE, KTN_CAT_DLY2},
    {0x104F, 11, "DLY2 TP", 88, FX_DELAY_TYPE, KTN_CAT_DLY2},
    {0x1050, TIME_2000, "DLY2 TIME", SHOW_DELAY_TIME, FX_DELAY_TYPE, KTN_CAT_DLY2}, // 80
    {0x1052, 101, "DLY2 F.BACK", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY2},
    {0x1053, 15, "DLY2 H.CUT", 125, FX_DELAY_TYPE, KTN_CAT_DLY2},
    {0x1054, 101, "DLY2 FX LVL", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY2},
    {0x1055, 101, "DLY2 DIR LVL", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY2},
    {0x1061, 101, "DLY1 MOD RATE", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x1062, 101, "DLY1 MOD DPTH", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY1},
    {0x0610, 2, "RVB SW", 99  | SUBLIST_FROM_BYTE2, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x0611, 7, "RVB TP", 99, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x0612, 100, "RVB TIME", SHOW_RVB_TIME, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x0613, TIME_500, "RVB PRE", SHOW_DELAY_TIME, FX_REVERB_TYPE, KTN_CAT_RVB}, // 90
    {0x0615, 18, "RVB L.CUT", 109, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x0616, 15, "RVB H.CUT", 125, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x0617, 11, "RVB DENS", SHOW_NUMBER, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x0618, 101, "FX LVL", SHOW_NUMBER, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x0619, 101, "DIR LVL", SHOW_NUMBER, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x061A, 101, "SPRING SENS", SHOW_NUMBER, FX_REVERB_TYPE, KTN_CAT_RVB},
    {0x0655, 2, "SEND/RETURN", 0, FX_FILTER_TYPE, KTN_CAT_MISC},
    {0x0656, 2, "S/R MODE", 146, FX_FILTER_TYPE, KTN_CAT_MISC},
    {0x0657, 101, "S/R SEND LVL", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_MISC},
    {0x0658, 101, "S/R RET. LVL", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_MISC}, // 100
    {0x0663, 2, "NOISE SUPRESSOR", 0, FX_FILTER_TYPE, KTN_CAT_MISC},
    {0x0664, 101, "N/S THRESH", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_MISC},
    {0x0665, 101, "N/S RLEASE", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_MISC},
    {0x0633, 101, "FOOT VOL", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_MISC},
    {0x1210, 3, "BOOST COLOR", 106, KTN_FX_BUTTON_COLOUR, KTN_CAT_MISC},
    {0x1211, 3, "MOD COLOR", 106, KTN_FX_BUTTON_COLOUR, KTN_CAT_MISC},
    {0x1212, 3, "DLY1 COLOR", 106, KTN_FX_BUTTON_COLOUR, KTN_CAT_MISC},
    {0x1213, 3, "FX COLOR", 106, KTN_FX_BUTTON_COLOUR, KTN_CAT_MISC},
    {0x1214, 3, "RVB COLOR", 106, KTN_FX_BUTTON_COLOUR, KTN_CAT_MISC},
    {0x7430, 3, "L/OUT AIR", 265, FX_AMP_TYPE, KTN_CAT_GLOBAL}, // 110 // Address 0x7xxx is translated to 0x00000xxx
    {0x7431, 3, "CAB RESO", 268, FX_AMP_TYPE, KTN_CAT_GLOBAL},
    {0x7432, 2, "GLOBAL EQ SW", 0, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x743E, 2, "GL EQ POS", 271, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x7433, 18, "GEQ LOW CUT", 109, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x7434, 41, "GEQ LOW LVL", SHOW_CUT_BOOST, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x7435, 26, "GEQ L-M F", 110, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x7436, 6, "GEQ L-M Q", 140, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x7437, 41, "GEQ L-M LVL", SHOW_CUT_BOOST, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x7438, 26, "GEQ H-M F", 110, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x7439, 6, "GEQ H-M Q", 140, FX_FILTER_TYPE, KTN_CAT_GLOBAL}, // 120
    {0x743A, 41, "GEQ H-M LVL", SHOW_CUT_BOOST, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x743B, 15, "GEQ HI CUT", 125, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x743C, 41, "GEQ HI LVL", SHOW_CUT_BOOST, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
    {0x743D, 101, "GEQ LEVEL", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_GLOBAL},
};

const uint16_t KTN_NUMBER_OF_PARAMETERS = KTN_parameters.size();

QStringList KTN_sublists = {
    // Sublist 1 - 22: Booster types
    "MILD B", "CLEAN B", "TREBLE B", "CRUNCH", "NAT OD", "WARM OD", "FAT DS", "LEAD DS", "METAL DS", "OCT.FUZZ",
    "BLUES OD", "OD-1", "TUBESCRM", "TURBO OD", "DIST", "RAT", "GUVNR DS", "DST+", "METAL ZN", "60s FUZZ",
    "MUFF FZ", "CUSTOM",

    // Sublist 23 - 59: MOD/FX types
    "TOUCHWAH", "AUTO WAH", "SUB WAH", "COMPRSOR", "LIMITER", "DIST", "GRAPH EQ", "PARAM EQ", "TONE MOD", "GTR SIM",
    "SLOW GR", "DEFRET", "WAV SNTH", "SITARSIM", "OCTAVE", "PITCH SH", "HARMONST", "SND HOLD", "AC. PROC", "PHASER",
    "FLANGER", "TREMOLO", "ROTARY 1", "UNI-V", "PAN", "SLICER", "VIBRATO", "RING MOD", "HUMANIZR", "2x2 CHOR",
    "SUB DLY", "AcGtrSim", "ROTARY 2", "TeraEcho", "OVERTONE", "PHAS 90E", "FLGR117E", // Tera Echo and Overtone do not work for the MOD FX

    // Sublist 60 - 87 : Amp types
    "NAT CLN", "ACOUSTIC", "COMBO C", "STACK C", "HiGAIN", "POWER DR", "EXTREME", "CORE MTL", "CLEAN!", "CLN TWIN",
    "PRO CR", "CRUNCH!", "DELUXE 2", "VO DRIVE", "VO LEAD", "MATCH DR", "BG LEAD", "BG DRIVE", "MS1959I", "MS1959II",
    "RFIER V", "RFIER M", "T-AMP L", "BROWN!", "LEAD!", "CUSTOM", "BGNR UBR", "ORNGE RR",

    // Sublist 88 - 98 : Delay types
    "DIGITAL", "PAN", "STEREO", "DUAL SER", "DUAL PAR", "DUAL L/R", "REVERSE", "ANALOG", "TAPE ECO", "MODULATE",
    "SDE-3000",

    // Sublist 99 - 105 : Reverb types
    "AMBIENCE", "ROOM", "HALL 1", "HALL 2", "PLATE", "SPRING", "MODULATE",

    // Sublist 106 - 108 : FX color types
    "GREEN", "RED", "YELLOW",

    // Sublist 109 - 139: Frequencies
    // Low: start at 109 -
    // Mid - start at 110
    // High - start at 125
    "FLAT", "20.0Hz", "25.0Hz", "31.5Hz", "40.0Hz", "50.0Hz", "63.0Hz", "80.0Hz", "100Hz", "125Hz",
    "160Hz", "200Hz", "250Hz", "315Hz", "400Hz", "500Hz", "630Hz", "800Hz", "1.00kHz", "1.25kHz",
    "1.60kHz", "2.0kHz", "2.50kHz", "3.15kHz", "4.00kHz", "5.00kHz", "6.30kHz", "8.00kHz", "10.0kHz", "12.5kHz",
    "FLAT",

    // Sublist 140 - 145 - Mid Q
    "0,5", "1", "2", "4", "8", "16",

    // Sublist 146 - 147 - S/R mode
    "SERIES", "PARALLEL",

    // Sublist 148 - 149: Wah Mode
    "LPF", "BPF",

    // Sublist 150 - 151 T.Wah polar
    "DOWN", "UP",

    // Sublist 152 - 157 Wah types
    "CRY WAH", "VO WAH", "FAT WAH", "LITE WAH", "7STR.WAH", "RESO WAH",

    // Sublist 158 - 165 Compressor types
    "BOSS CMP", "HI-BAND", "LIGHT", "D-COMP", "ORANGE", "FAT", "MILD", "ST COMP",

    // Sublist 166 - 168: Limiter types
    "BOSS LIM",  "RACK160D", "VTGRACKU",

    // Sublist 169 - 176: Tone MOD types
    "FAT", "PRESENCE", "MILD", "TIGHT", "ENHANCE", "RESO1", "RESO2", "RESO3",

    // Sublist 177 - 184: Guitar sim types
    "S->H", "H->S", "H->HF", "S->HLW", "H->HLW", "S->AC", "H->AC", "P->AC",

    // Sublist 185 - 186: Wave synth WAVE types
    "SAW", "SQUARE",

    // Sublist 187 - 190: Octave ranges
    "1: B1-E6", "2:B1-E5", "3:B1-E4", "4:B1-E3",

    // Sublist 191 - 192: Pitch shift voices
    "1-VOICE", "2-MONO",

    // Sublist 193 - 196: Pitch shift modes
    "FAST", "MEDIUM", "SLOW", "MONO",

    // Sublist 197 - 226
    "-2oct", "-14th", "-13th", "-12th", "-11th", "-10th", "-9th",
    "-1oct", "-7th", "-6th", "-5th", "-4th", "-3rd", "-2nd", "TONIC",
    "+2nd", "+3rd", "+4th", "+5th", "+6th", "+7th", "+1oct", "+9th", "+10th", "+11th",
    "+12th", "+13th", "+14th", "+2oct", "USER",

    // Sublist 227 - 230: Ac processor types
    "SMALL", "MEDIUM", "BRIGHT", "POWER",

    // Sublist 231 - 234: Phaser types
    "4STAGE", "8STAGE", "12STAGE", "BiPHASE",

    // Sublist 235 - 236: Rotary speed
    "SLOW", "FAST",

    // Sublist 237 - 238 PAN types
    "AUTO", "MANUAL",

    // Sublist 239 -240 RING MOD modes
    "NORMAL", "INTLGNT",

    // Sublist 241 -242 HUMANIZER modes
    "PICKING", "AUTO",

    // Sublist 243 -247 HUMANIZER vowels
    "a", "e", "i", "o", "u",

    // 248 - 249 SUB DELAY types
    "MONO", "PAN",

    // 250 - 252 TERA ECHO MODES
    "MONO", "ST1", "ST2",

    // Sublist 253 - 264 for key
    "C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B",

    // Sublist 265 - 267 for Line Out Live Feel
    "REC", "LIVE", "BLEND",

    // Sublist 268 - 270 for Cabinet Resonance
    "VINTAGE", "MODERN", "DEEP",

    // Sublist 271 - 272 for Global eq position
    "INPUT", "OUTPUT",
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
            case SHOW_CUT_BOOST:
              if (value > 20)  Output += "+";
              Output += QString::number(value - 20);
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
