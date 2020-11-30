#include "sy1000.h"

#include "VController/config.h"
#include "VController/leds.h"

void SY1000_class::init()
{
    device_name = "SY1000";
    full_device_name = "Boss SY-1000";
    patch_min = SY1000_PATCH_MIN;
    patch_max = SY1000_PATCH_MAX;
    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 4; // Default value: orange
    MIDI_channel = SY1000_MIDI_CHANNEL; // Default value
    bank_number = 0; // Default value
    is_always_on = true; // Default value
    my_device_page1 = SY1000_DEFAULT_PAGE1; // Default value
    my_device_page2 = SY1000_DEFAULT_PAGE2; // Default value
    my_device_page3 = SY1000_DEFAULT_PAGE3; // Default value
    my_device_page4 = SY1000_DEFAULT_PAGE4; // Default value
}

bool SY1000_class::check_command_enabled(uint8_t cmd)
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

QString SY1000_class::number_format(uint16_t patch_no)
{
   QString numberString = "";
   if (patch_no < 200) numberString.append("U");
   else numberString.append("P");;
   uint8_t bank_no = ((patch_no % 200) / 4) + 1;
   numberString.append(QString::number(bank_no / 10));
   numberString.append(QString::number(bank_no % 10));
   numberString.append('-');
   numberString.append(QString::number((patch_no % 4) + 1));
   return numberString;
}

struct SY1000_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  uint16_t Target_GM; // Target of the assign as given in the assignments of the SY1000
  uint16_t Target_BM; // Target of the assign as given in the assignments of the SY1000
  uint32_t Address_GM; // The address of the parameter in guitar mode
  uint32_t Address_BM; // The address of the parameter in bass mode
  uint8_t NumVals; // The number of values for this parameter
  QString Name; // The name for the label
  uint16_t Sublist; // Which sublist to read for the FX or amp type - 0 if second byte does not contain the type or if there is no sublist +100 Show value from sublist.
  uint8_t Colour; // The colour for this effect
};

//typedef struct stomper Stomper;
#define SY1000_FX_COLOUR 255 // Just a colour number to pick the colour from the SY1000_FX_colours table
#define SY1000_FX_TYPE_COLOUR 254 //Another number for the FX type
#define SUBLIST_FROM_BYTE2 32768 //Bit 16 set high means read sublist frm byte 2
#define SHOW_NUMBER 31767 // Special number for showing the number (used for gain/volume/etc) - set in sublist
#define SHOW_DOUBLE_NUMBER 31766 // Show number times 2 (used for patch volume/etc) - set in sublist
#define SHOW_PAN 31765 // Special number for showing the pan- set in sublist
#define SY1000_INST_SUBLIST 31750

