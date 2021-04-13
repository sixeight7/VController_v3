#include "sy1000.h"

#include "VController/config.h"
#include "VController/leds.h"
#include "VController/globals.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

void SY1000_class::init()
{
    device_name = "SY1000";
    full_device_name = "Boss SY-1000";
    patch_min = SY1000_PATCH_MIN;
    patch_max = SY1000_PATCH_MAX;
    patch_min_as_stored_on_VC = SY1000_PATCH_MIN;
    patch_max_as_stored_on_VC = 199;
    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 11; // Default value: light blue
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
    case SNAPSCENE:
    //case LOOPER:
        return true;
    }
    return false;
}

QString SY1000_class::number_format(uint16_t patch_no)
{
   bool bm = ((patch_no & BASS_MODE_NUMBER_OFFSET) == BASS_MODE_NUMBER_OFFSET);
   patch_no &= 0x0FFF;
    QString numberString = "";
   if (patch_no < 200) numberString.append("U");
   else numberString.append("P");;
   uint8_t bank_no = ((patch_no % 200) / 4) + 1;
   numberString.append(QString::number(bank_no / 10));
   numberString.append(QString::number(bank_no % 10));
   numberString.append('-');
   numberString.append(QString::number((patch_no % 4) + 1));

   if (bm) { // Is bass mode patch
       numberString.append("-BM");
   }
   return numberString;
}

struct SY1000_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  uint8_t Ctl_target; // The number of this effect in the target list of the CTLx
  uint16_t Target_GM; // Target of the assign as given in the assignments of the SY1000
  uint16_t Target_BM; // Target of the assign as given in the assignments of the SY1000
  uint32_t Address_GM; // The address of the parameter in guitar mode
  uint32_t Address_BM; // The address of the parameter in bass mode
  uint8_t NumVals; // The number of values for this parameter
  QString Name; // The name for the label
  uint16_t Sublist; // Which sublist to read for the FX or amp type - 0 if second byte does not contain the type or if there is no sublist +100 Show value from sublist.
  uint8_t Colour; // The colour for this effect
  uint8_t Type; // Types - used for addressess that depend on the INST_type
};

#define SY1000_FX_COLOUR 255 // Just a colour number to pick the colour from the SY1000_FX_colours table
#define SY1000_FX_TYPE_COLOUR 254 //Another number for the FX type
#define SUBLIST_FROM_BYTE2 32768 //Bit 16 set high means read sublist from byte 2
#define SHOW_NUMBER 31767 // Special number for showing the number (used for gain/volume/etc) - set in sublist
#define SHOW_DOUBLE_NUMBER 31766 // Show number times 2 (used for patch volume/etc) - set in sublist
#define SHOW_PAN 31765 // Special number for showing the pan- set in sublist
#define SY1000_INST_SUBLIST 31750
#define PREV2 0x8000 // To execute the previous 2 items
#define PREV3 0x8001 // To execute the previous 3 items
#define PREV9 0x8002 // To execute the previous 9 items - existing PREV2 items are skipped

// Types - used for addressess that depend on the INST_type, like LFO,
#define INST1_LFO 1
#define INST2_LFO 2
#define INST3_LFO 3
#define DO_TAP_TEMPO 4

#define VIA_ASSIGN 80 // Special address for controlling parameter via an assign

