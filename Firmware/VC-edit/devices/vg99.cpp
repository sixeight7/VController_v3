#include "vg99.h"
#include "VController/config.h"
#include "VController/leds.h"

void VG99_class::init()
{
    device_name = "VG99";
    full_device_name = "Roland VG-99";
    patch_min = VG99_PATCH_MIN;
    patch_max = VG99_PATCH_MAX;
    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 2; // Default value: red
    MIDI_channel = VG99_MIDI_CHANNEL; // Default value
    bank_number = 0; // Default value
    is_always_on = true; // Default value
    my_device_page1 = VG99_DEFAULT_PAGE1;  // Default value
    my_device_page2 = VG99_DEFAULT_PAGE2; // Default value
    my_device_page3 = VG99_DEFAULT_PAGE3; // Default value
    my_device_page4 = VG99_DEFAULT_PAGE4; // Default value

}

bool VG99_class::check_command_enabled(uint8_t cmd)
{
    switch (cmd) {
    case PATCH:
    case PARAMETER:
    case ASSIGN:
    case MUTE:
    case OPEN_PAGE_DEVICE:
    case OPEN_NEXT_PAGE_OF_DEVICE:
    case MASTER_EXP_PEDAL:
    case TOGGLE_EXP_PEDAL:
    //case SNAPSCENE:
    //case LOOPER:
        return true;
    }
    return false;
}

QString VG99_class::number_format(uint16_t patch_no)
{
   QString Output;
   if (patch_no > 199) Output =  "P";
     else Output =  "U";

     // Then add the patch number
     uint16_t number_plus_one = patch_no + 1;
     Output +=  QString::number(number_plus_one / 100) + QString::number((number_plus_one / 10) % 10) + QString::number(number_plus_one % 10);
   return Output;
}

// Parameter categories
#define VG99_CAT_NONE 0 // Some parameters cannot be read - should be in a category
#define VG99_CAT_GTR_B 1
#define VG99_CAT_FX_B 2
#define VG99_CAT_AMP_B 3
#define VG99_CAT_MIX_B 4
#define VG99_CAT_GTR_A 5
#define VG99_CAT_FX_A 6
#define VG99_CAT_AMP_A 7
#define VG99_CAT_MIX_A 8
#define VG99_CAT_ALT_TUNING 9
#define VG99_CAT_POLY_FX 10
#define VG99_CAT_DLY_RVB 11
#define VG99_CAT_SYSTEM 12
#define VG99_NUMBER_OF_FX_CATEGORIES 12

struct VG99_parameter_category_struct {
  QString Name; // The name for the label
};

QVector<VG99_parameter_category_struct> VG99_parameter_category = {
  { "COSM GTR B" },
  { "FX B" },
  { "AMP B" },
  { "MIX B" },
  { "COSM GTR A" },
  { "FX A" },
  { "AMP A" },
  { "MIX A" },
  { "ALT. TUNING" },
  { "POLY FX" },
  { "DLY/RVB" },
  { "SYSTEM SETTINGS" }
};

struct VG99_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  uint16_t Address; // The address of the parameter
  uint8_t NumVals; // The number of values this parameter may have
  QString Name; // The name for the label
  uint16_t Sublist; // Which sublist to read for the FX or amp type
  uint8_t Colour; // The colour for this effect.
  uint8_t Category; // The category of this effect
};

//typedef struct stomper Stomper;
#define VG99_FX_COLOUR 255 // Just a colour number to pick the colour from the VG99_FX_colours table
#define VG99_FX_TYPE_COLOUR 254 //Another number for the MFX type
#define VG99_POLYFX_COLOUR 253 // Just a colour number to pick the colour from the VG99_POLY_FX_colours table
#define VG99_POLYFX_TYPE_COLOUR 252 //Another number for the MOD type
#define SUBLIST_FROM_BYTE2 32768 //Bit 16 set high means read sublist frm byte 2
#define SHOW_NUMBER 31767 // Special number for showing the number (used for gain/volume/etc) - set in sublist
#define SHOW_DOUBLE_NUMBER 31766 // Show number times 2 (used for patch volume/etc) - set in sublist
#define SHOW_PAN 31765 // Special number for showing the pan- set in sublist

