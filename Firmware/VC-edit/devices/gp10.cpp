#include "gp10.h"
#include "VController/config.h"
#include "VController/leds.h"
#include "VController/globals.h"

void GP10_class::init()
{
    device_name = "GP10";
    full_device_name = "Boss GP-10";
    patch_min = GP10_PATCH_MIN;
    patch_max = GP10_PATCH_MAX;
    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 4; // Default value: orange
    MIDI_port_manual = MIDI_port_number(GP10_MIDI_PORT);
    MIDI_channel = GP10_MIDI_CHANNEL; // Default value
    bank_number = 0; // Default value
    is_always_on = true; // Default value
    if (VC_type == VCONTROLLER) {
      my_device_page1 = GP10_DEFAULT_VC_PAGE1; // Default value
      my_device_page2 = GP10_DEFAULT_VC_PAGE2; // Default value
      my_device_page3 = GP10_DEFAULT_VC_PAGE3; // Default value
      my_device_page4 = GP10_DEFAULT_VC_PAGE4; // Default value
    }
    if (VC_type == VCMINI) {
      my_device_page1 = GP10_DEFAULT_VCMINI_PAGE1; // Default value
      my_device_page2 = GP10_DEFAULT_VCMINI_PAGE2; // Default value
      my_device_page3 = GP10_DEFAULT_VCMINI_PAGE3; // Default value
      my_device_page4 = GP10_DEFAULT_VCMINI_PAGE4; // Default value
    }
    if (VC_type == VCTOUCH) {
      my_device_page1 = GP10_DEFAULT_VCTOUCH_PAGE1; // Default value
      my_device_page2 = GP10_DEFAULT_VCTOUCH_PAGE2; // Default value
      my_device_page3 = GP10_DEFAULT_VCTOUCH_PAGE3; // Default value
      my_device_page4 = GP10_DEFAULT_VCTOUCH_PAGE4; // Default value
    }
}