QVector<SY1000_parameter_struct> SY1000_parameters = {
      {  11,   11, 0x10001500, 0x10021500, 2, "INST1", SY1000_INST_SUBLIST | SUBLIST_FROM_BYTE2, FX_GTR_TYPE}, // 00
      { 339,  339, 0x10002000, 0x10021F00, 2, "INST2", SY1000_INST_SUBLIST | SUBLIST_FROM_BYTE2, FX_GTR_TYPE},
      { 669,  669, 0x10002B00, 0x10022900, 2, "INST3", SY1000_INST_SUBLIST | SUBLIST_FROM_BYTE2, FX_GTR_TYPE},
      {1014,  657, 0x10001201, 0x10021201, 2, "NORMAL PU", 0, FX_GTR_TYPE},
      {1967, 1910, 0x10003600, 0x10023300, 2, "COMP", 16 | SUBLIST_FROM_BYTE2, FX_GTR_TYPE},
      {1848, 1791, 0x10003700, 0x10023400, 2, "DIST", 23 | SUBLIST_FROM_BYTE2, FX_GTR_TYPE},
      {1828, 1771, 0x10003800, 0x10023500, 2, "AMP", 58 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE},
      {1840, 1783, 0x1000380C, 0x1002350C, 2, "AMP SOLO SW", 0, FX_AMP_TYPE},
      { 316,  297, 0x10001A16, 0x10021A22, 2, "AMP1 SOLO SW", 0, FX_AMP_TYPE},
      { 646,  608, 0x10002516, 0x10022422, 2, "AMP2 SOLO SW", 0, FX_AMP_TYPE},
      { 976,  919, 0x10003016, 0x10022E22, 2, "AMP3 SOLO SW", 0, FX_AMP_TYPE},
      {1930, 1873, 0x10003900, 0x10023600, 2, "NS", 0, FX_GTR_TYPE},
      {1857, 1800, 0x10003A00, 0x10023700, 2, "EQ1", 0, FX_FILTER_TYPE},
      {1869, 1812, 0x10003B00, 0x10023800, 2, "EQ2", 0, FX_FILTER_TYPE},
      {1905, 1848, 0x10003C00, 0x10023900, 2, "DELAY 1", 0, FX_DELAY_TYPE},
      {1912, 1855, 0x10003D00, 0x10023A00, 2, "DELAY 2", 0, FX_DELAY_TYPE},
      {1881, 1824, 0x10003E00, 0x10023B00, 2, "MST DLY", 89 | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE},
      {1942, 1885, 0x10003F00, 0x10023C00, 2, "CHORUS", 101 | SUBLIST_FROM_BYTE2, FX_MODULATE_TYPE},
      {1018,  961, 0x10004000, 0x10023D00, 2, "FX1", 105 | SUBLIST_FROM_BYTE2, SY1000_FX_COLOUR},
      {1288, 1231, 0x10006600, 0x10026300, 2, "FX2", 105 | SUBLIST_FROM_BYTE2, SY1000_FX_COLOUR},
      {1558, 1501, 0x10010C00, 0x10030900, 2, "FX3", 105 | SUBLIST_FROM_BYTE2, SY1000_FX_COLOUR},
      {1919, 1862, 0x10013200, 0x10032F00, 2, "RVB", 142 | SUBLIST_FROM_BYTE2, FX_REVERB_TYPE},
      {0xFFFF, 0xFFFF, 0x1000002A, 0x1002002A, 46, "EXP1 TP", 149, FX_DEFAULT_TYPE},
      {0xFFFF, 0xFFFF, 0x1000002B, 0x1002002B, 46, "EXP2 TP", 149, FX_DEFAULT_TYPE},
};

const uint16_t SY1000_NUMBER_OF_PARAMETERS = SY1000_parameters.size();

