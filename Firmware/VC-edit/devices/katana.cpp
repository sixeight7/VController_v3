#include "katana.h"
#include "VController/config.h"
#include "VController/leds.h"
#include "VController/globals.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QInputDialog>
#include <QObject>

void KTN_class::init()
{
    device_name = "KATANA";
    full_device_name = "Boss KATANA";
    patch_min = KTN_PATCH_MIN;
    patch_max = KTN_PATCH_MAX;
    patch_min_as_stored_on_VC = 9;
    patch_max_as_stored_on_VC = KTN_PATCH_MAX;
    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 2; // Default value: red
    MIDI_channel = KTN_MIDI_CHANNEL; // Default value
    bank_number = 0; // Default value
    is_always_on = true; // Default value
    my_device_page1 = KTN_DEFAULT_PAGE1; // Default value
    my_device_page2 = KTN_DEFAULT_PAGE2; // Default value
    my_device_page3 = KTN_DEFAULT_PAGE3; // Default value
    my_device_page4 = KTN_DEFAULT_PAGE4; // Default value

    //InitializePatchArea();
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
  uint16_t Address_MK1; // The address of the parameter
  uint16_t Address_MK2; // The address of the parameter
  uint8_t NumVals; // The number of values for this parameter
  QString Name; // The name for the label
  uint16_t Sublist; // Which sublist to read for the FX or amp type - 0 if second byte does not contain the type or if there is no sublist +100 Show value from sublist.
  uint8_t Colour; // The colour for this effect.
  uint8_t Category; // The category of this effect
  uint8_t Supported_in_version; // From which version number this parameter is supported
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
    {0x0620, 0x0550, 2, "PEDAL SW", 290 | SUBLIST_FROM_BYTE2, FX_FILTER_TYPE, KTN_CAT_PEDAL, 4}, //00
    {0x1111, 0x0551, 3, "PEDAL TYPE", 290, FX_FILTER_TYPE, KTN_CAT_PEDAL, 4 },
    {0xB000, 0xB000, 101, "PDL PAR 01", SHOW_NUMBER, KTN_PEDAL_TYPE_COLOUR, KTN_CAT_PEDAL, 4 },
    {0xB001, 0xB001, 101, "PDL PAR 02", SHOW_NUMBER, KTN_PEDAL_TYPE_COLOUR, KTN_CAT_PEDAL, 4 },
    {0xB002, 0xB002, 101, "PDL PAR 03", SHOW_NUMBER, KTN_PEDAL_TYPE_COLOUR, KTN_CAT_PEDAL, 4 },
    {0xB003, 0xB003, 101, "PDL PAR 04", SHOW_NUMBER, KTN_PEDAL_TYPE_COLOUR, KTN_CAT_PEDAL, 4 },
    {0xB004, 0xB004, 101, "PDL PAR 05", SHOW_NUMBER, KTN_PEDAL_TYPE_COLOUR, KTN_CAT_PEDAL, 4 },
    {0xB005, 0xB005, 101, "PDL PAR 06", SHOW_NUMBER, KTN_PEDAL_TYPE_COLOUR, KTN_CAT_PEDAL, 4 },
    {0x0030, 0x0010, 2, "BOOST", 1 | SUBLIST_FROM_BYTE2, FX_DIST_TYPE, KTN_CAT_BOOST, 1 },
    {0x0031, 0x0011, 22, "BST TP", 1, FX_DIST_TYPE, KTN_CAT_BOOST, 1 },
    {0x0032, 0x0012, 121, "BST DRIVE", SHOW_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST, 1 },  //10
    {0x0033, 0x0013, 101, "BST BOTTOM", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST, 1 },
    {0x0034, 0x0014, 101, "BST TONE", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST, 1 },
    {0x0035, 0x0015, 2, "BST SOLO SW", 0, FX_DIST_TYPE, KTN_CAT_BOOST, 1 },
    {0x0036, 0x0016, 101, "BST SOLO LVL", SHOW_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST, 1 },
    {0x0037, 0x0017, 101, "BST FX LVL", SHOW_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST, 1 },
    {0x0038, 0x0018, 101, "BST DIR LVL", SHOW_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST, 1 },
    {0x0039, 0x0019, 8,   "BST CUST.TP", 276, FX_DIST_TYPE, KTN_CAT_BOOST, 1 },
    {0x003A, 0x001A, 101, "BST CUST.BTM", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST, 1 },
    {0x003B, 0x001B, 101, "BST CUST.TOP", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST, 1 },
    {0x003C, 0x001C, 101, "BST CUST.LOW", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST, 1 }, //20
    {0x003D, 0x001D, 101, "BST CUST.HI", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST, 1 },
    {0x003E, 0x001E, 101, "BST CUST.CHAR", SHOW_TONE_NUMBER, FX_DIST_TYPE, KTN_CAT_BOOST, 1 },
    {0x0140, 0x0100, 2, "MOD", 23 | SUBLIST_FROM_BYTE2, KTN_FX_COLOUR, KTN_CAT_MOD, 1 },
    {0x0141, 0x0101, 40, "MOD TP", 23, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD, 1 },
    {0x8000, 0x8000, 101, "MOD PAR 01", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD, 1 },
    {0x8001, 0x8001, 101, "MOD PAR 02", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD, 1 },
    {0x8002, 0x8002, 101, "MOD PAR 03", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD, 1 },
    {0x8003, 0x8003, 101, "MOD PAR 04", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD, 1 },
    {0x8004, 0x8004, 101, "MOD PAR 05", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD, 1 },
    {0x8005, 0x8005, 101, "MOD PAR 06", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD, 1 }, // 30
    {0x8006, 0x8006, 101, "MOD PAR 07", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD, 1 },
    {0x8007, 0x8007, 101, "MOD PAR 08", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD, 1 },
    {0x8008, 0x8008, 101, "MOD PAR 09", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD, 1 },
    {0x8009, 0x8009, 101, "MOD PAR 10", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD, 1 },
    {0x800A, 0x800A, 101, "MOD PAR 11", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD, 1 },
    {0x800B, 0x800B, 101, "MOD PAR 12", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD, 1 },
    {0x800C, 0x800C, 101, "MOD PAR 13", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD, 1 },
    {0x800D, 0x800D, 101, "MOD PAR 14", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD, 1 },
    {0x800E, 0x800E, 101, "MOD PAR 15", SHOW_NUMBER, KTN_MOD_TYPE_COLOUR, KTN_CAT_MOD, 1 },
    {0x0051, 0x0021, 28, "AMP TP", 63, FX_AMP_TYPE, KTN_CAT_AMP, 1 },              // 40
    {0x0052, 0x0022, 121, "AMP GAIN", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP, 1 },
    {0x0054, 0x0024, 101, "AMP BASS", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP, 1 },
    {0x0055, 0x0025, 101, "AMP MIDDLE", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP, 1 },
    {0x0056, 0x0026, 101, "AMP TREBLE", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP, 1 },
    {0x0057, 0x0027, 101, "AMP PRESCENCE", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP, 1 },
    {0x0059, 0x0029, 2, "AMP BRIGHT", 0, FX_AMP_TYPE, KTN_CAT_AMP, 1 },
    {0x0058, 0x0028, 101, "AMP LEVEL", SHOW_NUMBER, FX_AMP_TYPE, KTN_CAT_AMP, 1 },
    {0x0130, 0x0040, 2, "EQ SW", 0, FX_FILTER_TYPE, KTN_CAT_EQ, 2 },
    {0x1104, 0x0041, 2, "EQ TYPE", 288, FX_FILTER_TYPE, KTN_CAT_EQ, 3 },
    {0xA000, 0xA000, 101, "L.Cut  /  31 Hz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ, 2 }, //50
    {0xA001, 0xA001, 101, "L.Lvl  /  62 Hz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ, 2 },
    {0xA002, 0xA002, 101, "LM.Freq/ 125 Hz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ, 2 },
    {0xA003, 0xA003, 101, "LM Q   / 250 Hz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ, 2 },
    {0xA004, 0xA004, 101, "LM LVL / 500 Hz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ, 2 },
    {0xA005, 0xA005, 101, "HM Freq/  1 kHz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ, 2 },
    {0xA006, 0xA006, 101, "HM Q   /  2 kHz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ, 2 },
    {0xA007, 0xA007, 101, "HM Lvl /  4 kHz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ, 2 },
    {0xA008, 0xA008, 101, "H Lvl  /  8 kHz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ, 2 },
    {0xA009, 0xA009, 101, "H.Cut  / 16 kHz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ, 2 },
    {0xA00A, 0xA00A, 101, "EQ LEVEL", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_EQ, 2 }, //60
    {0x0560, 0x0500, 2, "DLY1", 91  | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE, KTN_CAT_DLY1, 1 },
    {0x0561, 0x0501, 11, "DLY1 TP", 91, FX_DELAY_TYPE, KTN_CAT_DLY1, 1 },
    {0x0562, 0x0502, TIME_2000, "DLY1 TIME", SHOW_DELAY_TIME, FX_DELAY_TYPE, KTN_CAT_DLY1, 1 },
    {0x0564, 0x0504, 101, "DLY1 F.BACK", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY1, 1 },
    {0x0565, 0x0505, 15, "DLY1 H.CUT", 128, FX_DELAY_TYPE, KTN_CAT_DLY1, 1 },
    {0x0566, 0x0506, 101, "DLY1 FX LVL", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY1, 1 },
    {0x0567, 0x0507, 101, "DLY1 DIR LVL", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY1, 1 },
    {0x0573, 0x0513, 101, "DLY1 MOD RATE", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY1, 1 },
    {0x0574, 0x0514, 101, "DLY1 MOD DPTH", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY1, 1 },
    {0x034C, 0x0300, 2, "FX", 23  | SUBLIST_FROM_BYTE2, KTN_FX_COLOUR, KTN_CAT_FX, 1 }, // 70
    {0x034D, 0x0301, 40, "FX TYPE", 23, KTN_FX_TYPE_COLOUR, KTN_CAT_FX, 1 },
    {0x9000, 0x9000, 101, "FX PAR 01", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX, 1 },
    {0x9001, 0x9001, 101, "FX PAR 02", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX, 1 },
    {0x9002, 0x9002, 101, "FX PAR 03", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX, 1 },
    {0x9003, 0x9003, 101, "FX PAR 04", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX, 1 },
    {0x9004, 0x9004, 101, "FX PAR 05", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX, 1 },
    {0x9005, 0x9005, 101, "FX PAR 06", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX, 1 },
    {0x9006, 0x9006, 101, "FX PAR 07", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX, 1 },
    {0x9007, 0x9007, 101, "FX PAR 08", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX, 1 },
    {0x9008, 0x9008, 101, "FX PAR 09", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX, 1 }, //80
    {0x9009, 0x9009, 101, "FX PAR 10", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX, 1 },
    {0x900A, 0x900A, 101, "FX PAR 11", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX, 1 },
    {0x900B, 0x900B, 101, "FX PAR 12", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX, 1 },
    {0x900C, 0x900C, 101, "FX PAR 13", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX, 1 },
    {0x900D, 0x900D, 101, "FX PAR 14", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX, 1 },
    {0x900E, 0x900E, 101, "FX PAR 15", SHOW_NUMBER, KTN_FX_TYPE_COLOUR, KTN_CAT_FX, 1 },
    {0x104E, 0x0520, 2, "DLY2", 91  | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE, KTN_CAT_DLY2, 2 },
    {0x104F, 0x0521, 11, "DLY2 TP", 91, FX_DELAY_TYPE, KTN_CAT_DLY2, 2 },
    {0x1050, 0x0522, TIME_2000, "DLY2 TIME", SHOW_DELAY_TIME, FX_DELAY_TYPE, KTN_CAT_DLY2, 2 },
    {0x1052, 0x0524, 101, "DLY2 F.BACK", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY2, 2 }, // 90
    {0x1053, 0x0525, 15, "DLY2 H.CUT", 128, FX_DELAY_TYPE, KTN_CAT_DLY2, 2 },
    {0x1054, 0x0526, 101, "DLY2 FX LVL", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY2, 2 },
    {0x1055, 0x0527, 101, "DLY2 DIR LVL", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY2, 2 },
    {0x1061, 0x0533, 101, "DLY2 MOD RATE", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY2, 2 },
    {0x1062, 0x0534, 101, "DLY2 MOD DPTH", SHOW_NUMBER, FX_DELAY_TYPE, KTN_CAT_DLY2, 2 },
    {0x0610, 0x0540, 2, "RVB SW", 102  | SUBLIST_FROM_BYTE2, FX_REVERB_TYPE, KTN_CAT_RVB, 1 },
    {0x0611, 0x0541, 7, "RVB TP", 102, FX_REVERB_TYPE, KTN_CAT_RVB, 1 },
    {0x0612, 0x0542, 100, "RVB TIME", SHOW_RVB_TIME, FX_REVERB_TYPE, KTN_CAT_RVB, 1 },
    {0x0613, 0x0543, TIME_500, "RVB PRE", SHOW_DELAY_TIME, FX_REVERB_TYPE, KTN_CAT_RVB, 1 },
    {0x0615, 0x0545, 18, "RVB L.CUT", 112, FX_REVERB_TYPE, KTN_CAT_RVB, 1 }, // 100
    {0x0616, 0x0546, 15, "RVB H.CUT", 128, FX_REVERB_TYPE, KTN_CAT_RVB, 1 },
    {0x0617, 0x0547, 11, "RVB DENS", SHOW_NUMBER, FX_REVERB_TYPE, KTN_CAT_RVB, 1 },
    {0x0618, 0x0548, 101, "FX LVL", SHOW_NUMBER, FX_REVERB_TYPE, KTN_CAT_RVB, 1 },
    {0x0619, 0x0549, 101, "DIR LVL", SHOW_NUMBER, FX_REVERB_TYPE, KTN_CAT_RVB, 1 },
    {0x061B, 0x054B, 101, "SPRING COLOR", SHOW_NUMBER, FX_REVERB_TYPE, KTN_CAT_RVB, 1 },
    {0x0655, 0x0562, 2, "S/R LOOP", 0, FX_FILTER_TYPE, KTN_CAT_MISC, 1 },
    {0x0656, 0x0563, 2, "S/R MODE", 149, FX_FILTER_TYPE, KTN_CAT_MISC, 1 },
    {0x0657, 0x0564, 101, "S/R SEND LVL", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_MISC, 1 },
    {0x0658, 0x0565, 101, "S/R RET. LVL", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_MISC, 1 },
    {0x0663, 0x0566, 2, "N/S SW", 0, FX_DYNAMICS_TYPE, KTN_CAT_MISC, 1 }, // 110
    {0x0664, 0x0567, 101, "N/S THRESH", SHOW_NUMBER, FX_DYNAMICS_TYPE, KTN_CAT_MISC, 1 },
    {0x0665, 0x0568, 101, "N/S RLEASE", SHOW_NUMBER, FX_DYNAMICS_TYPE, KTN_CAT_MISC, 1 },
    {0x121F, 0x063E, 9, "EXP ASGN", 293, FX_FILTER_TYPE, KTN_CAT_MISC, 3 },
    {0x1220, 0x063F, 9, "FC E1 ASGN", 293, FX_FILTER_TYPE, KTN_CAT_MISC, 3 },
    {0x1221, 0x0640, 9, "FC E2 ASGN", 293, FX_FILTER_TYPE, KTN_CAT_MISC, 3 },
    {0x0633, 0x0561, 101, "FOOT VOL", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_MISC, 1 },
    {0x1210, 0x0639, 3, "BOOST COLOR", 109, KTN_FX_BUTTON_COLOUR, KTN_CAT_MISC, 1 },
    {0x1211, 0x063A, 3, "MOD COLOR", 109, KTN_FX_BUTTON_COLOUR, KTN_CAT_MISC, 1 },
    {0x1213, 0x063B, 3, "FX COLOR", 109, KTN_FX_BUTTON_COLOUR, KTN_CAT_MISC, 1 },  // 120
    {0x1212, 0x063C, 3, "DLY1 COLOR", 109, KTN_FX_BUTTON_COLOUR, KTN_CAT_MISC, 1 },
    {0x1214, 0x063D, 3, "RVB COLOR", 109, KTN_FX_BUTTON_COLOUR, KTN_CAT_MISC, 1 },
    {0x7430, 0x7029, 3, "L/OUT AIR", 268, FX_AMP_TYPE, KTN_CAT_GLOBAL, 2 }, // Address 0x7xxx is translated to 0x00000xxx
    {0x7431, 0x0643, 3, "CAB RESO", 271, FX_AMP_TYPE, KTN_CAT_GLOBAL, 2 },
    {0x7432, 0x7010, 2, "GLBL EQ SW", 0, FX_FILTER_TYPE, KTN_CAT_GLOBAL, 2 },
    {0x743E, 0x7012, 2, "GL EQ POS", 274, FX_FILTER_TYPE, KTN_CAT_GLOBAL, 2 },
    {0x743F, 0x7011, 2, "GL EQ TYPE", 288, FX_FILTER_TYPE, KTN_CAT_GLOBAL, 3 },
    {0xC000, 0xC000, 101, "GEQ L.Cut/31 Hz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_GLOBAL, 2 },
    {0xC001, 0xC001, 101, "GEQ L.Lvl/62 Hz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_GLOBAL, 2 },
    {0xC002, 0xC002, 101, "GEQ LM.F/125 Hz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_GLOBAL, 2 },
    {0xC003, 0xC003, 101, "GEQ LM Q/250 Hz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_GLOBAL, 2 }, // 130
    {0xC004, 0xC004, 101, "GEQ LM L/500 Hz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_GLOBAL, 2 },
    {0xC005, 0xC005, 101, "GEQ HM F/ 1 kHz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_GLOBAL, 2 },
    {0xC006, 0xC006, 101, "GEQ HM Q/ 2 kHz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_GLOBAL, 2 },
    {0xC007, 0xC007, 101, "GEQ HM L/ 4 kHz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_GLOBAL, 2 },
    {0xC008, 0xC008, 101, "GEQ H Lvl/8 kHz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_GLOBAL, 2 },
    {0xC009, 0xC009, 101, "GEQ HCut/16 kHz", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_GLOBAL, 2 },
    {0xC00A, 0xC00A, 101, "GLOBAL EQ LVL", SHOW_NUMBER, FX_FILTER_TYPE, KTN_CAT_GLOBAL, 2 },
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
#define KTN_PATCH_HEADER         0  // Patch header (7 bytes)
#define KTN_PATCH_NAME_INDEX     7  // Patch name (16 parameters)
#define KTN_BOOST_INDEX         23  // Boost FX (15 parameters)
#define KTN_AMP_INDEX           38  // Amp index (9 parameters)
#define KTN_EQ_SW_INDEX         47  // Eq (1 parameters)
#define KTN_EQ_GEQ_INDEX        48  // Eq or GEQ (11 parameters)
#define KTN_MOD_BASE_INDEX      59  // Mod base (2 parameters: ON/OFF and type)
#define KTN_MOD_INDEX           61  // MOD effect settings for active effect (max 15 parameters)
#define KTN_FX_BASE_INDEX       76  // FX base (2 parameters: ON/OFF and type)
#define KTN_FX_INDEX            78  // FX effect settings for active effect (max 15 parameters)
#define KTN_DELAY1_INDEX        93  // Delay 1 (9 parameters)
#define KTN_DELAY1_MOD_INDEX   102  // Delay 1 MOD (2 parameters)
#define KTN_DELAY1_SDE_INDEX   104  // Delay 1 SDE-3000 settings (5)
#define KTN_DELAY2_INDEX       109  // Delay 2 (9 parameters)
#define KTN_DELAY2_MOD_INDEX   118  // Delay 2 MOD (2 parameters)
#define KTN_SPARE_BYTES_NO1    120  // 7 bytes here
#define KTN_DELAY2_SDE_INDEX   127  // Delay 2 SDE-3000 settings (5)
#define KTN_REVERB_INDEX       132  // Reverb (12 parameters)
#define KTN_FOOT_VOL_INDEX     146  // Foot volume (1 parameter)
#define KTN_S_R_LOOP_INDEX     147  // S/R loop (4 parameters)
#define KTN_NOISE_GATE_INDEX   151  // Noise gate (4 parameters)
#define KTN_MASTER_KEY_INDEX   155  // Master key
#define KTN_FX_CHAIN_INDEX     156  // FX chain (20 parameters)
#define KTN_EQ_TYPE_INDEX      176  // GEQ (1 parameter)
#define KTN_PEDAL_SW_INDEX     177  // Pedal sw  (1 parameters: ON/OFF)
#define KTN_PEDAL_TYPE_INDEX   178  // Pedal type  (1 parameter)
#define KTN_PEDAL_INDEX        179  // Wah type (6 parameters)
#define KTN_EXP_ASSIGNS_INDEX  185  // Assignments for the expression pedals, FS_functions and cabinet resonance (6 bytes)
#define KTN_SPARE_BYTES_NO2    191  // One byte left

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
  { 0x1104, 1, KTN_EQ_TYPE_INDEX, "EQ_TYPE"     },  // eq type
  { 0x0140, 2 + 15, KTN_MOD_BASE_INDEX, "MOD"   },  // Mod base
  { 0x034C, 2 + 15, KTN_FX_BASE_INDEX, "FX"     },  // FX base
  { 0x0655, 4, KTN_S_R_LOOP_INDEX, "S_R_LOOP"   },  // S/R loop
  { 0x0560, 11, KTN_DELAY1_INDEX, "DELAY1"      },  // Delay 1
  { 0x0610, 12, KTN_REVERB_INDEX, "REVERB"      },  // Reverb
  { 0x104E, 11, KTN_DELAY2_INDEX, "DELAY2"      },  // Delay 2
  { 0x0630, 1, KTN_FOOT_VOL_INDEX, "FOOTVOL"    },  // Foot volume
  { 0x0663, 4, KTN_NOISE_GATE_INDEX, "N_GATE"   },  // Noise gate
  { 0x0718, 1, KTN_MASTER_KEY_INDEX, "M_KEY"    },  // Master key
  { 0x1049, 5, KTN_DELAY1_SDE_INDEX, "DLY1_SDE" },  // Delay 1 - SDE3000 settings
  { 0x1049, 5, KTN_DELAY2_SDE_INDEX, "DLY2_SDE" },  // Delay 2 - SDE3000 settings
  { 0x0620, 1, KTN_PEDAL_SW_INDEX, "PEDAL SW"   },  // Pedal SW
  { 0x1111, 1 + 6, KTN_PEDAL_TYPE_INDEX, "PEDAL TP" },  // Pedal type
  { 0x0626, 6, KTN_EXP_ASSIGNS_INDEX, "EXP ASGN"},  // Assigns for expression pedals
  { 0x0000, 16, KTN_PATCH_NAME_INDEX, "P_NAME"  },  // Patch name (1)
};

const uint8_t KTN_NUMBER_OF_PATCH_MESSAGES = KTN_patch_memory.size();

QString KTN_class::get_patch_info(uint16_t number)
{
    uint8_t patch_no = (Device_patches[number][1] << 7) + Device_patches[number][2];
    QString line = number_format(patch_no + 9);
    line.append(" \t");
    for (int c = 0; c < 16; c++) {
        line.append(static_cast<char>(Device_patches[number][c + KTN_PATCH_NAME_INDEX]));
    }
    line.append("   \t( Amp: ");
    int amp_type = Device_patches[number][KTN_AMP_INDEX];
    line.append(KTN_sublists[59 + amp_type]);
    line.append(",  Mod:");
    int mod_type = Device_patches[number][KTN_MOD_BASE_INDEX + 1];
    line.append(KTN_sublists[22 + mod_type]);
    line.append(",  FX:");
    int fx_type = Device_patches[number][KTN_FX_BASE_INDEX + 1];
    line.append(KTN_sublists[22 + fx_type]);
    line.append(" )");

    return line;
}

void KTN_class::WritePatchName(int number, QString name)
{
    int len = name.length();
    if (len > 16) len = 16;
    for (int c = 0; c < len; c++) {
        Device_patches[number][c + KTN_PATCH_NAME_INDEX] = name.at(c).toLatin1();
    }
    for (int c = len; c < 16; c++) {
        Device_patches[number][c + KTN_PATCH_NAME_INDEX] = 32; //space
    }
}

QString KTN_class::ReadPatchName(int number)
{
    QString line = "";
    for (int c = 0; c < 16; c++) {
        line.append(static_cast<char>(Device_patches[number][c + KTN_PATCH_NAME_INDEX]));
    }
    return line.trimmed();
}

uint8_t KTN_class::supportPatchSaving()
{
    return 1;
}

void KTN_class::readPatchData(int index, int patch_no, const QJsonObject &json)
{
    Device_patches[index][0] = my_device_number + 1;
    Device_patches[index][1] = patch_no >> 8;
    Device_patches[index][2] = patch_no & 0xFF;
    for (int m = 0; m < KTN_NUMBER_OF_PATCH_MESSAGES; m++) {
        QString blockName = KTN_patch_memory[m].Name;
        QJsonObject patchBlock = json[blockName].toObject();
        if (!patchBlock.isEmpty()) {
            for (int i = 0; i < KTN_patch_memory[m].Length; i++) {
                int value = patchBlock["Data"+ QString::number(i)].toInt();
                Device_patches[index][KTN_patch_memory[m].Index + i] = value;
            }
        }
    }
    // Backward comptibility for footvol which is stored in Data3 instead of Data1 in older Katana patch files
    // without this older patches will load correctly but foot volume is turned low.
    QJsonObject patchBlock = json["FOOTVOL"].toObject();
    if (patchBlock.contains("Data3")) {
        int value = patchBlock["Data3"].toInt();
        Device_patches[index][KTN_FOOT_VOL_INDEX] = value;
    }
}

void KTN_class::writePatchData(int patch_no, QJsonObject &json) const
{
    for (int m = 0; m < KTN_NUMBER_OF_PATCH_MESSAGES; m++) {
        QJsonObject patchBlock;
        for (int i = 0; i < KTN_patch_memory[m].Length; i++) {
            int value = Device_patches[patch_no][KTN_patch_memory[m].Index + i];
            patchBlock["Data"+ QString::number(i)] = value;
        }
        json[KTN_patch_memory[m].Name] = patchBlock;
    }
}

QString KTN_class::patchFileHeader()
{
    return "Katana"; // To ensure compatibility with older files
}

QString KTN_class::DefaultPatchFileName(int number)
{
    QString name = device_name;
    name.append('_');
    name.append(ReadPatchName(number));
    return name;
}