QVector<VG99_parameter_struct> VG99_parameters = {
  //{ // part 0: 0000 - 1000 System
  {0x0017, 12, "KEY", 161, FX_PITCH_TYPE, VG99_CAT_SYSTEM},
  {0x0015, 250, "BPM", 0, FX_DELAY_TYPE, VG99_CAT_NONE},
  {0x0024, 2, "FC AMP CTL1", 0, FX_AMP_TYPE, VG99_CAT_SYSTEM},
  {0x0025, 2, "FC AMP CTL2", 0, FX_AMP_TYPE, VG99_CAT_SYSTEM},
  {0x0D1E, 4, "D BM SELECT", 0, FX_PITCH_TYPE, VG99_CAT_SYSTEM},
  {0x0D1F, 2, "D BM PITCH TYP", 0, FX_PITCH_TYPE, VG99_CAT_SYSTEM},
  {0x0D20, 3, "D BM T-ARM CH", 0, FX_PITCH_TYPE, VG99_CAT_SYSTEM},
  {0x0D21, 4, "D BM T-ARM TYP", 0, FX_PITCH_TYPE, VG99_CAT_SYSTEM},
  {0x0D26, 2, "D BM FREEZE CH", 0, FX_DEFAULT_TYPE, VG99_CAT_SYSTEM},
  {0x0D29, 2, "D BM FRZ(A) LVL", 0, FX_DEFAULT_TYPE, VG99_CAT_SYSTEM},
  {0x0D2D, 2, "D BM FRZ(B) LVL", 0, FX_DEFAULT_TYPE, VG99_CAT_SYSTEM},
  {0x0D2F, 2, "D BM FILTER CH", 0, FX_FILTER_TYPE, VG99_CAT_SYSTEM},
  {0x0D30, 2, "D BM FLTR TYPE", 0, FX_FILTER_TYPE, VG99_CAT_SYSTEM},
  {0x0D34, 2, "D BM FILTR LVL", 0, FX_FILTER_TYPE, VG99_CAT_SYSTEM},
  {0x0D35, 2, "RIBBON SELECT", 0, FX_DEFAULT_TYPE, VG99_CAT_SYSTEM},
  {0x0D36, 2, "RBBN T-ARM CH", 0, FX_PITCH_TYPE, VG99_CAT_SYSTEM},
  {0x0D37, 2, "RBBN T-ARM TYPE", 0, FX_PITCH_TYPE, VG99_CAT_SYSTEM},
  {0x0D3C, 2, "RBBN FILTER CH", 0, FX_FILTER_TYPE, VG99_CAT_SYSTEM},
  {0x0D3D, 2, "RBBN FILTER TYP", 0, FX_FILTER_TYPE, VG99_CAT_SYSTEM},
  {0x0D41, 2, "RBBN FILTER LVL", 0, FX_FILTER_TYPE, VG99_CAT_SYSTEM},
  //},

  //{ // part 1: 1000 - 2000 Alt tuning parameters
  {0x1001, 2, "[A]TUN", 144 | SUBLIST_FROM_BYTE2, FX_PITCH_TYPE, VG99_CAT_ALT_TUNING},
  {0x1002, 13, "[A]TU TYP", 144, FX_PITCH_TYPE, VG99_CAT_ALT_TUNING},
  {0x1007, 2, "[A]BEND SW", 0, FX_PITCH_TYPE, VG99_CAT_ALT_TUNING},
  {0x1004, 2, "[A]12STRING SW", 0, FX_PITCH_TYPE, VG99_CAT_ALT_TUNING},
  {0x1003, 2, "[A]DETUNE SW", 0, FX_PITCH_TYPE, VG99_CAT_ALT_TUNING},
  {0x1005, 2, "[A]HM", 65 | SUBLIST_FROM_BYTE2, FX_PITCH_TYPE, VG99_CAT_ALT_TUNING},
  {0x1006, 30, "[A]HARMO", 65, FX_PITCH_TYPE, VG99_CAT_ALT_TUNING},
  {0x1009, 2, "[B]TUN", 144 | SUBLIST_FROM_BYTE2, FX_PITCH_TYPE, VG99_CAT_ALT_TUNING},
  {0x100A, 13, "[B]TU TYP", 144, FX_PITCH_TYPE, VG99_CAT_ALT_TUNING},
  {0x100F, 2, "[B]BEND SW", 0, FX_PITCH_TYPE, VG99_CAT_ALT_TUNING},
  {0x100C, 2, "[B]12STRING SW", 0, FX_PITCH_TYPE, VG99_CAT_ALT_TUNING},
  {0x100B, 2, "[B]DETUNE SW", 0, FX_PITCH_TYPE, VG99_CAT_ALT_TUNING},
  {0x100D, 2, "[B]HM SW", 65 | SUBLIST_FROM_BYTE2, FX_PITCH_TYPE, VG99_CAT_ALT_TUNING},
  {0x100E, 30, "[B]HARMO", 65, FX_PITCH_TYPE, VG99_CAT_ALT_TUNING},
  //},

  //{ // part 2: 2000 - 3000 Common parameters
  {0x2019, 2, "[A]MIX SW", 0, FX_DEFAULT_TYPE, VG99_CAT_MIX_A},
  {0x2014, 101, "[A]MIXER PAN", SHOW_PAN, FX_DEFAULT_TYPE, VG99_CAT_MIX_A},
  {0x2015, 101, "[A]MIXER LVL", SHOW_NUMBER, FX_DEFAULT_TYPE, VG99_CAT_MIX_A},
  {0x2000, 101, "PATCH LEVEL", SHOW_DOUBLE_NUMBER, FX_DEFAULT_TYPE, VG99_CAT_MIX_A},
  {0x2007, 2, "DYNA SW", 0, FX_DEFAULT_TYPE, VG99_CAT_MIX_A},
  {0x2008, 2, "DYNA TYPE", 0, FX_DEFAULT_TYPE, VG99_CAT_MIX_A},
  {0x202F, 2, "TOTAL EQ SW", 0, FX_DEFAULT_TYPE, VG99_CAT_MIX_A},
  {0x2013, 101, "A/B BAL", SHOW_PAN, FX_DEFAULT_TYPE, VG99_CAT_MIX_A},
  {0x201F, 2, "[B]MIX SW", 0, FX_DEFAULT_TYPE, VG99_CAT_MIX_B},
  {0x201A, 101, "[B]MIXER PAN", SHOW_PAN, FX_DEFAULT_TYPE, VG99_CAT_MIX_B},
  {0x201B, 101, "[B]MIXER LVL", SHOW_NUMBER, FX_DEFAULT_TYPE, VG99_CAT_MIX_B},
  {0x2000, 101, "PATCH LEVEL", SHOW_DOUBLE_NUMBER, FX_DEFAULT_TYPE, VG99_CAT_MIX_B},
  {0x2007, 2, "DYNA SW", 0, FX_DEFAULT_TYPE, VG99_CAT_MIX_B},
  {0x2008, 2, "DYNA TYPE", 0, FX_DEFAULT_TYPE, VG99_CAT_MIX_B},
  {0x202F, 2, "TOTAL EQ SW", 0, FX_DEFAULT_TYPE, VG99_CAT_MIX_B},
  {0x2013, 101, "A/B BAL", SHOW_PAN, FX_DEFAULT_TYPE, VG99_CAT_MIX_B},
  {0x2020, 2, "D/R RVB SW", 0, FX_REVERB_TYPE, VG99_CAT_DLY_RVB},
  {0x2021, 5, "D/R RVB TP", 241, FX_REVERB_TYPE, VG99_CAT_DLY_RVB},
  {0x2022, 2, "D/R REVRB TIME", 0, FX_REVERB_TYPE, VG99_CAT_DLY_RVB},
  {0x2023, 2, "D/R RVRB PREDY", 0, FX_REVERB_TYPE, VG99_CAT_DLY_RVB},
  {0x2024, 2, "D/R RVRB LWCUT", 0, FX_REVERB_TYPE, VG99_CAT_DLY_RVB},
  {0x2025, 2, "D/R RVRB HICUT", 0, FX_REVERB_TYPE, VG99_CAT_DLY_RVB},
  {0x2026, 2, "D/R REVRB DENS", 0, FX_REVERB_TYPE, VG99_CAT_DLY_RVB},
  {0x2027, 101, "D/R RVB LVL", SHOW_NUMBER, FX_REVERB_TYPE, VG99_CAT_DLY_RVB},
  {0x2028, 2, "D/R DELAY SW", 0, FX_DELAY_TYPE, VG99_CAT_DLY_RVB},
  {0x2029, 2, "D/R DELAY TIME", 0, FX_DELAY_TYPE, VG99_CAT_DLY_RVB},
  {0x202B, 2, "D/R DLAY FDBCK", 0, FX_DELAY_TYPE, VG99_CAT_DLY_RVB},
  {0x202C, 2, "D/R DLAY HICUT", 0, FX_DELAY_TYPE, VG99_CAT_DLY_RVB},
  {0x202D, 121, "D/R DLY LVL", SHOW_NUMBER, FX_DELAY_TYPE, VG99_CAT_DLY_RVB},
  //},

  //{ // part 3: 3000 - 4000 Guitar parameters
  {0x3000, 2, "[A]COSM GTR SW", 0, FX_GTR_TYPE, VG99_CAT_GTR_A},
  {0x3001, 4, "[A]MODEL", 173, FX_GTR_TYPE, VG99_CAT_GTR_A},
  {0x301B, 10, "[A]E.GTR", 177, FX_GTR_TYPE, VG99_CAT_GTR_A},
  {0x301D, 5, "[A]PU SEL", 187, FX_GTR_TYPE, VG99_CAT_GTR_A},
  {0x301F, 101, "[A]E.GTR VOL", SHOW_NUMBER, FX_GTR_TYPE, VG99_CAT_GTR_A},
  {0x3022, 101, "[A]EGTR TONE", SHOW_NUMBER, FX_FILTER_TYPE, VG99_CAT_GTR_A},
  {0x3045, 6, "[A]AC TYPE", 193, FX_GTR_TYPE, VG99_CAT_GTR_A},
  {0x3046, 5, "[A]BODY", 199, FX_GTR_TYPE, VG99_CAT_GTR_A},
  {0x3069, 2, "[A]BASS TYPE", 204, FX_GTR_TYPE, VG99_CAT_GTR_A},
  {0x3400, 11, "[A]SYNTH T", 206, FX_GTR_TYPE, VG99_CAT_GTR_A},
  {0x3002, 2, "[A]GTR EQ SW", 0, FX_FILTER_TYPE, VG99_CAT_GTR_A},
  {0x3018, 101, "[A]COSM LVL", SHOW_NUMBER, FX_GTR_TYPE, VG99_CAT_GTR_A},
  {0x301A, 101, "[A]NPU LEVEL", SHOW_NUMBER, FX_GTR_TYPE, VG99_CAT_GTR_A},
  {0x3457, 2, "[A]NS SW", 0, FX_GTR_TYPE, VG99_CAT_GTR_A},
  {0x3800, 2, "[B]COSM GTR SW", 0, FX_GTR_TYPE, VG99_CAT_GTR_B},
  {0x3801, 4, "[B]MODEL", 173, FX_GTR_TYPE, VG99_CAT_GTR_B},
  {0x381B, 10, "[B]E.GTR", 177, FX_GTR_TYPE, VG99_CAT_GTR_B},
  {0x381D, 5, "[B]PU SEL", 187, FX_GTR_TYPE, VG99_CAT_GTR_B},
  {0x381F, 101, "[B]E.GTR VOL", SHOW_NUMBER, FX_GTR_TYPE, VG99_CAT_GTR_B},
  {0x3822, 101, "[B]EGTR TONE", SHOW_NUMBER, FX_FILTER_TYPE, VG99_CAT_GTR_B},
  {0x3845, 6, "[B]AC TYPE", 193, FX_GTR_TYPE, VG99_CAT_GTR_B},
  {0x3846, 5, "[B]BODY", 199, FX_GTR_TYPE, VG99_CAT_GTR_B},
  {0x3869, 2, "[B]BASS TYPE", 204, FX_GTR_TYPE, VG99_CAT_GTR_B},
  {0x3C00, 11, "[B]SYNTH T", 206, FX_GTR_TYPE, VG99_CAT_GTR_B},
  {0x3802, 2, "[B]GTR EQ SW", 0, FX_FILTER_TYPE, VG99_CAT_GTR_B},
  {0x3818, 101, "[B]COSM LVL", SHOW_NUMBER, FX_GTR_TYPE, VG99_CAT_GTR_B},
  {0x381A, 101, "[B]NPU LEVEL", SHOW_NUMBER, FX_GTR_TYPE, VG99_CAT_GTR_B},
  {0x3C57, 2, "[B]NS SW", 0, FX_FILTER_TYPE, VG99_CAT_GTR_B},
  //},

  //{ // part 4: 4000 - 5000 Poly FX
  {0x4001, 2, "POLYFX", 157 | SUBLIST_FROM_BYTE2, VG99_POLYFX_COLOUR, VG99_CAT_POLY_FX},
  {0x4002, 4, "POLY TYPE", 157, VG99_POLYFX_TYPE_COLOUR, VG99_CAT_POLY_FX},
  {0x4000, 2, "POLY FX CHAN", 261, FX_FILTER_TYPE, VG99_CAT_POLY_FX},
  {0x4009, 101, "POLY COMP LEVEL", SHOW_NUMBER, FX_DYNAMICS_TYPE, VG99_CAT_POLY_FX},
  {0x400A, 101, "POLY COMP BAL", SHOW_NUMBER, FX_DYNAMICS_TYPE, VG99_CAT_POLY_FX},
  {0x400B, 5, "POLY DIST MODE", 263, FX_DIST_TYPE, VG99_CAT_POLY_FX},
  {0x400C, 101, "POLY DIST DRIVE", SHOW_NUMBER, FX_DIST_TYPE, VG99_CAT_POLY_FX},
  {0x400D, 10, "POLY D HIGH CUT", 268, FX_DIST_TYPE, VG99_CAT_POLY_FX},
  {0x400E, 101, "POLY D POLY BAL", SHOW_NUMBER, FX_DIST_TYPE, VG99_CAT_POLY_FX},
  {0x400F, 101, "POLY D DRIVE BAL", SHOW_NUMBER, FX_DIST_TYPE, VG99_CAT_POLY_FX},
  {0x4010, 101, "PDIST LVL", SHOW_NUMBER, FX_DIST_TYPE, VG99_CAT_POLY_FX},
  {0x402F, 101, "POLY SG RISETIME", SHOW_NUMBER, FX_MODULATE_TYPE, VG99_CAT_POLY_FX},
  {0x4030, 101, "POLY SG SENS", SHOW_NUMBER, FX_MODULATE_TYPE, VG99_CAT_POLY_FX},
  //},

  //{ // part 5: 5000 - 6000 FX and amps chain A
  {0x502B, 2, "[A]COMP SW", 0, FX_FILTER_TYPE, VG99_CAT_FX_A},
  {0x502C, 2, "[A]COMP TP", 217, FX_FILTER_TYPE, VG99_CAT_FX_A},
  {0x5033, 2, "[A]OD", 1 | SUBLIST_FROM_BYTE2, FX_DIST_TYPE, VG99_CAT_FX_A},
  {0x5034, 31, "[A]OD T", 1, FX_DIST_TYPE, VG99_CAT_FX_A},
  {0x503F, 2, "[A]WAH SW", 219 | SUBLIST_FROM_BYTE2, FX_FILTER_TYPE, VG99_CAT_FX_A},
  {0x5040, 7, "[A]WAH TP", 219, FX_FILTER_TYPE, VG99_CAT_FX_A}, // Parameter number: 155
  {0x5041, 101, "[A]WAH POS", SHOW_NUMBER, FX_FILTER_TYPE, VG99_CAT_FX_A},
  {0x5042, 101, "[A]WAH LVL", SHOW_NUMBER, FX_FILTER_TYPE, VG99_CAT_FX_A},
  {0x5048, 2, "[A]EQ SW", 0, FX_FILTER_TYPE, VG99_CAT_FX_A},
  {0x5054, 2, "[A]DLY SW", 226 | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE, VG99_CAT_FX_A},
  {0x5055, 11, "[A]DLY TP", 226, FX_DELAY_TYPE, VG99_CAT_FX_A},
  {0x506B, 101, "[A]DLY LVL", SHOW_NUMBER, FX_DELAY_TYPE, VG99_CAT_FX_A},
  {0x506D, 2, "[A]CHOR SW", 238 | SUBLIST_FROM_BYTE2, FX_MODULATE_TYPE, VG99_CAT_FX_A},
  {0x506E, 3, "[A]CHOR MD", 238, FX_MODULATE_TYPE, VG99_CAT_FX_A}, // Parameter number: 160
  {0x5074, 101, "[A]CHOR LVL", SHOW_NUMBER, FX_MODULATE_TYPE, VG99_CAT_FX_A},
  {0x5075, 2, "[A]RVB SW", 241 | SUBLIST_FROM_BYTE2, FX_REVERB_TYPE, VG99_CAT_FX_A},
  {0x5076, 7, "[A]RVB TP", 241, FX_REVERB_TYPE, VG99_CAT_FX_A},
  {0x507C, 101, "[A]RVB LVL", SHOW_NUMBER, FX_REVERB_TYPE, VG99_CAT_FX_A},
  {0x5400, 2, "[A]M1", 32 | SUBLIST_FROM_BYTE2, VG99_FX_COLOUR, VG99_CAT_FX_A}, // Check VG99_mod_type table (2)
  {0x5401, 33, "[A]M1 TP", 32, VG99_FX_TYPE_COLOUR, VG99_CAT_FX_A},
  {0x5800, 2, "[A]M2", 32 | SUBLIST_FROM_BYTE2, VG99_FX_COLOUR, VG99_CAT_FX_A}, // Check VG99_mod_type table (2)
  {0x5801, 33, "[A]M2 TP", 32, VG99_FX_TYPE_COLOUR, VG99_CAT_FX_A},
  {0x507E, 2, "[A]NS SW", 0, FX_FILTER_TYPE, VG99_CAT_FX_A},
  {0x5102, 101, "[A]FOOT VOL", SHOW_NUMBER, FX_DEFAULT_TYPE, VG99_CAT_FX_A},

  {0x500D, 2, "[A]AMP", 95 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE, VG99_CAT_AMP_A},
  {0x500E, 49, "[A]AMP TP", 95, FX_AMP_TYPE, VG99_CAT_AMP_A},
  {0x500F, 121, "[A]AMP GAIN", SHOW_NUMBER, FX_AMP_TYPE, VG99_CAT_AMP_A},
  {0x5014, 101, "[A]AMP LEVEL", SHOW_NUMBER, FX_AMP_TYPE, VG99_CAT_AMP_A},
  {0x5010, 101, "[A]AMP BASS", SHOW_NUMBER, FX_AMP_TYPE, VG99_CAT_AMP_A},
  {0x5011, 101, "[A]AMP MID", SHOW_NUMBER, FX_AMP_TYPE, VG99_CAT_AMP_A},
  {0x5012, 101, "[A]AMP TREB", SHOW_NUMBER, FX_AMP_TYPE, VG99_CAT_AMP_A},
  {0x5013, 101, "[A]AMP PRES", SHOW_NUMBER, FX_AMP_TYPE, VG99_CAT_AMP_A},
  {0x5015, 2, "[A]AMP BRIGHT", 0, FX_AMP_TYPE, VG99_CAT_AMP_A},
  {0x5016, 3, "[A]GAIN SW", 258, FX_AMP_TYPE, VG99_CAT_AMP_A},
  {0x5017, 2, "[A]AMP(SOLO) SW", 0, FX_AMP_TYPE, VG99_CAT_AMP_A},
  {0x5019, 10, "[A]AMP SP", 248, FX_AMP_TYPE, VG99_CAT_AMP_A},
  //},

  //{ // part 6: 6000 - 7000 FX and amps chain B
  {0x602B, 2, "[B]COMP SW", 0, FX_FILTER_TYPE, VG99_CAT_FX_B},
  {0x602C, 2, "[B]COMP TP", 217, FX_FILTER_TYPE, VG99_CAT_FX_B},
  {0x6033, 2, "[B]OD", 1 | SUBLIST_FROM_BYTE2, FX_DIST_TYPE, VG99_CAT_FX_B}, // Check VG99_odds_type table (1)
  {0x6034, 31, "[B]OD T", 1, FX_DIST_TYPE, VG99_CAT_FX_B},
  {0x603F, 2, "[B]WAH SW", 219 | SUBLIST_FROM_BYTE2, FX_FILTER_TYPE, VG99_CAT_FX_B},
  {0x6040, 7, "[B]WAH TP", 219, FX_FILTER_TYPE, VG99_CAT_FX_B},
  {0x6041, 101, "[B]WAH POS", SHOW_NUMBER, FX_FILTER_TYPE, VG99_CAT_FX_B},
  {0x6042, 101, "[B]WAH LVL", SHOW_NUMBER, FX_FILTER_TYPE, VG99_CAT_FX_B},
  {0x6048, 2, "[B]EQ SW", 0, FX_FILTER_TYPE, VG99_CAT_FX_B},
  {0x6054, 2, "[B]DLY SW", 226 | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE, VG99_CAT_FX_B},
  {0x6055, 11, "[B]DLY TP", 226, FX_DELAY_TYPE, VG99_CAT_FX_B},
  {0x606B, 101, "[B]DLY LVL", SHOW_NUMBER, FX_DELAY_TYPE, VG99_CAT_FX_B},
  {0x606D, 2, "[B]CHOR SW", 238 | SUBLIST_FROM_BYTE2, FX_MODULATE_TYPE, VG99_CAT_FX_B},
  {0x606E, 3, "[B]CHOR M", 238, FX_MODULATE_TYPE, VG99_CAT_FX_B},
  {0x6074, 101, "[B]CHOR LVL", SHOW_NUMBER, FX_MODULATE_TYPE, VG99_CAT_FX_B},
  {0x6075, 2, "[B]RVB SW", 241 | SUBLIST_FROM_BYTE2, FX_REVERB_TYPE, VG99_CAT_FX_B},
  {0x6076, 7, "[B]RVB TP", 241, FX_REVERB_TYPE, VG99_CAT_FX_B},
  {0x607C, 101, "[B]RVB LVL", SHOW_NUMBER, FX_REVERB_TYPE, VG99_CAT_FX_B},
  {0x6400, 2, "[B]M1", 32 | SUBLIST_FROM_BYTE2, VG99_FX_COLOUR, VG99_CAT_FX_B}, // Check VG99_mod_type table (2)
  {0x6401, 33, "[B]M1 TYPE", 32, VG99_FX_TYPE_COLOUR, VG99_CAT_FX_B},
  {0x6800, 2, "[B]M2", 32 | SUBLIST_FROM_BYTE2, VG99_FX_COLOUR, VG99_CAT_FX_B}, // Check VG99_mod_type table (2)
  {0x6801, 33, "[B]M2 TYPE", 32, VG99_FX_TYPE_COLOUR, VG99_CAT_FX_B},
  {0x607E, 2, "[B]NS SW", 0, FX_FILTER_TYPE, VG99_CAT_FX_B},
  {0x6102, 101, "[B]FOOT VOL", SHOW_NUMBER, FX_DEFAULT_TYPE, VG99_CAT_FX_B},

  {0x600D, 2, "[B]AMP", 95 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE, VG99_CAT_AMP_B}, // Sublist amps
  {0x600E, 49, "[B]AMP TP", 95, FX_AMP_TYPE, VG99_CAT_AMP_B},
  {0x600F, 121, "[B]AMP GAIN", SHOW_NUMBER, FX_AMP_TYPE, VG99_CAT_AMP_B},
  {0x6014, 101, "[B]AMP LEVEL", SHOW_NUMBER, FX_AMP_TYPE, VG99_CAT_AMP_B},
  {0x6010, 101, "[B]AMP BASS", SHOW_NUMBER, FX_AMP_TYPE, VG99_CAT_AMP_B},
  {0x6011, 101, "[B]AMP MID", SHOW_NUMBER, FX_AMP_TYPE, VG99_CAT_AMP_B},
  {0x6012, 101, "[B]AMP TREB", SHOW_NUMBER, FX_AMP_TYPE, VG99_CAT_AMP_B},
  {0x6013, 101, "[B]AMP PRES", SHOW_NUMBER, FX_AMP_TYPE, VG99_CAT_AMP_B},
  {0x6015, 2, "[B]AMP BRIGHT", 0, FX_AMP_TYPE, VG99_CAT_AMP_B},
  {0x6016, 3, "[B]GAIN SW", 258, FX_AMP_TYPE, VG99_CAT_AMP_B},
  {0x6017, 2, "[B]AMP(SOLO) SW", 0, FX_AMP_TYPE, VG99_CAT_AMP_B},
  {0x6019, 10, "[B]AMP SP", 248, FX_AMP_TYPE, VG99_CAT_AMP_B},
  //},

  //{ // part 7: 7000 - 8000 Special functions - these work only from an assign - not from a parameter - update VG99_NON_PARAMETER_TARGETS below when adding parameters here
  {0x7600, 2, "[A]BEND", 0, FX_PITCH_TYPE, VG99_CAT_NONE},
  {0x7601, 2, "[B]BEND", 0, FX_PITCH_TYPE, VG99_CAT_NONE},
  {0x7602, 2, "DB T-ARM CONTROL", 0, FX_PITCH_TYPE, VG99_CAT_NONE},
  {0x7603, 2, "DB T-ARM SW", 0, FX_PITCH_TYPE, VG99_CAT_NONE},
  {0x7604, 2, "DB FREEZE SW", 0, FX_DEFAULT_TYPE, VG99_CAT_NONE},
  {0x7606, 2, "DB FILTER CONTRL", 0, FX_FILTER_TYPE, VG99_CAT_NONE},
  {0x7607, 2, "DB FILTER SW", 0, FX_FILTER_TYPE, VG99_CAT_NONE},
  {0x7608, 2, "RB T-ARM CONTROL", 0, FX_PITCH_TYPE, VG99_CAT_NONE},
  {0x7609, 2, "RB T-ARM SW", 0, FX_PITCH_TYPE, VG99_CAT_NONE},
  {0x760A, 2, "RB FILTER CONTRL", 0, FX_FILTER_TYPE, VG99_CAT_NONE},
  {0x760B, 2, "RB FILTER SW", 0, FX_FILTER_TYPE, VG99_CAT_NONE},
  {0x760C, 2, "[A]FX DLY REC", 0, FX_DELAY_TYPE, VG99_CAT_NONE},
  {0x760D, 2, "[A]FX DLY STOP", 0, FX_DELAY_TYPE, VG99_CAT_NONE},
  {0x760E, 2, "[B]FX DLY REC", 0, FX_DELAY_TYPE, VG99_CAT_NONE},
  {0x760F, 2, "[B]FX DLY STOP", 0, FX_DELAY_TYPE, VG99_CAT_NONE},
  {0x7611, 2, "BPM TAP", 0, FX_DELAY_TYPE, VG99_CAT_NONE},
  {0x7610, 2, "V-LINK SW", 0, FX_AMP_TYPE, VG99_CAT_NONE},
  {0x7F7F, 0, "OFF", 0, 0, VG99_CAT_NONE},
  //}
};

