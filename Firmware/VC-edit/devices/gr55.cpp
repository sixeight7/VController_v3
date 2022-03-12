#include "gr55.h"
#include "VController/config.h"
#include "VController/leds.h"
#include "VController/globals.h"

void GR55_class::init()
{
    device_name = "GR55";
    full_device_name = "Roland GR-55";
    patch_min = GR55_PATCH_MIN;
    patch_max = GR55_PATCH_MAX;
    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 3; // Default value: blue
    MIDI_port_manual = MIDI_port_number(GR55_MIDI_PORT);
    MIDI_channel = GR55_MIDI_CHANNEL; // Default value
    bank_number = 0; // Default value
    is_always_on = true; // Default value
    if (VC_type == VCONTROLLER) {
      my_device_page1 = GR55_DEFAULT_VC_PAGE1; // Default value
      my_device_page2 = GR55_DEFAULT_VC_PAGE2; // Default value
      my_device_page3 = GR55_DEFAULT_VC_PAGE3; // Default value
      my_device_page4 = GR55_DEFAULT_VC_PAGE4; // Default value
    }
    if (VC_type == VCMINI) {
      my_device_page1 = GR55_DEFAULT_VCMINI_PAGE1; // Default value
      my_device_page2 = GR55_DEFAULT_VCMINI_PAGE2; // Default value
      my_device_page3 = GR55_DEFAULT_VCMINI_PAGE3; // Default value
      my_device_page4 = GR55_DEFAULT_VCMINI_PAGE4; // Default value
    }
    if (VC_type == VCTOUCH) {
      my_device_page1 = GR55_DEFAULT_VCTOUCH_PAGE1; // Default value
      my_device_page2 = GR55_DEFAULT_VCTOUCH_PAGE2; // Default value
      my_device_page3 = GR55_DEFAULT_VCTOUCH_PAGE3; // Default value
      my_device_page4 = GR55_DEFAULT_VCTOUCH_PAGE4; // Default value
    }
    preset_banks = 40; // Default number of preset banks is 40. When we are in bass mode, there are only 12.
}

bool GR55_class::check_command_enabled(uint8_t cmd)
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

QString GR55_class::number_format(uint16_t patch_no)
{
      QString Output = "";
      uint16_t patch_number_corrected = 0; // Need a corrected version of the patch number to deal with the funny numbering system of the GR-55
      uint16_t bank_number_corrected = 0; //Also needed, because with higher banks, we start counting again

      bank_number_corrected = (patch_no / 3); // Calculate the bank number from the patch number

      if (bank_number_corrected < 99) {
        Output += "U";
        patch_number_corrected = patch_no;  //In the User bank all is normal
      }

      else {
        if (bank_number_corrected >= (99 + (2 * preset_banks))) {   // In the Other bank we have to adjust the bank and patch numbers so we start with O01-1
          Output += "O";
          patch_number_corrected = patch_no - (297 + (6 * preset_banks));
          bank_number_corrected = bank_number_corrected - (99 + (2 * preset_banks));
        }

        else {
          if (bank_number_corrected >= (99 + preset_banks)) {   // In the Rhythm bank we have to adjust the bank and patch numbers so we start with R01-1
            Output += "R";
            patch_number_corrected = patch_no - (297 + (3 * preset_banks));
            bank_number_corrected = bank_number_corrected - (99 + preset_banks);
          }

          else    {// In the Lead bank we have to adjust the bank and patch numbers so we start with L01-1
            Output += "L";
            patch_number_corrected = patch_no - 297;
            bank_number_corrected = bank_number_corrected - 99;
          }
        }
      }

      // Then add the bank number
      Output += QString::number(((patch_number_corrected / 3) + 1) / 10) + QString::number(((patch_number_corrected / 3) + 1) % 10);
      // Finally add the patch number
      Output += "-" + QString::number((patch_number_corrected % 3) + 1);

      return Output;
}

struct GR55_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  uint16_t Target; // Target of the assign as given in the assignments of the GR55 / GR55
  uint32_t Address; // The address of the parameter
  uint8_t NumVals; // The number of values for this parameter
  QString Name; // The name for the label
  uint16_t Sublist; // Which sublist to read for the FX or amp type - 0 if second byte does not contain the type or if there is no sublist +100 Show value from sublist.
  uint8_t Colour; // The colour for this effect.
  bool Reversed; // The GR-55 has SYNTH 1 SW, SYNTH 2 SW, COSM GUITAR SW and NORMAL PICKUP reversed. For these parameters the on and off values will be read and written in reverse
};