bool GP10_class::check_command_enabled(uint8_t cmd)
{
    switch (cmd) {
    case PATCH:
    case PARAMETER:
    case ASSIGN:
    case MUTE:
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

QString GP10_class::number_format(uint16_t patch_no)
{
   QString numberString = "P";
   numberString.append(QString::number(patch_no / 10));
   numberString.append(QString::number(patch_no % 10));
   return numberString;
}

struct GP10_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  uint16_t Target; // Target of the assign as given in the assignments of the GP10 / GR55
  uint32_t Address; // The address of the parameter
  uint8_t NumVals; // The number of values for this parameter
  QString Name; // The name for the label
  uint16_t Sublist; // Which sublist to read for the FX or amp type - 0 if second byte does not contain the type or if there is no sublist +100 Show value from sublist.
  uint8_t Colour; // The colour for this effect.
};

//typedef struct stomper Stomper;
#define GP10_FX_COLOUR 255 // Just a colour number to pick the colour from the GP10_FX_colours table
#define GP10_FX_TYPE_COLOUR 254 //Another number for the FX type
#define SUBLIST_FROM_BYTE2 32768 //Bit 16 set high means read sublist frm byte 2
#define SHOW_NUMBER 31767 // Special number for showing the number (used for gain/volume/etc) - set in sublist
#define SHOW_DOUBLE_NUMBER 31766 // Show number times 2 (used for patch volume/etc) - set in sublist
#define SHOW_PAN 31765 // Special number for showing the pan- set in sublist

QVector<GP10_parameter_struct> GP10_parameters = {
      {0x166, 0x20016000, 2, "WAH SW", 180 | SUBLIST_FROM_BYTE2, FX_WAH_TYPE}, // 00
      {0x0F1, 0x20005800, 2, "FX SW", 1 | SUBLIST_FROM_BYTE2, GP10_FX_COLOUR},
      {0x16D, 0x20016800, 2, "Chorus SW", 17 | SUBLIST_FROM_BYTE2, FX_MODULATE_TYPE},
      {0x176, 0x20017000, 2, "DLY SW", 20 | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE},
      {0x188, 0x20017800, 2, "RVB SW", 30 | SUBLIST_FROM_BYTE2, FX_REVERB_TYPE},
      {0x192, 0x20020000, 2, "EQ SW", 0, FX_FILTER_TYPE},
      {0x0F2, 0x20005801, 16, "FX Type", 1, GP10_FX_TYPE_COLOUR},
      {0x167, 0x20016001, 6, "WAH TP", 180, FX_WAH_TYPE},
      {0x168, 0x20016002, 101, "WAH POS", SHOW_NUMBER, FX_WAH_TYPE},
      {0x161, 0x20016801, 3, "CHS Type", 17, FX_MODULATE_TYPE},
      {0x177, 0x20017001, 10, "DLY TYPE", 20, FX_DELAY_TYPE}, // 10
      {0x189, 0x20017801, 7, "RVB TYPE", 30, FX_REVERB_TYPE},
      {0xFFF, 0x20000800, 101, "PATCH LVL", SHOW_DOUBLE_NUMBER, FX_DEFAULT_TYPE},
      {0x000, 0x20001000, 2, "COSM GUITAR", 0, FX_GTR_TYPE},
      {0x001, 0x20001001, 4, "COSM Type", 115, FX_GTR_TYPE},
      {0x0A5, 0x20000804, 2, "NORMAL PU", 0, FX_GTR_TYPE},
      {0x002, 0x20001800, 12, "E.GTR TP", 119, FX_GTR_TYPE},
      {0x003, 0x20001801, 101, "E.GTR LVL", SHOW_NUMBER, FX_GTR_TYPE},
      {0xFFF, 0x20002001, 101, "A.GTR LVL", SHOW_NUMBER, FX_GTR_TYPE},
      {0xFFF, 0x20002801, 101, "BASS LVL", SHOW_NUMBER, FX_GTR_TYPE},
      {0xFFF, 0x20003001, 101, "SYNTH LVL", SHOW_NUMBER, FX_GTR_TYPE}, // 20
      {0xFFF, 0x20003801, 101, "POLYFX LVL", SHOW_NUMBER, FX_GTR_TYPE},
      {0x0CF, 0x20001002, 2, "COSM NS SW", 0, FX_GTR_TYPE},
      {0x09F, 0x20004000, 2, "TUN SW", 143  | SUBLIST_FROM_BYTE2, FX_PITCH_TYPE},
      {0x0A0, 0x20004001, 37, "TUNING", 143, FX_PITCH_TYPE},
      {0x0AD, 0x2000400E, 2, "12 STRING SW", 0, FX_PITCH_TYPE},
      {0x0DE, 0x20005000, 2, "Amp SW", 37 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE},
      {0x0DF, 0x20005001, 30, "Amp Type", 37, FX_AMP_TYPE},
      {0x0E0, 0x20005002, 101, "Amp Gain", SHOW_NUMBER, FX_AMP_TYPE},
      {0x0E2, 0x20005004, 101, "Amp LVL", SHOW_NUMBER, FX_AMP_TYPE},
      {0x0E8, 0x2000500B, 2, "Amp solo", 0, FX_AMP_TYPE}, // 30
      {0x0EC, 0x2000500A, 3, "Amp Gain SW", 131, FX_AMP_TYPE},
      {0xFFF, 0x20005028, 2, "Bend SW", 131, FX_AMP_TYPE},
      {0xFFF, 0x2000500E, 9, "Amp SP TP", 134, FX_AMP_TYPE},
      {0xFFF, 0x20020803, 101, "Foot Vol", SHOW_NUMBER, FX_AMP_TYPE}, // 34
      {0xFFF, 0x20021800, 18, "CTL1 ASGN", 67, FX_DEFAULT_TYPE},
      {0xFFF, 0x20021802, 18, "CTL2 ASGN", 67, FX_DEFAULT_TYPE},
      {0xFFF, 0x20021804, 17, "CTL3 ASGN", 67, FX_DEFAULT_TYPE},
      {0xFFF, 0x20021806, 17, "CTL4 ASGN", 67, FX_DEFAULT_TYPE},
      {0xFFF, 0x20021808, 17, "GK S1 ASGN", 67, FX_DEFAULT_TYPE},
      {0xFFF, 0x2002180A, 17, "GK S2 ASGN", 67, FX_DEFAULT_TYPE}, // 40
      {0xFFF, 0x2002180C, 15, "EXP SW ASGN", 85, FX_DEFAULT_TYPE}, // 41
      {0xFFF, 0x2002180E, 15, "EXP ASGN", 100, FX_DEFAULT_TYPE}, // 42
      {0xFFF, 0x2002180F, 15, "EXP ON ASGN", 100, FX_DEFAULT_TYPE}, // 43
      {0xFFF, 0x20021810, 15, "EXP2 ASGN", 100, FX_DEFAULT_TYPE},
      {0xFFF, 0x20021811, 15, "GK VOL ASGN", 100, FX_DEFAULT_TYPE},
      {0xFFF, 0x10001000, 2, "Guitar2MIDI", 0, FX_DEFAULT_TYPE},  // 46. Can not be controlled from assignment, but can be from GP10_PARAMETER!!!
};

const uint16_t GP10_NUMBER_OF_PARAMETERS = GP10_parameters.size();

QStringList GP10_sublists = {
  // Sublist 1 - 16: FX types
  "OD/DS", "COMPRSR", "LIMITER", "EQ", "T.WAH", "P.SHIFT", "HARMO", "P. BEND", "PHASER", "FLANGER", "TREMOLO", "PAN", "ROTARY", "UNI-V", "CHORUS", "DELAY",

  // Sublist 17 - 19: Chorus types
  "MONO", "ST1", "ST2",

  // Sublist 20 - 29: Delay types
  "SINGLE", "PAN", "STEREO", "DUAL-S", "DUAL-P", "DU L/R", "REVRSE", "ANALOG", "TAPE", "MODLTE",

  // Sublist 30 - 36: Reverb types
  "AMBNCE", "ROOM", "HALL1", "HALL2", "PLATE", "SPRING", "MODLTE",

  // Sublist 37 - 66 : Amp types
  "NAT CLN", "FULL RG", "COMBO C", "STACK C", "HiGAIN", "POWER D", "EXTREME", "CORE MT", "JC-120", "CL TWIN",
  "PRO CR", "TWEED", "DELUXE", "VO DRVE", "VO LEAD", "MTCH DR", "BG LEAD", "BG DRVE", "MS1959I", "M1959II",
  "RFIER V", "RFIER M", "T-AMP L", "SLDN", "5150 DR", "BGNR UB", "ORNG RR", "BASS CL", "BASS CR", "BASS Hi",

  // Sublist 67 - 84: CTL1-4 + GK assign function:
  "OFF", "PUSEL UP", "PUSEL DN", "12-STRNG", "ALT TN", "STR BEND", "MODELING", "N PU SW", "AMP SOLO", "FX SW", "EQ SW", "WAH SW", "CHOR SW", "DLY SW", "RVRB", "OSC HOLD", "TAPTEMPO", "LED TGL",

  // Sublist 85 - 99: CTL1-4 + GK assign function:
  "OFF", "12-STRNG", "ALT TN", "STR BEND", "MODELING", "N PU SW", "AMP SOLO", "FX SW", "EQ SW", "WAH SW", "CHOR SW", "DLY SW", "RVRB", "OSC HOLD", "LED TGL",

  // Sublist 100 - 114: EXP assign function
  "OFF", "FOOT VOL", "PTCH LVL", "MLNG VOL", "N PU VOL", "MIXER", "STG BEND", "MODELING", "FX", "AMP", "WAH", "CHORUS", "DELAY", "REVERB", "EQ",

  // Sublist 115 - 118: COSM types
  "E.GTR", "ACOUSTIC", "E.BASS", "SYNTH",

  // Sublist 119 - 130: E.GTR types
  "CLA ST", "MOD ST", "TE", "LP", "P90", "335", "L4", "RICK", "LIPS", "WIDE RNG", "BRI HUM", "FRETLESS",

  // Sublist 131 - 133: Amp Gain switch types
  "LOW", "MID", "HIGH",

  // Sublist 134 - 142: Amp speaker types
  "OFF", "ORIG", "1x8", "1x10", "1x12", "2x12", "4x10", "4x12", "8x12",

  // Sublist 143 - 179: Alternative tuning types
  "OPEN-D", "OPEN-E", "OPEN-G", "OPEN-A", "DROP-D", "DROP-Db", "DROP-C", "DROP-B", "DROP-Bb", "DROP-A", "D-MODAL", "NASHVL", "-12 STEP", "-11 STEP", "-10 STEP", "-9 STEP", "-8 STEP", "-7 STEP",
  "-6 STEP", "-5 STEP", "-4 STEP", "-3 STEP", "-2 STEP", "-1 STEP", "+1 STEP", "+2 STEP", "+3 STEP", "+4 STEP", "+5 STEP", "+6 STEP", "+7 STEP", "+8 STEP", "+9 STEP", "+10 STEP", "+11 STEP", "+12 STEP", "USER",

  // Sublist 180 - 185: Amp Gain switch types
  "CRY", "VO", "FAT", "LIGHT", "7-STRING", "RESO",

};

const uint16_t GP10_SIZE_OF_SUBLIST = GP10_sublists.size();

QString GP10_class::read_parameter_name(uint16_t par_no)
{
    if (par_no < number_of_parameters())  return GP10_parameters[par_no].Name;
      else return "?";
}

QString GP10_class::read_parameter_state(uint16_t par_no, uint8_t value)
{
    if (par_no < number_of_parameters())  {
        uint16_t my_sublist = GP10_parameters[par_no].Sublist;
        if ((my_sublist > 0) && !(my_sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
          switch (my_sublist) {
            case SHOW_NUMBER:
              return QString::number(value);
              break;
            case SHOW_DOUBLE_NUMBER:
              return QString::number(value * 2); //Patch level is displayed double
              break;
            case SHOW_PAN:
              if (value < 64) return "L" + QString::number(50 - value);
              if (value == 64) return "C";
              if (value > 64) return "R" + QString::number(value - 50);
              break;
            default:
              return GP10_sublists[my_sublist + value - 1];
              break;
          }
        }
        else if (value == 1) return "ON";
        else return "OFF";
      }
      return "?";
}

uint16_t GP10_class::number_of_parameters()
{
    return GP10_NUMBER_OF_PARAMETERS;
}

uint8_t GP10_class::number_of_values(uint16_t parameter)
{
    if (parameter < GP10_NUMBER_OF_PARAMETERS) return GP10_parameters[parameter].NumVals;
      else return 0;
}

uint8_t GP10_class::max_value(uint16_t par_no)
{
    if (par_no < GP10_NUMBER_OF_PARAMETERS) return GP10_parameters[par_no].NumVals -1;
    else return 0;
}

uint8_t GP10_class::get_number_of_assigns()
{
    return GP10_NUMBER_OF_ASSIGNS;
}

QString GP10_class::read_assign_name(uint8_t assign_no)
{
     if (assign_no < GP10_NUMBER_OF_ASSIGNS)  return "ASSIGN " + QString::number(assign_no + 1);
     else return "?";
}

QString GP10_class::read_assign_trigger(uint8_t trigger_no)
{
    if ((trigger_no > 0) && (trigger_no < 128)) return "CC#" + QString::number(trigger_no);
    else return "-";
}

uint8_t GP10_class::trigger_follow_assign(uint8_t assign_no)
{
    return assign_no + 21; // Default cc numbers are 21 and up
}