#define VG99_NON_PARAMETER_TARGETS 18

const uint16_t VG99_NUMBER_OF_PARAMETERS = VG99_parameters.size();

QStringList VG99_sublists = {
  // Sublist 1 - 31 from the "Control assign target" table on page 57 of the VG99 MIDI impementation guide
  "BOOST", "BLUES", "CRUNCH", "NATURAL", "TURBO", "FAT OD", "OD-1", "TSCREAM", "WARM OD", "DIST",
  "MILD DS", "DRIVE", "RAT", "GUV DS", "DST+", "SOLID", "MID DS", "STACK", "MODERN", "POWER", "R-MAN",
  "METAL", "HVY MTL", "LEAD", "LOUD", "SHARP", "MECHA", "60 FUZZ", "OCTFUZZ", "BIGMUFF", "CUSTOM",

  // Sublist 32 - 64 from the "FX mod type" table on page 71 of the VG99 MIDI impementation guide
  "COMPRSR", "LIMITER", "T. WAH", "AUTOWAH", "T_WAH", "---", "TREMOLO", "PHASER", //00 - 07
  "FLANGER", "PAN", "VIB", "UNI-V", "RINGMOD", "SLOW GR", "DEFRET", "", //08 - 0F
  "FEEDBKR", "ANTI FB", "HUMANZR", "SLICER", "---", "SUB EQ", "HARMO", "PITCH S", //10 - 17
  "P. BEND", "OCTAVE", "ROTARY", "2x2 CHS", "---", "---", "---", "---", //18 - 1F
  "S DELAY",

  // Sublist 65 - 94 from the "Harmony" table on page 56 of the VG99 MIDI impementation guide
  "-2oct", "-14th", "-13th", "-12th", "-11th", "-10th", "-9th",
  "-1oct", "-7th", "-6th", "-5th", "-4th", "-3rd", "-2nd", "TONIC",
  "+2nd", "+3rd", "+4th", "+5th", "+6th", "+7th", "+1oct", "+9th", "+10th", "+11th",
  "+12th", "+13th", "+14th", "+2oct", "USER",

  // Sublist 95 - 143 from the "COSM AMP" table on page 71 of the VG99 MIDI impementation guide
  "JC-120", "JC WRM", "JC JZZ", "JC FLL", "JC BRT", "CL TWN", "PRO CR", "TWEED", "WRM CR", "CRUNCH",
  "BLUES", "WILD C", "C STCK", "VXDRIV", "VXLEAD", "VXCLN", "MTCH D", "MTCH F", "MTCH L", "BGLEAD",
  "BGDRIV", "BRHYTM", "SMOOTH", "BGMILD", "MS1959", "MS1959", "MS1959", "MS HI", "MS PWR", "RF-CLN",
  "RF-RAW", "RF-VT1", "RF-MN1", "RF-VT2", "RF-MN1", "T-CLN", "T-CNCH", "T-LEAD", "T-EDGE", "SLDANO",
  "HI-DRV", "HI-LD", "HI-HVY", "5150", "MODERN", "M LEAD", "CUSTOM", "BASS V", "BASS M",

  // Sublist 144 - 156 from page 17 of the VG99 MIDI impementation guide
  "OPEN-D", "OPEN-E", "OPEN-G", "OPEN-A", "DROP-D", "D-MODAL", "-1 STEP", "-2 STEP", "BARITON", "NASHVL", "-1 OCT", "+1 OCT", "USER",

  // Sublist 157 - 160 for poly FX
  "COMPR", "DISTORT", "OCTAVE", "SLOW GR",

  // Sublist 161 - 172 for key
  "C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B",

  // Sublist 173 - 176 for Modeling type
  "E.GTR", "AC", "BASS", "SYNTH",

  // Sublist 177 - 186 for E.GTR type
  "CLA-ST", "MOD-ST", "TE", "LP", "P-90", "LIPS", "RICK", "335", "L4", "VAR",

  // Sublist 187 - 192: Pickup position
  "REAR", "R+C", "CENTER", "C+F", "FRONT", "ALL",

  // Sublist 193 - 198: AC. type
  "STEEL", "NYLON", "SITAR", "BANJO", "RESO", "VARI",

  // Sublist 199 - 203: Ac body type
  "MA28", "TRP-0", "GB45", "GB SML", "GLD40",

  //Sublist 204,205: Bass Type
  "JB", "PB",

  // Sublist 206 - 216: Synth type
  "GR-300", "BOWED", "DUAL", "FBASS", "PIPE", "SOLO", "PWM", "CRYSTL", "ORGAN", "BRASS", "WAVE",

  // Sublist 217, 218: Comp type
  "COMP", "LIMITR",

  // Sublist 219 - 225: Wah type
  "CRY", "VO", "FAT", "LIGHT", "7STR", "RESO", "CUSTOM",

  // Sublist 226 - 237: Delay type
  "SINGLE", "PAN", "STEREO", "DUAL-S", "DUAL-P", "DU L/R", "REVRSE", "ANALOG", "TAPE", "WARP", "MOD", "HOLD",

  // Sublist 238 -240: Chorus type
  "MONO", "ST1", "ST2",

  // Sublist 241 - 247: Reverb Type
  "AMB", "ROOM", "HALL1", "HALL2", "PLATE", "SPRING", "MOD",

  // Sublist 248 - 257: Speaker type
  "OFF", "ORIG", "1x8\"", "1x10\"", "1x12\"", "2x12\"", "4x10\"", "4x12\"", "8x12\"", "CUSTOM",

  // Sublist 258 - 260: Amp gain switch
  "LOW", "MID", "HIGH",

  // Sublist 261 - 262: Poly FX channel
  "CH A", "CH B",

   // Sublist 263 - 267: Poly DIST mode
   "CLA OD", "TurboOD", "DS1", "DS2", "FUZZ",

   // Sublist 268 - 277: Poly DIST high cut
   "700 Hz", "1.0 kHz", "1.4 kHz", "2.0 kHz", "3.0 kHz", "4.0 kHz", "6,0 kHz", "8.0 kHz", "11 kHz", "FLAT",
};