//typedef struct stomper Stomper;
#define GR55_MFX_COLOUR 255 // Just a colour number to pick the colour from the GR55_MFX_colours table
#define GR55_MFX_TYPE_COLOUR 254 //Another number for the MFX type
#define GR55_MOD_COLOUR 253 // Just a colour number to pick the colour from the GR55_MOD_colours table
#define GR55_MOD_TYPE_COLOUR 252 //Another number for the MOD type
#define SUBLIST_FROM_BYTE2 32768 //Bit 16 set high means read sublist frm byte 2
#define SHOW_NUMBER 31767 // Special number for showing the number (used for gain/volume/etc) - set in sublist
#define SHOW_DOUBLE_NUMBER 31766 // Special double byte number for showing the number (used for patch volume/etc) - set in sublist
#define SHOW_C64PAN 31765 // Special number for showing the number (used for gain/volume/etc) - set in sublist
#define SHOW_C127 31764 // Weird remapping of 0-127 to 0-100.

// All parameters that cannot be set from an assign have the target address at 0xFFF

QVector<GR55_parameter_struct> GR55_parameters = {
  {0x12B, 0x18000304, 2, "MFX", 1 | SUBLIST_FROM_BYTE2, GR55_MFX_COLOUR, false}, // 0
  {0x12C, 0x18000305, 20, "MFX TYPE", 1, GR55_MFX_TYPE_COLOUR, false},
  {0x0E6, 0x18000715, 2, "MOD", 63 | SUBLIST_FROM_BYTE2, GR55_MOD_COLOUR, false},
  {0x0E7, 0x18000716, 14, "MOD TYPE", 63, GR55_MOD_TYPE_COLOUR, false},
  {0x000, 0x18002003, 2, "SYNTH1 SW", 0, FX_GTR_TYPE, true},
  {0x003, 0x18002005, 68, "PCM1 TONE OCT", 153, FX_GTR_TYPE, false}, // Not a perfect solution, as there is no minimal value
  {0x03B, 0x18002103, 2, "SYNTH2 SW", 0, FX_GTR_TYPE, true},
  {0x03E, 0x18002105, 68, "PCM2 TONE OCT", 153, FX_GTR_TYPE, false}, // Not a perfect solution, as there is no minimal value
  {0x076, 0x1800100A, 2, "COSM GT SW", 0, FX_GTR_TYPE, true},
  {0x081, 0x1800101D, 2, "12STRING SW", 0, FX_PITCH_TYPE, false},
  {0x0D6, 0x18000700, 2, "AMP", 21 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE, false}, // 10
  {0xFFF, 0x18000701, 42, "AMP TP", 21, FX_AMP_TYPE, false},
  {0x0D7, 0x18000702, 121, "AMP GAIN", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0x0D8, 0x18000703, 101, "AMP LVL", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0x0D9, 0x18000704, 3, "AMP GAIN SW", 141, FX_AMP_TYPE, false},
  {0x0DA, 0x18000705, 2, "AMP SOLO SW", 0, FX_AMP_TYPE, false},
  {0x0DC, 0x18000707, 101, "AMP BASS", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0x0DD, 0x18000708, 101, "AMP MID", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0x0DE, 0x18000709, 101, "AMP TREBLE", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0x0DF, 0x1800070A, 101, "AMP PRESC", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0x0E0, 0x1800070B, 2, "AMP BRIGHT", 0, FX_AMP_TYPE, false}, // 20
  {0x0E1, 0x1800070C, 9, "SPKR TYPE", 144, FX_AMP_TYPE, false},
  {0x128, 0x1800075A, 2, "NS SWITCH", 0, FX_FILTER_TYPE, false},
  {0x1EC, 0x18000605, 2, "DLY SW", 129 | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE, false},
  {0x1ED, 0x18000606, 7, "DLY TYPE", 129, FX_DELAY_TYPE, false},
  {0x1F4, 0x1800060C, 2, "RVRB SW", 136 | SUBLIST_FROM_BYTE2, FX_REVERB_TYPE, false},
  {0x1F5, 0x1800060D, 5, "RVRB TYPE", 136, FX_REVERB_TYPE, false},
  {0x1FC, 0x18000600, 2, "CHOR SW", 125 | SUBLIST_FROM_BYTE2, FX_MODULATE_TYPE,  false},
  {0x1FD, 0x18000601, 4, "CHOR TYPE", 125, FX_MODULATE_TYPE,  false},
  {0x204, 0x18000611, 2, "EQ SWITCH", 0, FX_FILTER_TYPE, false},
  {0x213, 0x18000234, 2, "TUN SW", 160 | SUBLIST_FROM_BYTE2, FX_PITCH_TYPE, false}, // 30
  {0x214, 0x18000235, 13, "TUNING", 160, FX_PITCH_TYPE, false},
  {0x216, 0x18000230, 201, "PATCH LVL", SHOW_DOUBLE_NUMBER, FX_FILTER_TYPE, false},
  {0xFFF, 0x18000011, 2, "CTL", 94 | SUBLIST_FROM_BYTE2, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x18000012, 16, "CTL SW", 94, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x1800001F, 10, "EXP", 84, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x18000036, 10, "EXP ON", 84, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x1800004D, 2, "EXP SW", 111 | SUBLIST_FROM_BYTE2, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x1800004E, 14, "EXP SW", 111, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x1800005B, 10, "GK VOL", 84, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x18000072, 14, "GK S1", 111, FX_DEFAULT_TYPE, false}, // 40
  {0xFFF, 0x1800007F, 14, "GK S2", 111, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x02000007, 2, "GTR2MIDI", 0, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x02000008, 2, "G2M MODE", 77, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x02000009, 2, "G2M CHOMATIC", 0, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x02000016, 5, "GTR OUT", 79, FX_DEFAULT_TYPE, false},
};