QStringList SY1000_sublists = {
    // Sublist 1 - 8: INST types (GTR mode)
    "DYN SYN", "OSC SYN", "GR-300", "E.GUITAR", "ACOUSTIC", "E.BASS", "VIO GTR", "POLY FX",

    // Sublist 9 - 15: INST types (BASS mode)
    "DYN SYN", "OSC SYN", "ANALG GR", "E.BASS", "AC BASS", "E.GTR", "POLY FX",

    // Sublist 16 - 22: COMP types
    "BOSS CMP", "Hi-BAND", "LIGHT", "D-COMP", "ORANGE", "FAT", "MILD",

    // Sublist 23 - 57: DIST Types
    "MID BST", "CLN BST", "TREBLE B", "CRUNCH", "NAT OD", "WARM OD", "FAT DS", "LEAD DS", "METAL DS", "OCT FUZZ",
    "A-DIST", "X-OD", "X-DIST", "BLUES OD", "OD-1", "T-SCREAM", "TURBO OD", "DIST", "CENTA OD", "RAT",
    "GUV DS", "DIST+", "MTL ZONE", "HM-2", "MTL CORE", "60S FUZZ", "MUFF FUZ", "BASS OD", "BASS DS", "BASS MT",
    "BASS FUZ", "HI BND D", "X-BAS OD", "BASS DRV", "BASS DI",

    // Sublist 58 - 88: AMP types
    "TRANSP", "NATURAL", "BOUTIQUE", "SUPREME", "MAXIMUM", "JUGGERNT", "X-CRUNCH", "X-HI GN", "X-MODDED", "JC-120",
    "TWIN CMB", "DELUXE" , "TWEED", "DIAMOND", "BRIT STK", "RECTI ST", "MATCH CB", "BG COMBO", "ORNG STK", "BGNR UB",
    "NAT BASS", "X-DRV BS", "CONCERT", "SUPER FL", "FLIP TOP", "B MAN", "BASS 360", "SW-TOUR", "AC BASS", "GK BASS", "MARK",

    // Sublist 89 - 100: MAST DLY types
    "STEREO1", "STEREO2", "PAN", "DUAL-S", "DUAL-P", "DUAL L/R", "REVERSE", "ANALOG", "TAPE", "MOD", "WARP", "TWIST",

    // Sublist 101 - 104: CHORUS types
    "MONO", "STEREO1", "STEREO2", "DUAL",

    // Sublist 105 - 141: FX types
    "AC RESO", "AUTO WAH", "CHORUS", "CL-VIBE", "COMPRSSR", "DEFRETTR", "DEFRET B", "DELAY", "FLANGER", "FLANGR B",
    "FOOT VOL", "GRAPH EQ", "HARMO", "HUMANIZR", "ISOLATOR", "LIMITER", "LO-FI", "OCTAVE", "OCT BASS", "PAN",
    "PARAM EQ", "PDL BEND", "PHASER", "PITCH SH", "REVERB", "RING MOD", "ROTARY", "SITAR SM", "SLICER", "SLOW GR",
    "SLW GR B", "SND HOLD", "TOUCH W", "TW BASS", "TREMOLO", "VIBRATO", "WAH",

    // Sublist 142 - 148: REVERB types
    "AMBIENCE", "ROOM", "HALL1", "HALL2", "PLATE", "SPRING", "MOD",

    // Sublist 149 - 203: EXP pedal types
    "OFF", "FV1", "FV2", "FV1+TUNR", "FV2+TUNR", "FX1 P.PS", "FX2 P.PS", "FX3 P.PS", "P.LVL100", "P.LVL200",
    "INST1 LV", "INST2 LV", "INST3 LV", "INST ALL", "I1 CUTOF", "I2 CUTOF", "I3 CUTOF", "ALL CUTF", "I1 RESO", "I2 RESO",
    "I3 RESO", "ALL RESO", "1:GTR VL", "2:GTR VL", "3:GTR VL", "ALL:G VL", "1:NOR100", "1:NOR200", "2:NOR100", "2:NOR200",
    "3:NOR100", "3:NOR200", "AL:NM100", "AL:NM200", "1:STR B", "2:STR B", "3:STR B", "AL:STR B", "1:DYNA B", "2:DYNA B",
    "3:DYNA B", "A:DYNA B", "MIXR A/B", "BAL1 A/B", "BAL2 A/B", "BAL3 A/B",
};

const uint16_t SY1000_SIZE_OF_SUBLIST = SY1000_sublists.size();

QString SY1000_class::read_parameter_name(uint16_t par_no)
{
    if (par_no < number_of_parameters())  return SY1000_parameters[par_no].Name;
      else return "?";
}

QString SY1000_class::read_parameter_state(uint16_t par_no, uint8_t value)
{
    if (par_no < number_of_parameters())  {
        uint16_t my_sublist = SY1000_parameters[par_no].Sublist;
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
              return SY1000_sublists[my_sublist + value - 1];
              break;
          }
        }
        else if (value == 1) return "ON";
        else return "OFF";
      }
      return "?";
}

uint16_t SY1000_class::number_of_parameters()
{
    return SY1000_NUMBER_OF_PARAMETERS;
}

uint8_t SY1000_class::number_of_values(uint16_t parameter)
{
    if (parameter < SY1000_NUMBER_OF_PARAMETERS) return SY1000_parameters[parameter].NumVals;
      else return 0;
}

uint8_t SY1000_class::max_value(uint16_t par_no)
{
    if (par_no < SY1000_NUMBER_OF_PARAMETERS) return SY1000_parameters[par_no].NumVals -1;
    else return 0;
}

uint8_t SY1000_class::get_number_of_assigns()
{
    return SY1000_NUMBER_OF_ASSIGNS;
}

QString SY1000_class::read_assign_name(uint8_t assign_no)
{
     if (assign_no < SY1000_NUMBER_OF_ASSIGNS)  return "ASSIGN " + QString::number(assign_no + 1);
     else return "?";
}

QString SY1000_class::read_assign_trigger(uint8_t trigger_no)
{
    if ((trigger_no > 0) && (trigger_no < 128)) return "CC#" + QString::number(trigger_no);
    else return "-";
}

uint8_t SY1000_class::trigger_follow_assign(uint8_t assign_no)
{
    return assign_no + 21; // Default cc numbers are 21 and up
}