const uint16_t VG99_SIZE_OF_SUBLIST = VG99_sublists.size();

QString VG99_class::read_parameter_name(uint16_t par_no)
{
    if (par_no < number_of_parameters())  return VG99_parameters[par_no].Name;
      else return "?";
}

QString VG99_class::read_parameter_state(uint16_t par_no, uint8_t value)
{
    if (par_no < number_of_parameters())  {
        uint16_t my_sublist = VG99_parameters[par_no].Sublist;
        if ((my_sublist > 0) && !(my_sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
          switch (my_sublist) {
            case SHOW_NUMBER:
              return QString::number(value);
              break;
            case SHOW_DOUBLE_NUMBER:
              return QString::number(value * 2); //Patch level is displayed double
              break;
            case SHOW_PAN:
              if (value < 50) return "L" + QString::number(50 - value);
              if (value == 50) return "C";
              if (value > 50) return "R" + QString::number(value - 50);
              break;
            default:
              return VG99_sublists[my_sublist + value - 1];
              break;
          }
        }
        else if (value == 1) return "ON";
        else return "OFF";
      }
      return "?";
}

uint16_t VG99_class::number_of_parameters()
{
    return VG99_NUMBER_OF_PARAMETERS - VG99_NON_PARAMETER_TARGETS;
}

uint8_t VG99_class::max_value(uint16_t par_no)
{
    if (par_no < VG99_NUMBER_OF_PARAMETERS) return VG99_parameters[par_no].NumVals - 1;
    else return 0;
}

struct VG99_assign_struct {
  QString Title;
  QString Short_title;
  uint32_t Address;
};

QVector<VG99_assign_struct> VG99_assigns = {
    {"FC300 CTL1", "FC 1", 0x60000600},
    {"FC300 CTL2", "FC 2", 0x60000614},
    {"FC300 CTL3", "FC 3", 0x60000628},
    {"FC300 CTL4", "FC 4", 0x6000063C},
    {"FC300 CTL5", "FC 5", 0x60000650},
    {"FC300 CTL6", "FC 6", 0x60000664},
    {"FC300 CTL7", "FC 7", 0x60000678},
    {"FC300 CTL8", "FC 8", 0x6000070C},
    {"FC300 EXP1", "EXP1", 0x60000500},
    {"FC300 EXP SW1", "E S1", 0x60000514},
    {"FC300 EXP2", "EXP2", 0x60000528},
    {"FC300 EXP SW2", "E S2", 0x6000053C},
    {"GK S1/S2", "GK12", 0x60000114},
    {"GK VOL", "GK V", 0x60000100},
    {"EXP", "EXP", 0x60000150},
    {"CTL1", "CTL1", 0x60000128},
    {"CTL2", "CTL2", 0x6000013C},
    {"CTL3", "CTL3", 0x60000164},
    {"CTL4", "CTL4", 0x60000178},
    {"DBEAM-V", "DB-V", 0x60000300},
    {"DBEAM-H", "DB-H", 0x60000314},
    {"RIBBON ACT", "RACT", 0x60000328},
    {"RIBBON POS", "RPOS", 0x6000033C},
    {"ASSIGN 1", "ASG1", 0x60007000},
    {"ASSIGN 2", "ASG2", 0x6000701C},
    {"ASSIGN 3", "ASG3", 0x60007038},
    {"ASSIGN 4", "ASG4", 0x60007054},
    {"ASSIGN 5", "ASG5", 0x60007100},
    {"ASSIGN 6", "ASG6", 0x6000711C},
    {"ASSIGN 7", "ASG7", 0x60007138},
    {"ASSIGN 8", "ASG8", 0x60007154},
    {"ASSIGN 9", "ASG9", 0x60007200},
    {"ASSIGN 10", "AS10", 0x6000721C},
    {"ASSIGN 11", "AS11", 0x60007238},
    {"ASSIGN 12", "AS12", 0x60007254},
    {"ASSIGN 13", "AS13", 0x60007300},
    {"ASSIGN 14", "AS14", 0x6000731C},
    {"ASSIGN 15", "AS15", 0x60007338},
    {"ASSIGN 16", "AS16", 0x60007354},
  };

const uint16_t VG99_NUMBER_OF_ASSIGNS = VG99_assigns.size();

uint8_t VG99_class::get_number_of_assigns()
{
    return VG99_NUMBER_OF_ASSIGNS;
}

QString VG99_class::read_assign_name(uint8_t assign_no)
{
     if (assign_no < VG99_NUMBER_OF_ASSIGNS) return VG99_assigns[assign_no].Title;
     else return "?";
}

QString VG99_class::read_assign_trigger(uint8_t trigger_no)
{
      if ((trigger_no > 0) && (trigger_no <= 8))  return "FC300 CTL" + QString::number(trigger_no);
      else if (trigger_no == 9) return "FC300 EXP1";
      else if (trigger_no == 10) return "FC300 EXP SW1";
      else if (trigger_no == 11) return "FC300 EXP2";
      else if (trigger_no == 12) return "FC300 EXP SW2";
      else if ((trigger_no > 12) && (trigger_no < 128)) return "CC#" + QString::number(trigger_no);
      else return "-";
}

uint8_t VG99_class::trigger_follow_assign(uint8_t assign_no)
{
    if (assign_no < 12) return assign_no + 1; // Return the trigger for the FC300 pedals
    if ((assign_no >= 23) && (assign_no <= 33))  return assign_no - 2; // Default cc assign_nos are 21 - 31
    if ((assign_no >= 34) && (assign_no <= 65)) return assign_no + 30; // And higher up it is 64 - 95
    return 0;
}