#define GR55_EXP_SW 37
#define GR55_EXP 35
#define GR55_EXP_ON 36

const uint16_t GR55_NUMBER_OF_PARAMETERS = GR55_parameters.size();

//#define GR55_SIZE_OF_SUBLISTS 76
QStringList GR55_sublists = {
  // Sublist 1 - 20: MFX FX types
  "EQ", "S FILTR", "PHASER", "STEP PHR", "RING MOD", "TREMOLO", "AUTO PAN", "SLICER", "K ROTARY", "HEXA-CHS",
  "SPACE-D", "FLANGER", "STEP FLR", "AMP SIM", "COMPRESS", "LIMITER", "3TAP DLY", "TIME DLY", "LOFI CPS", "PITCH SH",

  // Sublist 21 - 62: Amp types
  "BOSS CLN", "JC-120", "JAZZ CBO", "FULL RNG", "CLN TWIN", "PRO CRCH", "TWEED", "DELUX CR", "BOSS CRH", "BLUES",
  "WILD CRH", "STACK CR", "VO DRIVE", "VO LEAD", "VO CLEAN", "MATCH DR", "FAT MTCH", "MATCH LD", "BG LEAD", "BG DRIVE",
  "BG RHYTH", "MS'59 I", "MS'59 II", "MS HIGN", "MS SCOOP", "R-FIER V", "R-FIER M", "R-FIER C", "T-AMP LD", "T-AMP CR",
  "T-AMP CL", "BOSS DRV", "SLDN", "LEAD STK", "HEAVY LD", "BOSS MTL", "5150 DRV", "METAL LD", "EDGE LD", "BASS CLN",
  "BASS CRH", "BASS HIG",

  // Sublist 63 - 76: MOD FX types
  "OD/DS", "WAH", "COMP", "LIMITER", "OCTAVE", "PHASER", "FLANGER", "TREMOLO", "ROTARY", "UNI-V",
  "PAN", "DELAY", "CHORUS", "EQ",

  // Sublist 77-78: G2M Mode
  "MONO", "POLY",

  // Sublist 79-83: Guitar out source select
  "PATCH", "OFF", "NRML PU", "MODEL", "BOTH",

  // Sublist 84-93: Assign function
  "OFF", "PTCH VOL", "TONE VOL", "PITCH BD", "MOD", "X FADER", "DLY LVL", "RVRB LVL", "CHOR LVL", "MOD CTL",

  // Sublist 94-110: Button assign function
  "OFF", "HOLD", "TAPTEMPO", "TONE SW", "AMP SW", "MOD SW", "MFX SW", "DLY SW", "RVRB SW", "CHOR SW", "AP PLAY", "AP SONG+", "AP SONG-", "AP SW", "VLINK SW", "LED MOMT", "LED TOGL",

  // Sublist 111-124: Button assign function
  "OFF", "TAPTEMPO", "TONE SW", "AMP SW", "MOD SW", "MFX SW", "DLY SW", "RVRB SW", "CHOR SW", "AP PLAY", "AP SONG+", "AP SONG-", "AP SW", "VLINK SW",

  // Sublist 125-128: Chorus Type
  "MONO", "STEREO", "MON MILD", "ST MILD",

  // Sublist 129 - 135: Delay type
  "SINGLE", "PAN", "REVERSE", "ANALOG", "TAPE", "MODULATE", "HICUT",

  // Sublist 136 - 140: Reverb type
  "AMBIENCE", "ROOM", "HALL1", "HALL2", "PLATE",

  // Sublist 141 - 143: Amp gain switch
  "LOW", "MID", "HIGH",

  //Sublist 144 - 152: Amp speaker type
  "OFF", "ORIGIN", "1x8\"", "1x10\"", "1x12\"", "2x12\"", "4x10\"", "4x12\"", "8x12\"",

  // Sublist 153 - 159: Part Octave shift - this sublist has to be called from 92 because the minimum value of the data is 61
  "-3", "-2", "-1", "0", "+1", "+2", "+3",

  // Sublist 160 - 172: Tunings
  "OPEN-D", "OPEN-E", "OPEN-G", "OPEN-A", "DROP-D", "D-MODAL", "-1 STEP", "-2 STEP", "BARITONE", "NASHVL", "-1 OCT", "+1 OCT", "USER",
};