QVector<SY1000_parameter_struct> SY1000_parameters = {
    {  20,   11,   11, 0x10001500, 0x10021500, 2, "INST1", SY1000_INST_SUBLIST | SUBLIST_FROM_BYTE2, FX_GTR_TYPE, 0 },
    {  21,  339,  339, 0x10002000, 0x10021F00, 2, "INST2", SY1000_INST_SUBLIST | SUBLIST_FROM_BYTE2, FX_GTR_TYPE, 0 },
    {  22,  669,  669, 0x10002B00, 0x10022900, 2, "INST3", SY1000_INST_SUBLIST | SUBLIST_FROM_BYTE2, FX_GTR_TYPE, 0 },
    {  23, PREV3, PREV3, 0x10001500, 0x10021500, 2, "INST ALL", SY1000_INST_SUBLIST | SUBLIST_FROM_BYTE2, FX_GTR_TYPE, 0 },
    { 100, 1967, 1910, 0x10003600, 0x10023300, 2, "COMP", 16 | SUBLIST_FROM_BYTE2, FX_DYNAMICS_TYPE, 0 },
    { 101, 1848, 1791, 0x10003700, 0x10023400, 2, "DIST", 23 | SUBLIST_FROM_BYTE2, FX_DIST_TYPE, 0 },
    { 102, 1855, 1798, 0x10003707, 0x10023407, 2, "DS SOLO", 0, FX_DIST_TYPE, 0 },
    { 103, 1828, 1771, 0x10003800, 0x10023500, 2, "AMP", 58 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE, 0 },
    { 104, 1840, 1783, 0x1000380C, 0x1002350C, 2, "AMP SOLO SW", 0, FX_AMP_TYPE, 0 },
    { 108, 1905, 1848, 0x10003C00, 0x10023900, 2, "DELAY 1", 0, FX_DELAY_TYPE, 0 },
    { 109, 1912, 1855, 0x10003D00, 0x10023A00, 2, "DELAY 2", 0, FX_DELAY_TYPE, 0 },
    { 110, 1881, 1824, 0x10003E00, 0x10023B00, 2, "MST DLY", 89 | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE, 0 },
    { 117, 1889, 1832, 0x10003E1A, 0x10023B1A, 2, "MST DLY TRIG", 0, FX_DELAY_TYPE, 0 },
    { 111, 1942, 1885, 0x10003F00, 0x10023C00, 2, "CHORUS", 101 | SUBLIST_FROM_BYTE2, FX_MODULATE_TYPE, 0 },
    {  97, 1018,  961, 0x10004000, 0x10023D00, 2, "FX1", 105 | SUBLIST_FROM_BYTE2, SY1000_FX_COLOUR, 0 },
    {   0, 1019,  962, 0x10004001, 0x10023D01, 37, "FX1", 105, SY1000_FX_TYPE_COLOUR, 0 },
    {  98, 1288, 1231, 0x10006600, 0x10026300, 2, "FX2", 105 | SUBLIST_FROM_BYTE2, SY1000_FX_COLOUR, 0 },
    {   0, 1289, 1232, 0x10006601, 0x10026301, 37, "FX2", 105, SY1000_FX_TYPE_COLOUR, 0 },
    {  99, 1558, 1501, 0x10010C00, 0x10030900, 2, "FX3", 105 | SUBLIST_FROM_BYTE2, SY1000_FX_COLOUR, 0 },
    {   0, 1559, 1502, 0x10010C01, 0x10030901, 37, "FX2", 105, SY1000_FX_TYPE_COLOUR, 0 },
    { 112, 1919, 1862, 0x10013200, 0x10032F00, 2, "RVB", 142 | SUBLIST_FROM_BYTE2, FX_REVERB_TYPE, 0 },
    { 118, 1981, 1923, 0x10001221, 0x10021221, 2, "DIV CH SELECT", 0, FX_DEFAULT_TYPE, 0 },
    { 107, 1930, 1873, 0x10003900, 0x10023600, 2, "NS", 0, FX_DYNAMICS_TYPE, 0 },
    { 105, 1857, 1800, 0x10003A00, 0x10023700, 2, "EQ1", 0, FX_FILTER_TYPE, 0 },
    { 106, 1869, 1812, 0x10003B00, 0x10023800, 2, "EQ2", 0, FX_FILTER_TYPE, 0 },
    {  27, 1014,  657, 0x10001201, 0x10021201, 2, "NORMAL PU", 0, FX_GTR_TYPE, 0 },
    {  91,  304,  285, 0x10001A0A, 0x10021A16, 2, "1:AMP", 58 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE, 0 },
    {  92,  634,  596, 0x1000250A, 0x10022416, 2, "2:AMP", 58 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE, 0 },
    {  93,  964,  907, 0x1000300A, 0x10022E16, 2, "3:AMP", 58 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE, 0 },
    {  94,  316,  297, 0x10001A16, 0x10021A22, 2, "1:AMP SOLO SW", 0, FX_AMP_TYPE, 0 },
    {  95,  646,  608, 0x10002516, 0x10022422, 2, "2:AMP SOLO SW", 0, FX_AMP_TYPE, 0 },
    {  96,  976,  919, 0x10003016, 0x10022E22, 2, "3:AMP SOLO SW", 0, FX_AMP_TYPE, 0 },
    {  28,   28,   28, 0x10001F00, 0x10021E00, 2, "1:ALT TN", 149 | SUBLIST_FROM_BYTE2, FX_GTR_TYPE, 0 },
    {  29,  356,  356, 0x10002A00, 0x10022800, 2, "2:ALT TN", 149 | SUBLIST_FROM_BYTE2, FX_GTR_TYPE, 0 },
    {  30,  686,  686, 0x10003500, 0x10023200, 2, "3:ALT TN", 149 | SUBLIST_FROM_BYTE2, FX_GTR_TYPE, 0 },
    {  31, PREV3, PREV3, 0x10001F00, 0x10021E00, 2, "A:ALT TN", 149 | SUBLIST_FROM_BYTE2, FX_GTR_TYPE, 0 },
    {  32,   42,   42, 0x10001F0E, 0x10021E0E, 2, "INST1: 12-STRING", 0, FX_GTR_TYPE, 0 },
    {  33,  370,  370, 0x10002A0E, 0x1002280E, 2, "INST2: 12-STRING", 0, FX_GTR_TYPE, 0 },
    {  34,  700,  700, 0x1000350E, 0x1002320E, 2, "INST3: 12-STRING", 0, FX_GTR_TYPE, 0 },
    {  35, PREV3, PREV3, 0x10001F0E, 0x10021E0E, 2, "ALL: 12-STRING", 0, FX_GTR_TYPE, 0 },
    {  36,   68,   68, 0x10001F28, 0x10021E28, 2, "INST1: BEND", 0, FX_GTR_TYPE, 0 },
    {  37,  396,  396, 0x10002A28, 0x10022828, 2, "INST2: BEND", 0, FX_GTR_TYPE, 0 },
    {  38,  726,  726, 0x10003528, 0x10023228, 2, "INST3: BEND", 0, FX_GTR_TYPE, 0 },
    {  39, PREV3, PREV3, 0x10001F28, 0x10021E28, 2, "ALL: BEND", 0, FX_GTR_TYPE, 0 },
    {  80,  999,  942, VIA_ASSIGN, VIA_ASSIGN, 2, "INST1: HOLD", 0, FX_GTR_TYPE, 0 },
    {  81, 1000,  943, VIA_ASSIGN, VIA_ASSIGN, 2, "INST2: HOLD", 0, FX_GTR_TYPE, 0 },
    {  82, 1001,  944, VIA_ASSIGN, VIA_ASSIGN, 2, "INST3: HOLD", 0, FX_GTR_TYPE, 0 },
    {  83, 1002,  945, VIA_ASSIGN, VIA_ASSIGN, 2, "ALL: HOLD", 0, FX_GTR_TYPE, 0 },
    {  40,  102,  102, 0x10001627, 0x10021627, 2, "INST1: LFO1", 0, FX_GTR_TYPE, INST1_LFO },
    {  41,  112,  113, 0x10001631, 0x10021631, 2, "INST1: LFO2", 0, FX_GTR_TYPE, INST1_LFO },
    {  42, PREV2, PREV2, 0x10001627, 0x10021627, 2, "INST1: LFO1+2", 0, FX_GTR_TYPE, INST1_LFO },
    {  43,  432,  413, 0x10002127, 0x10022027, 2, "INST2: LFO1", 0, FX_GTR_TYPE, INST2_LFO },
    {  44,  442,  423, 0x10002131, 0x10022031, 2, "INST2: LFO2", 0, FX_GTR_TYPE, INST2_LFO },
    {  45, PREV2, PREV2, 0x10002127, 0x10022027, 2, "INST2: LFO1+2", 0, FX_GTR_TYPE, INST2_LFO },
    {  46,  762,  724, 0x10002C27, 0x10022A27, 2, "INST3: LFO1", 0, FX_GTR_TYPE, INST3_LFO },
    {  47,  772,  736, 0x10002A31, 0x10022A31, 2, "INST3: LFO2", 0, FX_GTR_TYPE, INST3_LFO },
    {  48, PREV2, PREV2, 0x10002C27, 0x10022A27, 2, "INST3: LFO1+2", 0, FX_GTR_TYPE, INST3_LFO },
    {  49, PREV9, PREV9, 0x10001627, 0x10021627, 2, "ALL: LFO", 0, FX_GTR_TYPE, INST1_LFO },
    {  50,  125,  125, 0x1000171E, 0x1002171E, 2, "INST1: SEQ1", 0, FX_GTR_TYPE, 0 },
    {  51,  131,  131, 0x10001734, 0x10021734, 2, "INST1: SEQ2", 0, FX_GTR_TYPE, 0 },
    {  52, PREV2, PREV2, 0x1000171E, 0x1002171E, 2, "INST1: SEQ1+2", 0, FX_GTR_TYPE, 0 },
    {  53,  455,  436, 0x1000221E, 0x1002211E, 2, "INST2: SEQ1", 0, FX_GTR_TYPE, 0 },
    {  54,  461,  442, 0x10002234, 0x10022134, 2, "INST2: SEQ2", 0, FX_GTR_TYPE, 0 },
    {  55, PREV2, PREV2, 0x1000221E, 0x1002211E, 2, "INST2: SEQ1+2", 0, FX_GTR_TYPE, 0 },
    {  56,  785,  747, 0x10002d1E, 0x10022B1E, 2, "INST3: SEQ1", 0, FX_GTR_TYPE, 0 },
    {  57,  791,  753, 0x10002D34, 0x10022B34, 2, "INST3: SEQ2", 0, FX_GTR_TYPE, 0 },
    {  58, PREV2, PREV2, 0x10002D1E, 0x10022B1E, 2, "INST3: SEQ1+2", 0, FX_GTR_TYPE, 0 },
    {  59, PREV9, PREV9, 0x1000171E, 0x1002171E, 2, "ALL: SEQ", 0, FX_GTR_TYPE, 0 },
    {  70,  130,  130, 0x10001723, 0x10021723, 2, "INST1: TURBO1", 0, FX_GTR_TYPE, 0 },
    {  71,  136,  136, 0x10001739, 0x10021739, 2, "INST1: TURBO2", 0, FX_GTR_TYPE, 0 },
    {  72, PREV2, PREV2, 0x10001723, 0x10021723, 2, "INST1: TURBO1+2", 0, FX_GTR_TYPE, 0 },
    {  73,  460,  441, 0x10002223, 0x10022123, 2, "INST2: TURBO1", 0, FX_GTR_TYPE, 0 },
    {  74,  466,  447, 0x10002239, 0x10022139, 2, "INST2: TURBO2", 0, FX_GTR_TYPE, 0 },
    {  75, PREV2, PREV2, 0x10002223, 0x10022123, 2, "INST2: TURBO1+2", 0, FX_GTR_TYPE, 0 },
    {  76,  790,  752, 0x10002D23, 0x10022B23, 2, "INST3: TURBO1", 0, FX_GTR_TYPE, 0 },
    {  77,  796,  758, 0x10002D39, 0x10022B39, 2, "INST3: TURBO2", 0, FX_GTR_TYPE, 0 },
    {  78, PREV2, PREV2, 0x10002D23, 0x10022B23, 2, "INST3: TURBO1+2", 0, FX_GTR_TYPE, 0 },
    {  79, PREV9, PREV9, 0x10001723, 0x10021723, 2, "ALL: TURBO", 0, FX_GTR_TYPE, 0 },
    {  60, 1003,  946, VIA_ASSIGN, VIA_ASSIGN, 2, "1: SEQ 1 TRIG", 0, FX_GTR_TYPE, 0 },
    {  61, 1004,  947, VIA_ASSIGN, VIA_ASSIGN, 2, "1: SEQ 2 TRIG", 0, FX_GTR_TYPE, 0 },
    {  62, 1005,  948, VIA_ASSIGN, VIA_ASSIGN, 2, "1: SEQ 1&2 TRIG", 0, FX_GTR_TYPE, 0 },
    {  63, 1006,  949, VIA_ASSIGN, VIA_ASSIGN, 2, "2: SEQ 1 TRIG", 0, FX_GTR_TYPE, 0 },
    {  64, 1007,  950, VIA_ASSIGN, VIA_ASSIGN, 2, "2: SEQ 2 TRIG", 0, FX_GTR_TYPE, 0 },
    {  65, 1008,  951, VIA_ASSIGN, VIA_ASSIGN, 2, "2: SEQ 1&2 TRIG", 0, FX_GTR_TYPE, 0 },
    {  66, 1009,  952, VIA_ASSIGN, VIA_ASSIGN, 2, "3: SEQ 1 TRIG", 0, FX_GTR_TYPE, 0 },
    {  67, 1010,  953, VIA_ASSIGN, VIA_ASSIGN, 2, "3: SEQ 2 TRIG", 0, FX_GTR_TYPE, 0 },
    {  68, 1011,  954, VIA_ASSIGN, VIA_ASSIGN, 2, "3: SEQ 1&2 TRIG", 0, FX_GTR_TYPE, 0 },
    {  69, 1012,  955, VIA_ASSIGN, VIA_ASSIGN, 2, "ALL: SEQ TRIG", 0, FX_GTR_TYPE, 0 },
    {  24,   14,   14, 0x10001503, 0x10021503, 2, "INST1: NOR MIX", 0, FX_GTR_TYPE, 0 },
    {  25,  342,  323, 0x10002003, 0x10021F03, 2, "INST2: NOR MIX", 0, FX_GTR_TYPE, 0 },
    {  26,  672,  634, 0x10002B03, 0x10022903, 2, "INST3: NOR MIX", 0, FX_GTR_TYPE, 0 },
    {  17, 0xFFFF, 0xFFFF, 0x0001103F, 0x0008103F, 2, "MANUAL", 0, FX_GTR_TYPE, 0 },
    {   9,    4,    4, VIA_ASSIGN, VIA_ASSIGN, 2, "BPM TAP", 0, FX_SHOW_TAP_TEMPO, DO_TAP_TEMPO },
    {   0, 0xFFFF, 0xFFFF, 0x1000001C, 0x1002001C, 121, "CTL1 TP", 231, FX_DEFAULT_TYPE, 0 },
    {   0, 0xFFFF, 0xFFFF, 0x1000001E, 0x1002001E, 121, "CTL2 TP", 231, FX_DEFAULT_TYPE, 0 },
    {   0, 0xFFFF, 0xFFFF, 0x1000002A, 0x1002002A, 46, "EXP1 TP", 186, FX_DEFAULT_TYPE, 0 },
    {   0, 0xFFFF, 0xFFFF, 0x1000002B, 0x1002002B, 46, "EXP2 TP", 186, FX_DEFAULT_TYPE, 0 },
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

    // Sublist 149 - 185: Alt Tune types
    "OPEN D", "OPEN E", "OPEN G", "OPEN A", "DROP D", "DROP Db", "DROP C", "DROP B", "DROP Bb", "DROP A",
    "D-MODAL", "NASHVL", "-12STEP", "-11STEP", "-10STEP", "-9STEP", "-8STEP", "-7STEP", "-6STEP", "-5STEP",
    "-4STEP", "-3STEP", "-2STEP", "-1STEP", "+1STEP", "+2STEP", "+3STEP", "+4STEP", "+5STEP", "+6STEP",
    "+7STEP", "+8STEP", "+9STEP", "+10STEP", "+11STEP", "+12STEP", "USER",

    // Sublist 186 - 230: EXP pedal types
    "OFF", "FV1", "FV2", "FV1+TUNR", "FV2+TUNR", "FX1 P.PS", "FX2 P.PS", "FX3 P.PS", "P.LVL100", "P.LVL200",
    "INST1 LV", "INST2 LV", "INST3 LV", "INST ALL", "I1 CUTOF", "I2 CUTOF", "I3 CUTOF", "ALL CUTF", "I1 RESO", "I2 RESO",
    "I3 RESO", "ALL RESO", "1:GTR VL", "2:GTR VL", "3:GTR VL", "ALL:G VL", "1:NOR100", "1:NOR200", "2:NOR100", "2:NOR200",
    "3:NOR100", "3:NOR200", "AL:NM100", "AL:NM200", "1:STR B", "2:STR B", "3:STR B", "AL:STR B", "1:DYNA B", "2:DYNA B",
    "3:DYNA B", "A:DYNA B", "MIXR A/B", "BAL1 A/B", "BAL2 A/B", "BAL3 A/B",

    // Sublist 231 - 351: CTL types
    "OFF", "*/BANK+", "BANK-", "PATCH +1", "PATCH -1", "LEVEL+10", "LEVEL+20", "LEVEL-10", "LEVEL-20", "BPM TAP",
    "DLY1 TAP", "DLY2 TAP", "MDLY TAP", "FX1D TAP", "FX2D TAP", "FX3D TAP", "TUNER", "MANUAL", "MANUAL/T", "TUNER/M",
    "INST1", "INST2", "INST3", "INST ALL", "1:NM MIX", "2:NM MIX", "3:NM MIX", "NORMAL", "1:ALT TN", "2:ALT TN",
    "3:ALT TN", "A:ALT TN", "1: 12STR", "2: 12STR", "3: 12STR", "ALL:12ST", "1:STR BD", "2:STR BD", "3:STR BD", "ALL:ST B",
    "1:LFO1", "1:LFO2", "1:LFO1&2", "2:LFO1", "2:LFO2", "2:LFO1&2", "3:LFO1", "3:LFO2", "3:LFO1&2", "ALL:LFO",
    "1:SEQ1", "1:SEQ2", "1:SEQ1&2", "2:SEQ1", "2:SEQ2", "2:SEQ1&2", "3:SEQ1", "3:SEQ2", "3:SEQ1&2", "ALL:SEQ",
    "1:SEQ1 T", "1:SEQ2 T", "1:S1&2 T", "2:SEQ1 T", "2:SEQ2 T", "2:S1&2 T", "3:SEQ1 T", "3:SEQ2 T", "3:S1&2 T", "ALL:SQ T",
    "1:S1 TBO", "1:S2 TBO", "1:1&2TBO", "2:S1 TBO", "2:S2 TBO", "2:1&2TBO", "3:S1 TBO", "3:S2 TBO", "3:1&2TBO", "ALL:TBO",
    "1:HOLD", "2:HOLD", "3:HOLD", "ALL:HOLD", "SYNC TRG", "1:PU UP", "2:PU UP", "3:PU UP", "1:PU DN", "2: PU DN",
    "3: PU DN", "1:AMP", "2:AMP", "3:AMP", "1:AMP SL", "2:AMP SL", "3:AMP SL", "FX1", "FX2", "FX3",
    "CMP", "DS", "DS SOLO", "AMP", "AMP SOLO", "EQ1", "EQ2", "NS", "DLY1", "DLY2",
    "MST DLY", "CHO", "REV", "S/R", "FX1 TRIG", "FX2 TRIG", "FX3 TRIG", "MDLY TRG", "DIV CH S", "MD START",
    "MMC PLAY",
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

QStringList SY1000_assign_titles = {
  {"CTL1", "CTL2", "CTL3", "CTL4", "CTL5", "CTL6", "BANK DN", "BANK UP", "NUM1", "NUM2", "NUM3", "NUM4",
   "MANUAL1", "MANUAL2", "MANUAL3", "MANUAL4", "GK SW1", "GK SW2"},
};

uint8_t SY1000_class::get_number_of_assigns()
{
    return SY1000_NUMBER_OF_CTL_FUNCTIONS + SY1000_NUMBER_OF_ASSIGNS;
}

QString SY1000_class::read_assign_name(uint8_t assign_no)
{
    if (assign_no < SY1000_NUMBER_OF_CTL_FUNCTIONS) return SY1000_assign_titles[assign_no];
    if (assign_no < SY1000_NUMBER_OF_CTL_FUNCTIONS + SY1000_NUMBER_OF_ASSIGNS)  return "ASSIGN " + QString::number(assign_no + 1 - SY1000_NUMBER_OF_CTL_FUNCTIONS);
     else return "?";
}

QString SY1000_class::read_assign_trigger(uint8_t trigger_no)
{
    if (trigger_no < SY1000_NUMBER_OF_CTL_FUNCTIONS) return SY1000_assign_titles[trigger_no];
      else if (trigger_no < 96) return "CC#" + QString::number(trigger_no);
    else return "-";
}

uint8_t SY1000_class::trigger_follow_assign(uint8_t assign_no)
{
    if (assign_no <= SY1000_NUMBER_OF_CTL_FUNCTIONS) return assign_no;
      else return assign_no + 21 - SY1000_NUMBER_OF_CTL_FUNCTIONS; // Default cc numbers are 21 and up
}

QString SY1000_class::get_patch_info(uint16_t number)
{
    uint16_t patch_no = (Device_patches[number][1] << 8) + Device_patches[number][2];
    QString line = number_format(patch_no);
    line.append("\t");
    for (int s = 0; s < 8; s++) {
        line.append("     scene ");
        line.append(QString::number(s + 1));
        line.append(": ");
        line.append(read_scene_name_from_buffer(number, s));
    }
    return line;
}

QString SY1000_class::read_scene_name_from_buffer(int number, uint8_t scene) const {
  QString lbl = "";
  int b = get_scene_index(scene) + SY1000_SCENE_NAME_BYTE;
  uint8_t last_char = 0;
  for (uint8_t c = 0; c < 8; c++) {
      if (Device_patches[number][b++] > 32) last_char = c;
  }
  b = get_scene_index(scene) + SY1000_SCENE_NAME_BYTE;
  for (uint8_t c = 0; c <= last_char; c++) {
    lbl.append(static_cast<char>(Device_patches[number][b++]));
  }
  return lbl;
}

void SY1000_class::store_scene_name_to_buffer(int number, uint8_t scene, QString lbl) {
  int b = get_scene_index(scene) + SY1000_SCENE_NAME_BYTE;
  uint8_t len = lbl.length();
  if (len > 8) len = 8;
  for (uint8_t c = 0; c < len; c++) Device_patches[number][b++] = lbl.at(c).toLatin1();
  for (uint8_t c = len; c < 8; c++) Device_patches[number][b++] = 0x20;
}

int SY1000_class::get_scene_index(uint8_t scene) const {
  //if (scene > 0) scene--;
    return (scene * SY1000_SCENE_SIZE) + SY1000_COMMON_DATA_SIZE;
}

uint8_t SY1000_class::supportPatchSaving()
{
    return 2;
}

void SY1000_class::readPatchData(int index, int patch_no, const QJsonObject &json)
{
    Device_patches[index][0] = my_device_number + 1;
    Device_patches[index][1] = patch_no >> 8;
    Device_patches[index][2] = patch_no & 0xFF;

    QJsonObject headerBlock = json["PatchHeader"].toObject();
    if (!headerBlock.isEmpty()) {
        for (int i = 3; i < 8; i++) {
            int value = headerBlock["Header data"+ QString::number(i)].toInt();
            Device_patches[index][i] = value;
        }
    }

    for (uint8_t s = 0; s < 8; s++ ) {
        QJsonObject sceneBlock = json["Scene" + QString::number(s + 1)].toObject();
        if (!sceneBlock.isEmpty()) {
            int b = get_scene_index(s);
            for (int i = 0; i < SY1000_SCENE_SIZE; i++) {
                int value = sceneBlock["Data"+ numConv(i)].toInt();
                Device_patches[index][b++] = value;
            }
        }
    }
}

void SY1000_class::writePatchData(int patch_no, QJsonObject &json) const
{
    QJsonObject headerBlock;
    for (int i = 0; i < 8; i++) {
        int value = Device_patches[patch_no][i];
        headerBlock["Header data"+ QString::number(i)] = value;
    }
    json["PatchHeader"] = headerBlock;

    QJsonObject sceneBlock;
    for (uint8_t s = 0; s < 8; s++ ) {
        sceneBlock["Name"] = read_scene_name_from_buffer(patch_no, s);
        int b = get_scene_index(s);
        for (int i = 0; i < SY1000_SCENE_SIZE; i++) {
            int value = Device_patches[patch_no][b++];
            sceneBlock["Data"+ numConv(i)] = value;
        }
        json["Scene" + QString::number(s + 1)] = sceneBlock;
    }
}