const uint16_t GR55_SIZE_OF_SUBLIST = GR55_sublists.size();

QString GR55_class::read_parameter_name(uint16_t par_no)
{
    if (par_no < number_of_parameters())  return GR55_parameters[par_no].Name;
    else return "?";
}

QString GR55_class::read_parameter_state(uint16_t par_no, uint8_t value)
{
    QString Output = "";
    if (par_no < number_of_parameters())  {
        uint16_t my_sublist = GR55_parameters[par_no].Sublist;
        if ((my_sublist > 0) && !(my_sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
          switch (my_sublist) {
            case SHOW_NUMBER:
            case SHOW_DOUBLE_NUMBER:
              //if (GR55_parameters[number].Address == 0x2000) Output += QString::number(value * 2); //Patch level is displayed double
              Output += QString::number(value);
              break;
            case SHOW_C64PAN:
              if (value < 64) Output += "L" + QString::number(50 - value);
              if (value == 64) Output += "C";
              if (value > 64) Output += "R" + QString::number(value - 50);
              break;
            default:
              QString type_name = GR55_sublists[my_sublist + value - 1];
              Output += type_name;
              break;
          }
        }
        else if (value == 1) Output += "ON";
        else Output += "OFF";
      }
      else Output += "?";
      return Output;
}

uint16_t GR55_class::number_of_parameters()
{
  return GR55_NUMBER_OF_PARAMETERS;
}

uint8_t GR55_class::max_value(uint16_t par_no)
{
    if (par_no < GR55_NUMBER_OF_PARAMETERS) return GR55_parameters[par_no].NumVals - 1;
    else return 0;
}

uint8_t GR55_class::min_value(uint16_t par_no)
{
    if (par_no < GR55_NUMBER_OF_PARAMETERS) {
        if ((GR55_parameters[par_no].Address == 0x18002005) || (GR55_parameters[par_no].Address == 0x18002105)) return 61;
        else return 0;
    }
    else return 0;
}

uint8_t GR55_class::get_number_of_assigns()
{
    return GR55_NUMBER_OF_ASSIGNS;
}

QString GR55_class::read_assign_name(uint8_t assign_no)
{
    if (assign_no < GR55_NUMBER_OF_ASSIGNS)  return "ASSIGN " + QString::number(assign_no + 1);
    else return "?";
}

QString GR55_class::read_assign_trigger(uint8_t trigger_no)
{
    if ((trigger_no > 0) && (trigger_no < 128)) return "CC#" + QString::number(trigger_no);
    else return "-";
}

uint8_t GR55_class::trigger_follow_assign(uint8_t assign_no)
{
    return assign_no + 21; // Default cc numbers are 21 and up
}


