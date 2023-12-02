#include "config.h"
#include "hardware.h"

// Copied data from config.ino (VController Teensy Code)

// ********************************* Section 2: VController configuration for fixed command pages ********************************************


const QVector<Cmd_struct> VC_fixed_commands = {
    // ******************************* PAGE 201: MENU *************************************************
      {PAGE_VC_MENU, LABEL, 'M', 'E', 'N', 'U', ' ', ' ', ' ', ' ' },
      {PAGE_VC_MENU, 1, MENU, COMMON, 9}, // ** Switch 01 **
      {PAGE_VC_MENU, 2, MENU, COMMON, 10}, // ** Switch 02 **
      {PAGE_VC_MENU, 3, MENU, COMMON, 11}, // ** Switch 03 **
      {PAGE_VC_MENU, 4, MENU, COMMON, 12}, // ** Switch 04 **
      {PAGE_VC_MENU, 5, MENU, COMMON, 5}, // ** Switch 05 **
      {PAGE_VC_MENU, 6, MENU, COMMON, 6}, // ** Switch 06 **
      {PAGE_VC_MENU, 7, MENU, COMMON, 7}, // ** Switch 07 **
      {PAGE_VC_MENU, 8, MENU, COMMON, 8}, // ** Switch 08 **
      {PAGE_VC_MENU, 9, MENU, COMMON, 1}, // ** Switch 09 **
      {PAGE_VC_MENU, 10, MENU, COMMON, 2}, // ** Switch 10 **
      {PAGE_VC_MENU, 11, MENU, COMMON, 3}, // ** Switch 11 **
      {PAGE_VC_MENU, 12, MENU, COMMON, 4}, // ** Switch 12 **
      {PAGE_VC_MENU, 13, MENU, COMMON, 13}, // ** Switch 13 **
      {PAGE_VC_MENU, 14, MENU, COMMON, 14}, // ** Switch 14 **
      {PAGE_VC_MENU, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_MENU, 16, NOTHING, COMMON}, // ** Switch 16 **
      {PAGE_VC_MENU, 17, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 1 or expr pedal 1
      {PAGE_VC_MENU, 18, NOTHING, COMMON}, // External switch 2
      {PAGE_VC_MENU, 19, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 3 or expr pedal 2
      {PAGE_VC_MENU, 20, NOTHING, COMMON}, // External switch 4
      {PAGE_VC_MENU, 21, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 5 or expr pedal 3
      {PAGE_VC_MENU, 22, NOTHING, COMMON}, // External switch 6
      {PAGE_VC_MENU, 23, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 7 or expr pedal 4
      {PAGE_VC_MENU, 24, NOTHING, COMMON}, // External switch 8

      // ******************************* PAGE 202: Current device Direct Select *************************************************
      {PAGE_VC_CURRENT_DIRECT_SELECT, LABEL, 'D', 'I', 'R', 'E', 'C', 'T', ' ', 'S' },
      {PAGE_VC_CURRENT_DIRECT_SELECT, LABEL, 'E', 'L', 'E', 'C', 'T', ' ', ' ', ' ' },
      {PAGE_VC_CURRENT_DIRECT_SELECT, 1, DIRECT_SELECT, CURRENT, 1, 10}, // ** Switch 01 **
      {PAGE_VC_CURRENT_DIRECT_SELECT, 2, DIRECT_SELECT, CURRENT, 2, 10}, // ** Switch 02 **
      {PAGE_VC_CURRENT_DIRECT_SELECT, 3, DIRECT_SELECT, CURRENT, 3, 10}, // ** Switch 03 **
      {PAGE_VC_CURRENT_DIRECT_SELECT, 4, DIRECT_SELECT, CURRENT, 4, 10}, // ** Switch 04 **
      {PAGE_VC_CURRENT_DIRECT_SELECT, 5, DIRECT_SELECT, CURRENT, 5, 10}, // ** Switch 05 **
      {PAGE_VC_CURRENT_DIRECT_SELECT, 6, DIRECT_SELECT, CURRENT, 6, 10}, // ** Switch 06 **
      {PAGE_VC_CURRENT_DIRECT_SELECT, 7, DIRECT_SELECT, CURRENT, 7, 10}, // ** Switch 07 **
      {PAGE_VC_CURRENT_DIRECT_SELECT, 8, DIRECT_SELECT, CURRENT, 8, 10}, // ** Switch 08 **
      {PAGE_VC_CURRENT_DIRECT_SELECT, 9, DIRECT_SELECT, CURRENT, 9, 10}, // ** Switch 09 **
      {PAGE_VC_CURRENT_DIRECT_SELECT, 10, DIRECT_SELECT, CURRENT, 0, 10}, // ** Switch 10 **
      {PAGE_VC_CURRENT_DIRECT_SELECT, 11, NOTHING, COMMON}, // ** Switch 11 **
      //{PAGE_VC_CURRENT_DIRECT_SELECT, 12, NOTHING, COMMON}, // ** Switch 12 **
      {PAGE_VC_CURRENT_DIRECT_SELECT, 13, PATCH, CURRENT, BANKDOWN, 100}, // ** Switch 13 **
      {PAGE_VC_CURRENT_DIRECT_SELECT, 14, PATCH, CURRENT, BANKUP, 100}, // ** Switch 14 **
      //{PAGE_VC_CURRENT_DIRECT_SELECT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_CURRENT_DIRECT_SELECT, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

      // ******************************* PAGE 203: DEVICE MODE *************************************************
      {PAGE_VC_DEVICE_MODE, LABEL, 'D', 'E', 'V', 'I', 'C', 'E', ' ', 'M' },
      {PAGE_VC_DEVICE_MODE, LABEL, 'O', 'D', 'E', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VC_DEVICE_MODE, 1, NOTHING, COMMON}, // ** Switch 01 **
      {PAGE_VC_DEVICE_MODE, 2, NOTHING, COMMON}, // ** Switch 02 **
      {PAGE_VC_DEVICE_MODE, 3, NOTHING, COMMON}, // ** Switch 03 **
      {PAGE_VC_DEVICE_MODE, 4, NOTHING, COMMON}, // ** Switch 04 **
      {PAGE_VC_DEVICE_MODE, 5, PAGE, COMMON, SELECT, PAGE_VC_MENU}, // ** Switch 05 **
      {PAGE_VC_DEVICE_MODE, 6, NOTHING, COMMON}, // ** Switch 06 **
      {PAGE_VC_DEVICE_MODE, 7, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 07 **
      {PAGE_VC_DEVICE_MODE, 8, PAGE, COMMON, SELECT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 08 **
      {PAGE_VC_DEVICE_MODE, 9, MODE, COMMON, SELECT, SONG_MODE}, // ** Switch 09 **
      {PAGE_VC_DEVICE_MODE, 10, MODE, COMMON, SELECT, PAGE_MODE}, // ** Switch 10 **
      {PAGE_VC_DEVICE_MODE, 11, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 11 **
      {PAGE_VC_DEVICE_MODE, 12, PAGE, COMMON, SELECT, PAGE_VC_SETLIST_SELECT}, // ** Switch 12 **
      {PAGE_VC_DEVICE_MODE, 13, NOTHING, COMMON}, // ** Switch 13 **
      {PAGE_VC_DEVICE_MODE, 14, NOTHING, COMMON}, // ** Switch 14 **
      {PAGE_VC_DEVICE_MODE, 15, PAGE, COMMON, SELECT, PAGE_VC_MENU}, // ** Switch 15 **

      // ******************************* PAGE 204: PAGE MODE *************************************************
      {PAGE_VC_PAGE_MODE, LABEL, 'P', 'A', 'G', 'E', ' ', 'M', 'O', 'D' },
      {PAGE_VC_PAGE_MODE, LABEL, 'E', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VC_PAGE_MODE, 1, PAGE, COMMON, BANKSELECT, 1, 8}, // ** Switch 01 **
      {PAGE_VC_PAGE_MODE, 2, PAGE, COMMON, BANKSELECT, 2, 8}, // ** Switch 02 **
      {PAGE_VC_PAGE_MODE, 3, PAGE, COMMON, BANKSELECT, 3, 8}, // ** Switch 03 **
      {PAGE_VC_PAGE_MODE, 4, PAGE, COMMON, BANKSELECT, 4, 8}, // ** Switch 04 **
      {PAGE_VC_PAGE_MODE, 5, PAGE, COMMON, BANKSELECT, 5, 8}, // ** Switch 05 **
      {PAGE_VC_PAGE_MODE, 6, PAGE, COMMON, BANKSELECT, 6, 8}, // ** Switch 06 **
      {PAGE_VC_PAGE_MODE, 7, PAGE, COMMON, BANKSELECT, 7, 8}, // ** Switch 07 **
      {PAGE_VC_PAGE_MODE, 8, PAGE, COMMON, BANKSELECT, 8, 8}, // ** Switch 08 **
      {PAGE_VC_PAGE_MODE, 9, MODE, COMMON, SELECT, SONG_MODE}, // ** Switch 09 **
      {PAGE_VC_PAGE_MODE, 10, MODE, COMMON, SELECT, PAGE_MODE}, // ** Switch 10 **
      {PAGE_VC_PAGE_MODE, 11, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 11 **
      {PAGE_VC_PAGE_MODE, 12, PAGE, COMMON, SELECT, PAGE_VC_SETLIST_SELECT}, // ** Switch 12 **
      {PAGE_VC_PAGE_MODE, 13, PAGE, COMMON, BANKDOWN, 8}, // ** Switch 13 **
      {PAGE_VC_PAGE_MODE, 14, PAGE, COMMON, BANKUP, 8}, // ** Switch 14 **
      {PAGE_VC_PAGE_MODE, 15, PAGE, COMMON, SELECT, PAGE_VC_MENU}, // ** Switch 15 **


      // ******************************* PAGE 205: Current_patch_bank (10 buttons per page) *************************************************
      {PAGE_VC_CURRENT_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
      {PAGE_VC_CURRENT_PATCH_BANK, LABEL, 'N', 'K', ' ', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VC_CURRENT_PATCH_BANK, 1, PATCH, CURRENT, BANKSELECT, 1, 10}, // ** Switch 01 **
      {PAGE_VC_CURRENT_PATCH_BANK, 2, PATCH, CURRENT, BANKSELECT, 2, 10}, // ** Switch 02 **
      {PAGE_VC_CURRENT_PATCH_BANK, 3, PATCH, CURRENT, BANKSELECT, 3, 10}, // ** Switch 03 **
      {PAGE_VC_CURRENT_PATCH_BANK, 4, PATCH, CURRENT, BANKSELECT, 4, 10}, // ** Switch 04 **
      {PAGE_VC_CURRENT_PATCH_BANK, 5, PATCH, CURRENT, BANKSELECT, 5, 10}, // ** Switch 05 **
      {PAGE_VC_CURRENT_PATCH_BANK, 6, PATCH, CURRENT, BANKSELECT, 6, 10}, // ** Switch 06 **
      {PAGE_VC_CURRENT_PATCH_BANK, 7, PATCH, CURRENT, BANKSELECT, 7, 10}, // ** Switch 07 **
      {PAGE_VC_CURRENT_PATCH_BANK, 8, PATCH, CURRENT, BANKSELECT, 8, 10}, // ** Switch 08 **
      {PAGE_VC_CURRENT_PATCH_BANK, 9, PATCH, CURRENT, BANKSELECT, 9, 10}, // ** Switch 09 **
      {PAGE_VC_CURRENT_PATCH_BANK, 10, PATCH, CURRENT, BANKSELECT, 10, 10}, // ** Switch 10 **
      //{PAGE_VC_CURRENT_PATCH_BANK, 11, PARAMETER, CURRENT, 1, TOGGLE, 1, 0}, // ** Switch 11 **
      //{PAGE_VC_CURRENT_PATCH_BANK, 11, PARAMETER, CURRENT, 2, TOGGLE, 1, 0}, // ** Switch 11 **
      //{PAGE_VC_CURRENT_PATCH_BANK, 11, PARAMETER, CURRENT, 3, TOGGLE, 1, 0}, // ** Switch 11 **
      //{PAGE_VC_CURRENT_PATCH_BANK, 11, PARAMETER, CURRENT, 4, TOGGLE, 1, 0}, // ** Switch 11 **
      //{PAGE_VC_CURRENT_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_CURRENT_PATCH_BANK, 13, PATCH, CURRENT, BANKDOWN, 10}, // ** Switch 13 **
      {PAGE_VC_CURRENT_PATCH_BANK, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
      {PAGE_VC_CURRENT_PATCH_BANK, 14, PATCH, CURRENT, BANKUP, 10}, // ** Switch 14 **
      {PAGE_VC_CURRENT_PATCH_BANK, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 14 **
      //{PAGE_VC_CURRENT_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_CURRENT_PATCH_BANK, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

      // ******************************* PAGE 206: GR55 select *************************************************
      {PAGE_VC_GR55_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
      {PAGE_VC_GR55_PATCH_BANK, LABEL, 'N', 'K', ' ', 'G', 'R', ' ', ' ', ' ' },
      {PAGE_VC_GR55_PATCH_BANK, 1, PATCH, GR55, BANKSELECT, 1, 9}, // ** Switch 01 **
      {PAGE_VC_GR55_PATCH_BANK, 2, PATCH, GR55, BANKSELECT, 2, 9}, // ** Switch 02 **
      {PAGE_VC_GR55_PATCH_BANK, 3, PATCH, GR55, BANKSELECT, 3, 9}, // ** Switch 03 **
      {PAGE_VC_GR55_PATCH_BANK, 4, PARAMETER, GR55, 33, TOGGLE, 1, 0}, // ** Switch 04 **
      {PAGE_VC_GR55_PATCH_BANK, 5, PATCH, GR55, BANKSELECT, 4, 9}, // ** Switch 05 **
      {PAGE_VC_GR55_PATCH_BANK, 6, PATCH, GR55, BANKSELECT, 5, 9}, // ** Switch 06 **
      {PAGE_VC_GR55_PATCH_BANK, 7, PATCH, GR55, BANKSELECT, 6, 9}, // ** Switch 07 **
      {PAGE_VC_GR55_PATCH_BANK, 8, NOTHING, COMMON}, // ** Switch 08 **
      {PAGE_VC_GR55_PATCH_BANK, 9, PATCH, GR55, BANKSELECT, 7, 9}, // ** Switch 09 **
      {PAGE_VC_GR55_PATCH_BANK, 10, PATCH, GR55, BANKSELECT, 8, 9}, // ** Switch 10 **
      {PAGE_VC_GR55_PATCH_BANK, 11, PATCH, GR55, BANKSELECT, 9, 9}, // ** Switch 11 **
      //{PAGE_VC_GR55_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_GR55_PATCH_BANK, 13, PATCH, GR55, BANKDOWN, 9}, // ** Switch 13 **
      {PAGE_VC_GR55_PATCH_BANK, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
      {PAGE_VC_GR55_PATCH_BANK, 14, PATCH, GR55, BANKUP, 9}, // ** Switch 14 **
      {PAGE_VC_GR55_PATCH_BANK, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
      //{PAGE_VC_GR55_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_GR55_PATCH_BANK, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

      // ******************************* PAGE 207: Zoom patch bank *************************************************
      {PAGE_VC_ZOOM_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', '+', ' ' },
      {PAGE_VC_ZOOM_PATCH_BANK, LABEL, 'F', 'X', ' ', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VC_ZOOM_PATCH_BANK, 1, PATCH, CURRENT, BANKSELECT, 1, 5}, // ** Switch 01 **
      {PAGE_VC_ZOOM_PATCH_BANK, 2, PATCH, CURRENT, BANKSELECT, 2, 5}, // ** Switch 02 **
      {PAGE_VC_ZOOM_PATCH_BANK, 3, PATCH, CURRENT, BANKSELECT, 3, 5}, // ** Switch 03 **
      {PAGE_VC_ZOOM_PATCH_BANK, 4, PATCH, CURRENT, BANKSELECT, 4, 5}, // ** Switch 04 **
      {PAGE_VC_ZOOM_PATCH_BANK, 5, PARAMETER, CURRENT, 0, TOGGLE, 1, 0}, // ** Switch 05 **
      {PAGE_VC_ZOOM_PATCH_BANK, 6, PARAMETER, CURRENT, 1, TOGGLE, 1, 0}, // ** Switch 06 **
      {PAGE_VC_ZOOM_PATCH_BANK, 7, PARAMETER, CURRENT, 2, TOGGLE, 1, 0}, // ** Switch 07 **
      {PAGE_VC_ZOOM_PATCH_BANK, 8, PATCH, CURRENT, BANKSELECT, 5, 5}, // ** Switch 08 **
      {PAGE_VC_ZOOM_PATCH_BANK, 9, PARAMETER, CURRENT, 3, TOGGLE, 1, 0}, // ** Switch 09 **
      {PAGE_VC_ZOOM_PATCH_BANK, 10, PARAMETER, CURRENT, 4, TOGGLE, 1, 0}, // ** Switch 10 **
      {PAGE_VC_ZOOM_PATCH_BANK, 11, PARAMETER, CURRENT, 5, TOGGLE, 1, 0}, // ** Switch 11 **
      //{PAGE_VC_ZOOM_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_ZOOM_PATCH_BANK, 13, PATCH, CURRENT, BANKDOWN, 5}, // ** Switch 13 **
      {PAGE_VC_ZOOM_PATCH_BANK, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
      {PAGE_VC_ZOOM_PATCH_BANK, 14, PATCH, CURRENT, BANKUP, 5}, // ** Switch 14 **
      {PAGE_VC_ZOOM_PATCH_BANK, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
      //{PAGE_VC_ZOOM_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_ZOOM_PATCH_BANK, 16, NOTHING, COMMON}, // ** Switch 16 **

      // ******************************* PAGE 208: Parameters current device *************************************************
      {PAGE_VC_CURRENT_PARAMETER, LABEL, 'P', 'A', 'R', ' ', 'B', 'A', 'N', 'K' },
      {PAGE_VC_CURRENT_PARAMETER, 1, PAR_BANK, CURRENT, 1, 10}, // ** Switch 01 **
      {PAGE_VC_CURRENT_PARAMETER, 2, PAR_BANK, CURRENT, 2, 10}, // ** Switch 02 **
      {PAGE_VC_CURRENT_PARAMETER, 3, PAR_BANK, CURRENT, 3, 10}, // ** Switch 03 **
      {PAGE_VC_CURRENT_PARAMETER, 4, PAR_BANK, CURRENT, 4, 10}, // ** Switch 04 **
      {PAGE_VC_CURRENT_PARAMETER, 5, PAR_BANK, CURRENT, 5, 10}, // ** Switch 05 **
      {PAGE_VC_CURRENT_PARAMETER, 6, PAR_BANK, CURRENT, 6, 10}, // ** Switch 06 **
      {PAGE_VC_CURRENT_PARAMETER, 7, PAR_BANK, CURRENT, 7, 10}, // ** Switch 07 **
      {PAGE_VC_CURRENT_PARAMETER, 8, PAR_BANK, CURRENT, 8, 10}, // ** Switch 08 **
      {PAGE_VC_CURRENT_PARAMETER, 9, PAR_BANK, CURRENT, 9, 10}, // ** Switch 09 **
      {PAGE_VC_CURRENT_PARAMETER, 10, PAR_BANK, CURRENT, 10, 10}, // ** Switch 10 **
      //{PAGE_VC_CURRENT_PARAMETER, 11, NOTHING, COMMON}, // ** Switch 11 **
      //{PAGE_VC_CURRENT_PARAMETER, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_CURRENT_PARAMETER, 13, PAR_BANK_DOWN, CURRENT, 10}, // ** Switch 13 **
      {PAGE_VC_CURRENT_PARAMETER, 14, PAR_BANK_UP, CURRENT, 10}, // ** Switch 14 **
      //{PAGE_VC_CURRENT_PARAMETER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_CURRENT_PARAMETER, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

      // ******************************* PAGE 209: GP10 assign *************************************************
      {PAGE_VC_GP10_ASSIGNS, LABEL, 'A', 'S', 'S', 'G', 'N', ' ', 'G', 'P' },
      {PAGE_VC_GP10_ASSIGNS, LABEL, '1', '0', ' ', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VC_GP10_ASSIGNS, 1, ASSIGN, GP10, SELECT, 0, 21}, // ** Switch 01 **
      {PAGE_VC_GP10_ASSIGNS, 2, ASSIGN, GP10, SELECT, 1, 22}, // ** Switch 02 **
      {PAGE_VC_GP10_ASSIGNS, 3, ASSIGN, GP10, SELECT, 2, 23}, // ** Switch 03 **
      {PAGE_VC_GP10_ASSIGNS, 4, ASSIGN, GP10, SELECT, 3, 24}, // ** Switch 04 **
      {PAGE_VC_GP10_ASSIGNS, 5, ASSIGN, GP10, SELECT, 4, 25}, // ** Switch 05 **
      {PAGE_VC_GP10_ASSIGNS, 6, ASSIGN, GP10, SELECT, 5, 26}, // ** Switch 06 **
      {PAGE_VC_GP10_ASSIGNS, 7, ASSIGN, GP10, SELECT, 6, 27}, // ** Switch 07 **
      {PAGE_VC_GP10_ASSIGNS, 8, ASSIGN, GP10, SELECT, 7, 28}, // ** Switch 08 **
      {PAGE_VC_GP10_ASSIGNS, 9, NOTHING, COMMON}, // ** Switch 09 **
      {PAGE_VC_GP10_ASSIGNS, 10, NOTHING, COMMON}, // ** Switch 10 **
      //{PAGE_VC_GP10_ASSIGNS, 11, NOTHING, COMMON}, // ** Switch 11 **
      //{PAGE_VC_GP10_ASSIGNS, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_GP10_ASSIGNS, 13, PATCH, GP10, PREV, 9}, // ** Switch 13 **
      {PAGE_VC_GP10_ASSIGNS, 14, PATCH, GP10, NEXT, 9}, // ** Switch 14 **
      //{PAGE_VC_GP10_ASSIGNS, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_GP10_ASSIGNS, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

      // ******************************* PAGE 210: GR55 Assign *************************************************
      {PAGE_VC_GR55_ASSIGNS, LABEL, 'A', 'S', 'S', 'G', 'N', ' ', 'G', 'R'},
      {PAGE_VC_GR55_ASSIGNS, LABEL, '5', '5', ' ', ' ', ' ', ' ', ' ', ' '},
      {PAGE_VC_GR55_ASSIGNS, 1, PARAMETER, GR55, 4, TOGGLE, 1, 0}, // ** Switch 01 **
      {PAGE_VC_GR55_ASSIGNS, 2, PARAMETER, GR55, 6, TOGGLE, 1, 0}, // ** Switch 02 **
      {PAGE_VC_GR55_ASSIGNS, 3, PARAMETER, GR55, 8, TOGGLE, 1, 0}, // ** Switch 03 **
      {PAGE_VC_GR55_ASSIGNS, 4, PARAMETER, GR55, 9, TOGGLE, 1, 0}, // ** Switch 04 **
      {PAGE_VC_GR55_ASSIGNS, 5, ASSIGN, GR55, SELECT, 4}, // ** Switch 05 **
      {PAGE_VC_GR55_ASSIGNS, 6, ASSIGN, GR55, SELECT, 5}, // ** Switch 06 **
      {PAGE_VC_GR55_ASSIGNS, 7, ASSIGN, GR55, SELECT, 6}, // ** Switch 07 **
      {PAGE_VC_GR55_ASSIGNS, 8, ASSIGN, GR55, SELECT, 7}, // ** Switch 08 **
      {PAGE_VC_GR55_ASSIGNS, 9, ASSIGN, GR55, SELECT, 0}, // ** Switch 09 **
      {PAGE_VC_GR55_ASSIGNS, 10, ASSIGN, GR55, SELECT, 1}, // ** Switch 10 **
      //{PAGE_VC_GR55_ASSIGNS, 11, NOTHING, COMMON}, // ** Switch 11 **
      //{PAGE_VC_GR55_ASSIGNS, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_GR55_ASSIGNS, 13, PATCH, GR55, PREV}, // ** Switch 13 **
      {PAGE_VC_GR55_ASSIGNS, 14, PATCH, GR55, NEXT}, // ** Switch 14 **
      //{PAGE_VC_GR55_ASSIGNS, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_GR55_ASSIGNS, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

      // ******************************* PAGE 211: VG99 Edit *************************************************
      {PAGE_VC_VG99_EDIT, LABEL, 'V', 'G', '9', '9', ' ', 'E', 'D', 'I'},
      {PAGE_VC_VG99_EDIT, LABEL, 'T', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
      {PAGE_VC_VG99_EDIT, 1, PAR_BANK_CATEGORY, VG99, 1}, // ** Switch 01 **
      {PAGE_VC_VG99_EDIT, 2, PAR_BANK_CATEGORY, VG99, 2}, // ** Switch 02 **
      {PAGE_VC_VG99_EDIT, 3, PAR_BANK_CATEGORY, VG99, 3}, // ** Switch 03 **
      {PAGE_VC_VG99_EDIT, 4, PAR_BANK_CATEGORY, VG99, 4}, // ** Switch 04 **
      {PAGE_VC_VG99_EDIT, 5, PAR_BANK_CATEGORY, VG99, 5}, // ** Switch 05 **
      {PAGE_VC_VG99_EDIT, 6, PAR_BANK_CATEGORY, VG99, 6}, // ** Switch 06 **
      {PAGE_VC_VG99_EDIT, 7, PAR_BANK_CATEGORY, VG99, 7}, // ** Switch 07 **
      {PAGE_VC_VG99_EDIT, 8, PAR_BANK_CATEGORY, VG99, 8}, // ** Switch 08 **
      {PAGE_VC_VG99_EDIT, 9, PAR_BANK_CATEGORY, VG99, 9}, // ** Switch 09 **
      {PAGE_VC_VG99_EDIT, 10, PAR_BANK_CATEGORY, VG99, 10}, // ** Switch 10 **
      {PAGE_VC_VG99_EDIT, 11, PAR_BANK_CATEGORY, VG99, 11 }, // ** Switch 11 **
      {PAGE_VC_VG99_EDIT, 12, PAR_BANK_CATEGORY, VG99, 12}, // ** Switch 12 **
      {PAGE_VC_VG99_EDIT, 13, PATCH, VG99, PREV}, // ** Switch 10 **
      {PAGE_VC_VG99_EDIT, 14, PATCH, VG99, NEXT}, // ** Switch 11 **
      //{PAGE_VC_VG99_EDIT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 12 **
      {PAGE_VC_VG99_EDIT, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 12 **

      // ******************************* PAGE 212: VG99 Assign *************************************************
      {PAGE_VC_VG99_ASSIGNS, LABEL, 'F', 'C', '3', '0', '0', ' ', 'A', 'S'},
      {PAGE_VC_VG99_ASSIGNS, LABEL, 'G', 'N', ' ', ' ', ' ', ' ', ' ', ' '},
      {PAGE_VC_VG99_ASSIGNS, 1, FC300_CTL1}, // ** Switch 01 **
      {PAGE_VC_VG99_ASSIGNS, 2, FC300_CTL2}, // ** Switch 02 **
      {PAGE_VC_VG99_ASSIGNS, 3, FC300_CTL3}, // ** Switch 03 **
      {PAGE_VC_VG99_ASSIGNS, 4, FC300_CTL4}, // ** Switch 04 **
      {PAGE_VC_VG99_ASSIGNS, 5, FC300_CTL5}, // ** Switch 05 **
      {PAGE_VC_VG99_ASSIGNS, 6, FC300_CTL6}, // ** Switch 06 **
      {PAGE_VC_VG99_ASSIGNS, 7, FC300_CTL7}, // ** Switch 07 **
      {PAGE_VC_VG99_ASSIGNS, 8, FC300_CTL8}, // ** Switch 08 **
      {PAGE_VC_VG99_ASSIGNS, 9, FC300_EXP_SW1}, // ** Switch 09 **
      {PAGE_VC_VG99_ASSIGNS, 10, FC300_EXP_SW2}, // ** Switch 10 **
      {PAGE_VC_VG99_ASSIGNS, 11, PAGE, COMMON, SELECT, PAGE_VC_VG99_ASSIGNS2 }, // ** Switch 11 **
      //{PAGE_VC_VG99_ASSIGNS, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_VG99_ASSIGNS, 13, PATCH, VG99, PREV}, // ** Switch 10 **
      {PAGE_VC_VG99_ASSIGNS, 14, PATCH, VG99, NEXT}, // ** Switch 11 **
      //{PAGE_VC_VG99_ASSIGNS, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 12 **
      {PAGE_VC_VG99_ASSIGNS, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 12 **

      // ******************************* PAGE 213: VG99 Assign2 *************************************************
      {PAGE_VC_VG99_ASSIGNS2, LABEL, 'M', 'O', 'R', 'E', ' ', 'A', 'S', 'G'},
      {PAGE_VC_VG99_ASSIGNS2, 1, ASSIGN, VG99, SELECT, 15, 255}, // ** Switch 01 **
      {PAGE_VC_VG99_ASSIGNS2, 2, ASSIGN, VG99, SELECT, 16, 255}, // ** Switch 02 **
      {PAGE_VC_VG99_ASSIGNS2, 3, ASSIGN, VG99, SELECT, 17, 255}, // ** Switch 03 **
      {PAGE_VC_VG99_ASSIGNS2, 4, ASSIGN, VG99, SELECT, 18, 255}, // ** Switch 04 **
      {PAGE_VC_VG99_ASSIGNS2, 5, ASSIGN, VG99, SELECT, 19, 255}, // ** Switch 05 **
      {PAGE_VC_VG99_ASSIGNS2, 6, ASSIGN, VG99, SELECT, 20, 255}, // ** Switch 06 **
      {PAGE_VC_VG99_ASSIGNS2, 7, ASSIGN, VG99, SELECT, 21, 255}, // ** Switch 07 **
      {PAGE_VC_VG99_ASSIGNS2, 8, ASSIGN, VG99, SELECT, 22, 255}, // ** Switch 08 **
      {PAGE_VC_VG99_ASSIGNS2, 9, ASSIGN, VG99, SELECT, 12, 255}, // ** Switch 09 **
      {PAGE_VC_VG99_ASSIGNS2, 10, ASSIGN, VG99, SELECT, 13, 255}, // ** Switch 10 **
      {PAGE_VC_VG99_ASSIGNS2, 11, ASSIGN, VG99, SELECT, 14, 255}, // ** Switch 11 **
      // ** Switch 12 **
      {PAGE_VC_VG99_ASSIGNS2, 13, PATCH, VG99, PREV}, // ** Switch 10 **
      {PAGE_VC_VG99_ASSIGNS2, 14, PATCH, VG99, NEXT}, // ** Switch 11 **
      //{PAGE_VC_VG99_ASSIGNS2, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 12 **
      {PAGE_VC_VG99_ASSIGNS2, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 12 **

      // ******************************* PAGE 214: Line 6 M13 effects *************************************************
      {PAGE_VC_M13_PARAMETER, LABEL, 'L', 'I', 'N', 'E', '6', ' ', 'M', '1' },
      {PAGE_VC_M13_PARAMETER, LABEL, '3', ' ', 'F', 'X', ' ', ' ', ' ', ' ' },
      {PAGE_VC_M13_PARAMETER, 1, PARAMETER, M13, 0, TOGGLE, 1, 0}, // ** Switch 01 **
      {PAGE_VC_M13_PARAMETER, 2, PARAMETER, M13, 3, TOGGLE, 1, 0}, // ** Switch 02 **
      {PAGE_VC_M13_PARAMETER, 3, PARAMETER, M13, 6, TOGGLE, 1, 0}, // ** Switch 03 **
      {PAGE_VC_M13_PARAMETER, 4, PARAMETER, M13, 9, TOGGLE, 1, 0}, // ** Switch 04 **
      {PAGE_VC_M13_PARAMETER, 5, PARAMETER, M13, 1, TOGGLE, 1, 0}, // ** Switch 05 **
      {PAGE_VC_M13_PARAMETER, 6, PARAMETER, M13, 4, TOGGLE, 1, 0}, // ** Switch 06 **
      {PAGE_VC_M13_PARAMETER, 7, PARAMETER, M13, 7, TOGGLE, 1, 0}, // ** Switch 07 **
      {PAGE_VC_M13_PARAMETER, 8, PARAMETER, M13, 10, TOGGLE, 1, 0}, // ** Switch 08 **
      {PAGE_VC_M13_PARAMETER, 9, PARAMETER, M13, 2, TOGGLE, 1, 0}, // ** Switch 09 **
      {PAGE_VC_M13_PARAMETER, 10, PARAMETER, M13, 5, TOGGLE, 1, 0}, // ** Switch 10 **
      {PAGE_VC_M13_PARAMETER, 11, PARAMETER, M13, 8, TOGGLE, 1, 0}, // ** Switch 11 **
      {PAGE_VC_M13_PARAMETER, 12, PARAMETER, M13, 11, TOGGLE, 1, 0}, // ** Switch 12 **
      {PAGE_VC_M13_PARAMETER, 13, PATCH, M13, PREV}, // ** Switch 13 **
      {PAGE_VC_M13_PARAMETER, 14, PATCH, M13, NEXT}, // ** Switch 14 **
      //{PAGE_VC_M13_PARAMETER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_M13_PARAMETER, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

      // ******************************* PAGE 215: Looper control *************************************************
      {PAGE_VC_FULL_LOOPER, LABEL, 'L', 'O', 'O', 'P', 'E', 'R', ' ', 'C' },
      {PAGE_VC_FULL_LOOPER, LABEL, 'O', 'N', 'T', 'R', 'O', 'L', ' ', ' ' },
      {PAGE_VC_FULL_LOOPER, 1, LOOPER, CURRENT, LOOPER_REC_OVERDUB}, // ** Switch 01 **
      {PAGE_VC_FULL_LOOPER, 2, LOOPER, CURRENT, LOOPER_PLAY_STOP}, // ** Switch 02 **
      {PAGE_VC_FULL_LOOPER, 3, LOOPER, CURRENT, LOOPER_HALF_SPEED}, // ** Switch 03 **
      {PAGE_VC_FULL_LOOPER, 4, LOOPER, CURRENT, LOOPER_REVERSE}, // ** Switch 04 **
      {PAGE_VC_FULL_LOOPER, 5, LOOPER, CURRENT, LOOPER_UNDO_REDO}, // ** Switch 05 **
      {PAGE_VC_FULL_LOOPER, 6, LOOPER, CURRENT, LOOPER_PLAY_ONCE}, // ** Switch 06 **
      {PAGE_VC_FULL_LOOPER, 7, LOOPER, CURRENT, LOOPER_PRE_POST}, // ** Switch 07 **
      {PAGE_VC_FULL_LOOPER, 8, LOOPER, CURRENT, LOOPER_SHOW_HIDE}, // ** Switch 08 **
      //{PAGE_VC_FULL_LOOPER, 9, PATCH, CURRENT, BANKSELECT, 1, 3}, // ** Switch 09 **
      {PAGE_VC_FULL_LOOPER, 10, PATCH, CURRENT, PREV}, // ** Switch 10 **
      {PAGE_VC_FULL_LOOPER, 11, PATCH, CURRENT, NEXT, 3, 3}, // ** Switch 11 **
      {PAGE_VC_FULL_LOOPER, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_FULL_LOOPER, 13, PATCH, CURRENT, 3, PREV}, // ** Switch 13 **
      {PAGE_VC_FULL_LOOPER, 14, PATCH, CURRENT, 3, NEXT}, // ** Switch 14 **
      //{PAGE_VC_FULL_LOOPER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_FULL_LOOPER, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

      // ******************************* PAGE 216: Helix_patch_bank (8 buttons per page) *************************************************
      {PAGE_VC_HLX_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
      {PAGE_VC_HLX_PATCH_BANK, LABEL, 'N', 'K', ' ', 'H', 'E', 'L', 'I', 'X' },
      {PAGE_VC_HLX_PATCH_BANK, 1, PATCH, HLX, BANKSELECT, 1, 8}, // ** Switch 01 **
      {PAGE_VC_HLX_PATCH_BANK, 2, PATCH, HLX, BANKSELECT, 2, 8}, // ** Switch 02 **
      {PAGE_VC_HLX_PATCH_BANK, 3, PATCH, HLX, BANKSELECT, 3, 8}, // ** Switch 03 **
      {PAGE_VC_HLX_PATCH_BANK, 4, PATCH, HLX, BANKSELECT, 4, 8}, // ** Switch 04 **
      {PAGE_VC_HLX_PATCH_BANK, 5, PATCH, HLX, BANKSELECT, 5, 8}, // ** Switch 05 **
      {PAGE_VC_HLX_PATCH_BANK, 6, PATCH, HLX, BANKSELECT, 6, 8}, // ** Switch 06 **
      {PAGE_VC_HLX_PATCH_BANK, 7, PATCH, HLX, BANKSELECT, 7, 8}, // ** Switch 07 **
      {PAGE_VC_HLX_PATCH_BANK, 8, PATCH, HLX, BANKSELECT, 8, 8}, // ** Switch 08 **
      {PAGE_VC_HLX_PATCH_BANK, 9, PARAMETER, HLX, 14, STEP, 0, 6, 1}, // ** Switch 09 **
      {PAGE_VC_HLX_PATCH_BANK, 10, PAGE, COMMON, SELECT, PAGE_VC_FULL_LOOPER}, // ** Switch 10 **
      //{PAGE_VC_HLX_PATCH_BANK, 11, PARAMETER, HLX, 1, TOGGLE, 1, 0}, // ** Switch 11 **
      //{PAGE_VC_HLX_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_HLX_PATCH_BANK, 13, PATCH, HLX, BANKDOWN, 8}, // ** Switch 13 **
      {PAGE_VC_HLX_PATCH_BANK, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
      {PAGE_VC_HLX_PATCH_BANK, 14, PATCH, HLX, BANKUP, 8}, // ** Switch 14 **
      {PAGE_VC_HLX_PATCH_BANK, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
      //{PAGE_VC_HLX_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_HLX_PATCH_BANK, 16, OPEN_NEXT_PAGE_OF_DEVICE, HLX}, // ** Switch 16 **

      // ******************************* PAGE 217: Helix_parameter *************************************************
      {PAGE_VC_HLX_PARAMETER, LABEL, 'P', 'A', 'R', 'A', 'M', 'E', 'T', 'E' },
      {PAGE_VC_HLX_PARAMETER, LABEL, 'R', 'S', ' ', 'H', 'E', 'L', 'I', 'X' },
      {PAGE_VC_HLX_PARAMETER, 1, PARAMETER, HLX, 6, MOMENTARY, 127, 0}, // ** Switch 01 **
      {PAGE_VC_HLX_PARAMETER, 2, PARAMETER, HLX, 7, MOMENTARY, 127, 0}, // ** Switch 02 **
      {PAGE_VC_HLX_PARAMETER, 3, PARAMETER, HLX, 8, MOMENTARY, 127, 0}, // ** Switch 03 **
      {PAGE_VC_HLX_PARAMETER, 4, PARAMETER, HLX, 9, MOMENTARY, 127, 0}, // ** Switch 04 **
      {PAGE_VC_HLX_PARAMETER, 5, PARAMETER, HLX, 1, MOMENTARY, 127, 0}, // ** Switch 05 **
      {PAGE_VC_HLX_PARAMETER, 6, PARAMETER, HLX, 2, MOMENTARY, 127, 0}, // ** Switch 06 **
      {PAGE_VC_HLX_PARAMETER, 7, PARAMETER, HLX, 3, MOMENTARY, 127, 0}, // ** Switch 07 **
      {PAGE_VC_HLX_PARAMETER, 8, PARAMETER, HLX, 4, MOMENTARY, 127, 0}, // ** Switch 08 **
      {PAGE_VC_HLX_PARAMETER, 9, LOOPER, HLX, LOOPER_REC_OVERDUB}, // ** Switch 09 **
      {PAGE_VC_HLX_PARAMETER, 10, LOOPER, HLX, LOOPER_PLAY_STOP}, // ** Switch 10 **
      //{PAGE_VC_HLX_PARAMETER, 11, PARAMETER, HLX, 10, MOMENTARY, 127, 0}, // ** Switch 11 **
      //{PAGE_VC_HLX_PARAMETER, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_HLX_PARAMETER, 13, PATCH, HLX, PREV}, // ** Switch 13 **
      {PAGE_VC_HLX_PARAMETER, 14, PATCH, HLX, NEXT}, // ** Switch 14 **
      //{PAGE_VC_HLX_PARAMETER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_HLX_PARAMETER, 16, OPEN_NEXT_PAGE_OF_DEVICE, HLX}, // ** Switch 16 **

      // ******************************* PAGE 218: Snapshots (Helix) /scenes (AxeFX) *************************************************
      {PAGE_VC_SNAPSCENE_LOOPER, LABEL, 'S', 'N', 'A', 'P', 'S', 'C', 'E', 'N' },
      {PAGE_VC_SNAPSCENE_LOOPER, LABEL, 'E', '/', 'L', 'O', 'O', 'P', 'E', 'R' },
      {PAGE_VC_SNAPSCENE_LOOPER, 1, SNAPSCENE, CURRENT, 1, 0, 0}, // ** Switch 01 **
      {PAGE_VC_SNAPSCENE_LOOPER, 2, SNAPSCENE, CURRENT, 2, 0, 0}, // ** Switch 02 **
      {PAGE_VC_SNAPSCENE_LOOPER, 3, SNAPSCENE, CURRENT, 3, 0, 0}, // ** Switch 03 **
      {PAGE_VC_SNAPSCENE_LOOPER, 4, SNAPSCENE, CURRENT, 4, 0, 0}, // ** Switch 04 **
      {PAGE_VC_SNAPSCENE_LOOPER, 5, SNAPSCENE, CURRENT, 5, 0, 0}, // ** Switch 05 **
      {PAGE_VC_SNAPSCENE_LOOPER, 6, SNAPSCENE, CURRENT, 6, 0, 0}, // ** Switch 06 **
      {PAGE_VC_SNAPSCENE_LOOPER, 7, SNAPSCENE, CURRENT, 7, 0, 0}, // ** Switch 07 **
      {PAGE_VC_SNAPSCENE_LOOPER, 8, SNAPSCENE, CURRENT, 8, 0, 0}, // ** Switch 08 **
      {PAGE_VC_SNAPSCENE_LOOPER, 9, LOOPER, CURRENT, LOOPER_REC_OVERDUB}, // ** Switch 09 **
      {PAGE_VC_SNAPSCENE_LOOPER, 10, LOOPER, CURRENT, LOOPER_PLAY_STOP}, // ** Switch 10 **
      //{PAGE_VC_SNAPSCENE_LOOPER, 11, PARAMETER, CURRENT, 10, MOMENTARY, 127, 0}, // ** Switch 11 **
      //{PAGE_VC_SNAPSCENE_LOOPER, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_SNAPSCENE_LOOPER, 13, PATCH, CURRENT, PREV}, // ** Switch 13 **
      {PAGE_VC_SNAPSCENE_LOOPER, 14, PATCH, CURRENT, NEXT}, // ** Switch 14 **
      //{PAGE_VC_SNAPSCENE_LOOPER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_SNAPSCENE_LOOPER, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

      // ******************************* PAGE 219: KATANA_patch_bank (8 buttons per page) *************************************************
      {PAGE_VC_KTN_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', 'B', 'A', 'N' },
      {PAGE_VC_KTN_PATCH_BANK, LABEL, 'K', ' ', 'K', 'A', 'T', 'A', 'N', 'A' },
      {PAGE_VC_KTN_PATCH_BANK, 1, PATCH, KTN, BANKSELECT, 2, 8}, // ** Switch 01 **
      {PAGE_VC_KTN_PATCH_BANK, 2, PATCH, KTN, BANKSELECT, 3, 8}, // ** Switch 02 **
      {PAGE_VC_KTN_PATCH_BANK, 3, PATCH, KTN, BANKSELECT, 4, 8}, // ** Switch 03 **
      {PAGE_VC_KTN_PATCH_BANK, 4, PATCH, KTN, BANKSELECT, 5, 8}, // ** Switch 04 **
      {PAGE_VC_KTN_PATCH_BANK, 5, PATCH, KTN, BANKSELECT, 6, 8}, // ** Switch 05 **
      {PAGE_VC_KTN_PATCH_BANK, 6, PATCH, KTN, BANKSELECT, 7, 8}, // ** Switch 06 **
      {PAGE_VC_KTN_PATCH_BANK, 7, PATCH, KTN, BANKSELECT, 8, 8}, // ** Switch 07 **
      {PAGE_VC_KTN_PATCH_BANK, 8, PATCH, KTN, BANKSELECT, 9, 8}, // ** Switch 08 **
      {PAGE_VC_KTN_PATCH_BANK, 9, PATCH, KTN, SELECT, 0, 0}, // ** Switch 09 **
      {PAGE_VC_KTN_PATCH_BANK, 10, OPEN_PAGE_DEVICE, KTN, PAGE_VC_KTN_EDIT}, // ** Switch 10 **
      //{PAGE_VC_KTN_PATCH_BANK, 11, PARAMETER, KTN, 1, TOGGLE, 1, 0}, // ** Switch 11 **
      //{PAGE_VC_KTN_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_KTN_PATCH_BANK, 13, PATCH, KTN, BANKDOWN, 8}, // ** Switch 13 **
      {PAGE_VC_KTN_PATCH_BANK, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
      {PAGE_VC_KTN_PATCH_BANK, 14, PATCH, KTN, BANKUP, 8}, // ** Switch 14 **
      {PAGE_VC_KTN_PATCH_BANK, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
      //{PAGE_VC_KTN_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_KTN_PATCH_BANK, 16, OPEN_NEXT_PAGE_OF_DEVICE, KTN}, // ** Switch 16 **

      // ******************************* PAGE 220: KATANA Edit *************************************************
      {PAGE_VC_KTN_EDIT, LABEL, 'K', 'A', 'T', 'A', 'N', 'A', ' ', 'E'},
      {PAGE_VC_KTN_EDIT, LABEL, 'D', 'I', 'T', ' ', ' ', ' ', ' ', ' '},
      {PAGE_VC_KTN_EDIT, 1, PAR_BANK_CATEGORY, KTN, 1}, // ** Switch 01 **
      {PAGE_VC_KTN_EDIT, 2, PAR_BANK_CATEGORY, KTN, 2}, // ** Switch 02 **
      {PAGE_VC_KTN_EDIT, 3, PAR_BANK_CATEGORY, KTN, 3}, // ** Switch 03 **
      {PAGE_VC_KTN_EDIT, 4, PAR_BANK_CATEGORY, KTN, 4}, // ** Switch 04 **
      {PAGE_VC_KTN_EDIT, 5, PAR_BANK_CATEGORY, KTN, 5}, // ** Switch 05 **
      {PAGE_VC_KTN_EDIT, 6, PAR_BANK_CATEGORY, KTN, 6}, // ** Switch 06 **
      {PAGE_VC_KTN_EDIT, 7, PAR_BANK_CATEGORY, KTN, 7}, // ** Switch 07 **
      {PAGE_VC_KTN_EDIT, 8, PAR_BANK_CATEGORY, KTN, 8}, // ** Switch 08 **
      {PAGE_VC_KTN_EDIT, 9, PAR_BANK_CATEGORY, KTN, 9}, // ** Switch 09 **
      {PAGE_VC_KTN_EDIT, 10, PAR_BANK_CATEGORY, KTN, 10}, // ** Switch 10 **
      {PAGE_VC_KTN_EDIT, 11, PAR_BANK_CATEGORY, KTN, 11 }, // ** Switch 11 **
      {PAGE_VC_KTN_EDIT, 12, SAVE_PATCH, KTN }, // ** Switch 11 **
      {PAGE_VC_KTN_EDIT, 13, PATCH, KTN, PREV}, // ** Switch 10 **
      {PAGE_VC_KTN_EDIT, 14, PATCH, KTN, NEXT}, // ** Switch 11 **
      //{PAGE_VC_KTN_EDIT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 12 **
      {PAGE_VC_KTN_EDIT, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 12 **

      // ******************************* PAGE 221: KATANA FX CTRL  *************************************************
      {PAGE_VC_KTN_FX, LABEL, 'K', 'A', 'T', 'A', 'N', 'A', ' ', 'F'},
      {PAGE_VC_KTN_FX, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', ' '},
      {PAGE_VC_KTN_FX, 1, PARAMETER, KTN, 0, TOGGLE, 1, 0}, // ** Switch 01 **
      {PAGE_VC_KTN_FX, 2, PARAMETER, KTN, 8, TOGGLE, 1, 0}, // ** Switch 02 **
      {PAGE_VC_KTN_FX, 3, PARAMETER, KTN, 23, TOGGLE, 1, 0}, // ** Switch 03 **
      {PAGE_VC_KTN_FX, 4, PARAMETER, KTN, 70, TOGGLE, 1, 0}, // ** Switch 04 **
      {PAGE_VC_KTN_FX, 5, PARAMETER, KTN, 61, TOGGLE, 1, 0}, // ** Switch 05 **
      {PAGE_VC_KTN_FX, 6, PARAMETER, KTN, 87, TOGGLE, 1, 0}, // ** Switch 06 **
      {PAGE_VC_KTN_FX, 7, PARAMETER, KTN, 96, TOGGLE, 1, 0}, // ** Switch 07 **
      {PAGE_VC_KTN_FX, 8, PARAMETER, KTN, 48, TOGGLE, 1, 0}, // ** Switch 08 **
      {PAGE_VC_KTN_FX, 9, PARAMETER, KTN, 106, TOGGLE, 1, 0}, // ** Switch 09 **
      {PAGE_VC_KTN_FX, 10, OPEN_PAGE_DEVICE, KTN, PAGE_VC_KTN_EDIT}, // ** Switch 10 **
      //{PAGE_VC_KTN_FX, 11, PAR_BANK_CATEGORY, KTN, 11 }, // ** Switch 11 **
      //{PAGE_VC_KTN_FX, 12, PAR_BANK_CATEGORY, KTN, 12}, // ** Switch 12 **
      {PAGE_VC_KTN_FX, 13, PATCH, KTN, PREV}, // ** Switch 10 **
      {PAGE_VC_KTN_FX, 14, PATCH, KTN, NEXT}, // ** Switch 11 **
      //{PAGE_VC_KTN_FX, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 12 **
      {PAGE_VC_KTN_FX, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 12 **

      // ******************************* PAGE 222: KPA Rig select *************************************************
      {PAGE_VC_KPA_RIG_SELECT, LABEL, 'K', 'P', 'A', ' ', 'R', 'I', 'G', ' ' },
      {PAGE_VC_KPA_RIG_SELECT, LABEL, 'S', 'E', 'L', 'E', 'C', 'T', ' ', ' ' },
      {PAGE_VC_KPA_RIG_SELECT, 1, SNAPSCENE, CURRENT, 1, 6, 0}, // ** Switch 01 **
      {PAGE_VC_KPA_RIG_SELECT, 2, SNAPSCENE, CURRENT, 2, 7, 0}, // ** Switch 02 **
      {PAGE_VC_KPA_RIG_SELECT, 3, SNAPSCENE, CURRENT, 3, 8, 0}, // ** Switch 03 **
      {PAGE_VC_KPA_RIG_SELECT, 4, SNAPSCENE, CURRENT, 4, 9, 0}, // ** Switch 04 **
      {PAGE_VC_KPA_RIG_SELECT, 5, PARAMETER, KPA, 0, TOGGLE, 1, 0}, // ** Switch 05 **
      {PAGE_VC_KPA_RIG_SELECT, 6, PARAMETER, KPA, 4, TOGGLE, 1, 0}, // ** Switch 06 **
      {PAGE_VC_KPA_RIG_SELECT, 7, PARAMETER, KPA, 5, TOGGLE, 1, 0}, // ** Switch 07 **
      {PAGE_VC_KPA_RIG_SELECT, 8, SNAPSCENE, CURRENT, 5, 10, 0}, // ** Switch 08 **
      {PAGE_VC_KPA_RIG_SELECT, 9, PATCH, KPA, BANKSELECT, 1, 4}, // ** Switch 09 **
      {PAGE_VC_KPA_RIG_SELECT, 10, PATCH, KPA, BANKSELECT, 2, 4}, // ** Switch 10 **
      {PAGE_VC_KPA_RIG_SELECT, 11, PATCH, KPA, BANKSELECT, 3, 4}, // ** Switch 11 **
      {PAGE_VC_KPA_RIG_SELECT, 12, PATCH, KPA, BANKSELECT, 4, 4}, // ** Switch 12 **
      {PAGE_VC_KPA_RIG_SELECT, 13, PATCH, KPA, BANKDOWN, 4}, // ** Switch 11 **
      {PAGE_VC_KPA_RIG_SELECT, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 11 **
      {PAGE_VC_KPA_RIG_SELECT, 14, PATCH, KPA, BANKUP, 4}, // ** Switch 12 **
      {PAGE_VC_KPA_RIG_SELECT, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 12 **
      //{PAGE_VC_KPA_RIG_SELECT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_KPA_RIG_SELECT, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

      // ******************************* PAGE 223: KPA FX control *************************************************
      {PAGE_VC_KPA_FX_CONTROL, LABEL, 'K', 'P', 'A', ' ', 'F', 'X', ' ', 'C' },
      {PAGE_VC_KPA_FX_CONTROL, LABEL, 'O', 'N', 'T', 'R', 'O', 'L', ' ', ' ' },
      {PAGE_VC_KPA_FX_CONTROL, 1, PARAMETER, KPA, 0, TOGGLE, 1, 0}, // ** Switch 01 **
      {PAGE_VC_KPA_FX_CONTROL, 2, PARAMETER, KPA, 1, TOGGLE, 1, 0}, // ** Switch 02 **
      {PAGE_VC_KPA_FX_CONTROL, 3, PARAMETER, KPA, 2, TOGGLE, 1, 0}, // ** Switch 03 **
      {PAGE_VC_KPA_FX_CONTROL, 4, PARAMETER, KPA, 3, TOGGLE, 1, 0}, // ** Switch 04 **
      {PAGE_VC_KPA_FX_CONTROL, 5, PARAMETER, KPA, 4, TOGGLE, 1, 0}, // ** Switch 05 **
      {PAGE_VC_KPA_FX_CONTROL, 6, PARAMETER, KPA, 5, TOGGLE, 1, 0}, // ** Switch 06 **
      {PAGE_VC_KPA_FX_CONTROL, 7, PARAMETER, KPA, 6, TOGGLE, 1, 0}, // ** Switch 07 **
      {PAGE_VC_KPA_FX_CONTROL, 8, PARAMETER, KPA, 7, TOGGLE, 1, 0}, // ** Switch 09 **
      {PAGE_VC_KPA_FX_CONTROL, 9, LOOPER, CURRENT, LOOPER_REC_PLAY_OVERDUB}, // ** Switch 08 **
      {PAGE_VC_KPA_FX_CONTROL, 10 | ON_RELEASE, LOOPER, CURRENT, LOOPER_STOP_ERASE}, // ** Switch 10 **
      {PAGE_VC_KPA_FX_CONTROL, 10 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_KPA_LOOPER},
      {PAGE_VC_KPA_FX_CONTROL, 13, PARAMETER, KPA, 18, ONE_SHOT, 0, 0}, // ** Switch 11 **
      {PAGE_VC_KPA_FX_CONTROL, 14, PARAMETER, KPA, 17, ONE_SHOT, 0, 0}, // ** Switch 12 **
      //{PAGE_VC_KPA_FX_CONTROL, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_KPA_FX_CONTROL, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

      // ******************************* PAGE 224: KPA Looper control *************************************************
      {PAGE_VC_KPA_LOOPER, LABEL, 'K', 'P', 'A', ' ', 'L', 'O', 'O', 'P' },
      {PAGE_VC_KPA_LOOPER, LABEL, 'E', 'R', ' ', 'C', 'T', 'L', ' ', ' ' },
      {PAGE_VC_KPA_LOOPER, 1, LOOPER, CURRENT, LOOPER_REC_PLAY_OVERDUB}, // ** Switch 01 **
      {PAGE_VC_KPA_LOOPER, 2, LOOPER, CURRENT, LOOPER_STOP_ERASE}, // ** Switch 02 **
      {PAGE_VC_KPA_LOOPER, 3, PARAMETER, KPA, 0, TOGGLE, 1, 0}, // ** Switch 03 **
      {PAGE_VC_KPA_LOOPER, 4, PARAMETER, KPA, 1, TOGGLE, 1, 0}, // ** Switch 04 **
      {PAGE_VC_KPA_LOOPER, 5, LOOPER, CURRENT, LOOPER_UNDO_REDO}, // ** Switch 05 **
      {PAGE_VC_KPA_LOOPER, 6, LOOPER, CURRENT, LOOPER_PLAY_ONCE}, // ** Switch 06 **
      {PAGE_VC_KPA_LOOPER, 7, PARAMETER, KPA, 5, TOGGLE, 1, 0}, // ** Switch 07 **
      {PAGE_VC_KPA_LOOPER, 8, PARAMETER, KPA, 6, TOGGLE, 1, 0}, // ** Switch 08 **
      {PAGE_VC_KPA_LOOPER, 9, LOOPER, CURRENT, LOOPER_HALF_SPEED}, // ** Switch 09 **
      {PAGE_VC_KPA_LOOPER, 10, LOOPER, CURRENT, LOOPER_REVERSE}, // ** Switch 10 **
      //{PAGE_VC_KPA_LOOPER, 11, PATCH, CURRENT, BANKSELECT, 3, 3}, // ** Switch 11 **
      {PAGE_VC_KPA_LOOPER, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_KPA_LOOPER, 13, PATCH, CURRENT, PREV}, // ** Switch 13 **
      {PAGE_VC_KPA_LOOPER, 14, PATCH, CURRENT, NEXT}, // ** Switch 14 **
      //{PAGE_VC_KPA_LOOPER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_KPA_LOOPER, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

      // ******************************* PAGE 225: Parameters current device *************************************************
      {PAGE_VC_CURRENT_ASSIGN, LABEL, 'A', 'S', 'G', ' ', 'B', 'A', 'N', 'K' },
      {PAGE_VC_CURRENT_ASSIGN, 1, ASSIGN, CURRENT, BANKSELECT, 1, 10}, // ** Switch 01 **
      {PAGE_VC_CURRENT_ASSIGN, 2, ASSIGN, CURRENT, BANKSELECT, 2, 10}, // ** Switch 02 **
      {PAGE_VC_CURRENT_ASSIGN, 3, ASSIGN, CURRENT, BANKSELECT, 3, 10}, // ** Switch 03 **
      {PAGE_VC_CURRENT_ASSIGN, 4, ASSIGN, CURRENT, BANKSELECT, 4, 10}, // ** Switch 04 **
      {PAGE_VC_CURRENT_ASSIGN, 5, ASSIGN, CURRENT, BANKSELECT, 5, 10}, // ** Switch 05 **
      {PAGE_VC_CURRENT_ASSIGN, 6, ASSIGN, CURRENT, BANKSELECT, 6, 10}, // ** Switch 06 **
      {PAGE_VC_CURRENT_ASSIGN, 7, ASSIGN, CURRENT, BANKSELECT, 7, 10}, // ** Switch 07 **
      {PAGE_VC_CURRENT_ASSIGN, 8, ASSIGN, CURRENT, BANKSELECT, 8, 10}, // ** Switch 08 **
      {PAGE_VC_CURRENT_ASSIGN, 9, ASSIGN, CURRENT, BANKSELECT, 9, 10}, // ** Switch 09 **
      {PAGE_VC_CURRENT_ASSIGN, 10, ASSIGN, CURRENT, BANKSELECT, 10, 10}, // ** Switch 10 **
      //{PAGE_VC_CURRENT_ASSIGN, 11, NOTHING, COMMON}, // ** Switch 11 **
      //{PAGE_VC_CURRENT_ASSIGN, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_CURRENT_ASSIGN, 13, ASSIGN, CURRENT, BANKDOWN, 10}, // ** Switch 13 **
      {PAGE_VC_CURRENT_ASSIGN, 14, ASSIGN, CURRENT, BANKUP, 10}, // ** Switch 14 **
      //{PAGE_VC_CURRENT_ASSIGN, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_CURRENT_ASSIGN, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

      // ******************************* PAGE 226: SY1000_patch_bank (8 buttons per page) *************************************************
      {PAGE_VC_SY1000_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'N' },
      {PAGE_VC_SY1000_PATCH_BANK, LABEL, 'K', ' ', 'S', 'Y', '1', '0', '0', '0' },
      {PAGE_VC_SY1000_PATCH_BANK, 1, PATCH, SY1000, BANKSELECT, 1, 8}, // ** Switch 01 **
      {PAGE_VC_SY1000_PATCH_BANK, 2, PATCH, SY1000, BANKSELECT, 2, 8}, // ** Switch 02 **
      {PAGE_VC_SY1000_PATCH_BANK, 3, PATCH, SY1000, BANKSELECT, 3, 8}, // ** Switch 03 **
      {PAGE_VC_SY1000_PATCH_BANK, 4, PATCH, SY1000, BANKSELECT, 4, 8}, // ** Switch 04 **
      {PAGE_VC_SY1000_PATCH_BANK, 5, PATCH, SY1000, BANKSELECT, 5, 8}, // ** Switch 05 **
      {PAGE_VC_SY1000_PATCH_BANK, 6, PATCH, SY1000, BANKSELECT, 6, 8}, // ** Switch 06 **
      {PAGE_VC_SY1000_PATCH_BANK, 7, PATCH, SY1000, BANKSELECT, 7, 8}, // ** Switch 07 **
      {PAGE_VC_SY1000_PATCH_BANK, 8, PATCH, SY1000, BANKSELECT, 8, 8}, // ** Switch 08 **
      {PAGE_VC_SY1000_PATCH_BANK, 9, ASSIGN, SY1000, SELECT, 0, 0}, // ** Switch 09 **
      {PAGE_VC_SY1000_PATCH_BANK, 10, ASSIGN, SY1000, SELECT, 1, 1}, // ** Switch 10 **
      //{PAGE_VC_SY1000_PATCH_BANK, 11, PARAMETER, SY1000, 1, TOGGLE, 1, 0}, // ** Switch 11 **
      //{PAGE_VC_SY1000_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_SY1000_PATCH_BANK, 13, PATCH, SY1000, BANKDOWN, 8}, // ** Switch 13 **
      {PAGE_VC_SY1000_PATCH_BANK, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
      {PAGE_VC_SY1000_PATCH_BANK, 14, PATCH, SY1000, BANKUP, 8}, // ** Switch 14 **
      {PAGE_VC_SY1000_PATCH_BANK, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
      //{PAGE_VC_SY1000_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_SY1000_PATCH_BANK, 16, OPEN_NEXT_PAGE_OF_DEVICE, SY1000}, // ** Switch 16 **

      // ******************************* PAGE 227: SY1000 assign *************************************************
      {PAGE_VC_SY1000_ASSIGNS, LABEL, 'A', 'S', 'S', 'I', 'G', 'N', 'S', ' ' },
      {PAGE_VC_SY1000_ASSIGNS, LABEL, 'S', 'Y', '1', '0', '0', '0', ' ', ' ' },
      {PAGE_VC_SY1000_ASSIGNS, 1, ASSIGN, SY1000, BANKSELECT, 1, 8}, // ** Switch 01 **
      {PAGE_VC_SY1000_ASSIGNS, 2, ASSIGN, SY1000, BANKSELECT, 2, 8}, // ** Switch 02 **
      {PAGE_VC_SY1000_ASSIGNS, 3, ASSIGN, SY1000, BANKSELECT, 3, 8}, // ** Switch 03 **
      {PAGE_VC_SY1000_ASSIGNS, 4, ASSIGN, SY1000, BANKSELECT, 4, 8}, // ** Switch 04 **
      {PAGE_VC_SY1000_ASSIGNS, 5, ASSIGN, SY1000, BANKSELECT, 5, 8}, // ** Switch 05 **
      {PAGE_VC_SY1000_ASSIGNS, 6, ASSIGN, SY1000, BANKSELECT, 6, 8}, // ** Switch 06 **
      {PAGE_VC_SY1000_ASSIGNS, 7, ASSIGN, SY1000, BANKSELECT, 7, 8}, // ** Switch 07 **
      {PAGE_VC_SY1000_ASSIGNS, 8, ASSIGN, SY1000, BANKSELECT, 8, 8}, // ** Switch 08 **
      {PAGE_VC_SY1000_ASSIGNS, 9, PATCH, SY1000, PREV, 1}, // ** Switch 09 **
      {PAGE_VC_SY1000_ASSIGNS, 10, PATCH, SY1000, NEXT, 1}, // ** Switch 10 **
      //{PAGE_VC_SY1000_ASSIGNS, 11, NOTHING, COMMON}, // ** Switch 11 **
      //{PAGE_VC_SY1000_ASSIGNS, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_SY1000_ASSIGNS, 13, ASSIGN, SY1000, BANKDOWN, 8}, // ** Switch 13 **
      {PAGE_VC_SY1000_ASSIGNS, 14, ASSIGN, SY1000, BANKUP, 8}, // ** Switch 14 **
      //{PAGE_VC_SY1000_ASSIGNS, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_SY1000_ASSIGNS, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

      // ******************************* PAGE 228: Scenes (SY1000) *************************************************
      {PAGE_VC_SY1000_SCENES, LABEL, 'S', 'C', 'E', 'N', 'E', 'S', ' ', 'S' },
      {PAGE_VC_SY1000_SCENES, LABEL, 'Y', '1', '0', '0', '0', ' ', ' ', ' ' },
      {PAGE_VC_SY1000_SCENES, 1, SNAPSCENE, CURRENT, 1, 0, 0}, // ** Switch 01 **
      {PAGE_VC_SY1000_SCENES, 2, SNAPSCENE, CURRENT, 2, 0, 0}, // ** Switch 02 **
      {PAGE_VC_SY1000_SCENES, 3, SNAPSCENE, CURRENT, 3, 0, 0}, // ** Switch 03 **
      {PAGE_VC_SY1000_SCENES, 4, SNAPSCENE, CURRENT, 4, 0, 0}, // ** Switch 04 **
      {PAGE_VC_SY1000_SCENES, 5, SNAPSCENE, CURRENT, 5, 0, 0}, // ** Switch 05 **
      {PAGE_VC_SY1000_SCENES, 6, SNAPSCENE, CURRENT, 6, 0, 0}, // ** Switch 06 **
      {PAGE_VC_SY1000_SCENES, 7, SNAPSCENE, CURRENT, 7, 0, 0}, // ** Switch 07 **
      {PAGE_VC_SY1000_SCENES, 8, SNAPSCENE, CURRENT, 8, 0, 0}, // ** Switch 08 **
      //{PAGE_VC_SY1000_SCENES, 9, SAVE_PATCH, SY1000}, // ** Switch 09 **
      {PAGE_VC_SY1000_SCENES, 10, SAVE_PATCH, CURRENT}, // ** Switch 10 **
      //{PAGE_VC_SY1000_SCENES, 11, PARAMETER, CURRENT, 10, MOMENTARY, 127, 0}, // ** Switch 11 **
      //{PAGE_VC_SY1000_SCENES, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_SY1000_SCENES, 13, PATCH, CURRENT, PREV}, // ** Switch 13 **
      {PAGE_VC_SY1000_SCENES, 14, PATCH, CURRENT, NEXT}, // ** Switch 14 **
      //{PAGE_VC_SY1000_SCENES, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_SY1000_SCENES, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

      // ******************************* PAGE 229: MG300_patch_bank (8 buttons per page) *************************************************
      {PAGE_VC_MG300_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
      {PAGE_VC_MG300_PATCH_BANK, LABEL, 'N', 'K', ' ', 'M', 'G', '3', '0', '0' },
      {PAGE_VC_MG300_PATCH_BANK, 1, PATCH, MG300, BANKSELECT, 1, 8}, // ** Switch 01 **
      {PAGE_VC_MG300_PATCH_BANK, 2, PATCH, MG300, BANKSELECT, 2, 8}, // ** Switch 02 **
      {PAGE_VC_MG300_PATCH_BANK, 3, PATCH, MG300, BANKSELECT, 3, 8}, // ** Switch 03 **
      {PAGE_VC_MG300_PATCH_BANK, 4, PATCH, MG300, BANKSELECT, 4, 8}, // ** Switch 04 **
      {PAGE_VC_MG300_PATCH_BANK, 5, PATCH, MG300, BANKSELECT, 5, 8}, // ** Switch 05 **
      {PAGE_VC_MG300_PATCH_BANK, 6, PATCH, MG300, BANKSELECT, 6, 8}, // ** Switch 06 **
      {PAGE_VC_MG300_PATCH_BANK, 7, PATCH, MG300, BANKSELECT, 7, 8}, // ** Switch 07 **
      {PAGE_VC_MG300_PATCH_BANK, 8, PATCH, MG300, BANKSELECT, 8, 8}, // ** Switch 08 **
      //{PAGE_VC_MG300_PATCH_BANK, 9, PARAMETER, MG300, 14, STEP, 0, 6, 1}, // ** Switch 09 **
      //{PAGE_VC_MG300_PATCH_BANK, 10, PAGE, COMMON, SELECT, PAGE_VC_FULL_LOOPER}, // ** Switch 10 **
      //{PAGE_VC_MG300_PATCH_BANK, 11, PARAMETER, MG300, 1, TOGGLE, 1, 0}, // ** Switch 11 **
      //{PAGE_VC_MG300_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
      {PAGE_VC_MG300_PATCH_BANK, 13, PATCH, MG300, BANKDOWN, 8}, // ** Switch 13 **
      {PAGE_VC_MG300_PATCH_BANK, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
      {PAGE_VC_MG300_PATCH_BANK, 14, PATCH, MG300, BANKUP, 8}, // ** Switch 14 **
      {PAGE_VC_MG300_PATCH_BANK, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
      //{PAGE_VC_MG300_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      {PAGE_VC_MG300_PATCH_BANK, 16, OPEN_NEXT_PAGE_OF_DEVICE, MG300}, // ** Switch 16 **

      // ******************************* PAGE 230: SONG SELECT *************************************************
      {PAGE_VC_SONG_MODE, LABEL, 'S', 'O', 'N', 'G', ' ', 'M', 'O', 'D' },
      {PAGE_VC_SONG_MODE, LABEL, 'E', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VC_SONG_MODE, 1, SONG, COMMON, SONG_PARTSEL, 0}, // ** Switch 01 **
      {PAGE_VC_SONG_MODE, 2, SONG, COMMON, SONG_PARTSEL, 1}, // ** Switch 02 **
      {PAGE_VC_SONG_MODE, 3, SONG, COMMON, SONG_PARTSEL, 2}, // ** Switch 03 **
      {PAGE_VC_SONG_MODE, 4, SONG, COMMON, SONG_PARTSEL, 3}, // ** Switch 04 **
      {PAGE_VC_SONG_MODE, 5, SONG, COMMON, SONG_PARTSEL, 4}, // ** Switch 05 **
      {PAGE_VC_SONG_MODE, 6, SONG, COMMON, SONG_PARTSEL, 5}, // ** Switch 06 **
      {PAGE_VC_SONG_MODE, 7, SONG, COMMON, SONG_PARTSEL, 6}, // ** Switch 07 **
      {PAGE_VC_SONG_MODE, 8, SONG, COMMON, SONG_PARTSEL, 7}, // ** Switch 08 **
      {PAGE_VC_SONG_MODE, 9, MODE, COMMON, SELECT, SONG_MODE}, // ** Switch 09 **
      {PAGE_VC_SONG_MODE, 10, MODE, COMMON, SELECT, PAGE_MODE}, // ** Switch 10 **
      {PAGE_VC_SONG_MODE, 11, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 11 **
      {PAGE_VC_SONG_MODE, 12, PAGE, COMMON, SELECT, PAGE_VC_SETLIST_SELECT}, // ** Switch 12 **
      {PAGE_VC_SONG_MODE, 13, SONG, COMMON, SONG_PREV, SONG_PREVNEXT_SONG}, // ** Switch 13 **
      {PAGE_VC_SONG_MODE, 13 | ON_LONG_PRESS, PAGE, COMMON, SELECT, PAGE_VC_SONG_SELECT},
      {PAGE_VC_SONG_MODE, 14, SONG, COMMON, SONG_NEXT, SONG_PREVNEXT_SONG}, // ** Switch 14 **
      {PAGE_VC_SONG_MODE, 14 | ON_LONG_PRESS, PAGE, COMMON, SELECT, PAGE_VC_SONG_SELECT},

      // ******************************* PAGE 231: SONG SELECT *************************************************
      {PAGE_VC_SONG_SELECT, LABEL, 'S', 'O', 'N', 'G', ' ', 'S', 'E', 'L' },
      {PAGE_VC_SONG_SELECT, LABEL, 'E', 'C', 'T', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VC_SONG_SELECT, 1, SONG, COMMON, SONG_BANKSELECT, 0, 8}, // ** Switch 01 **
      {PAGE_VC_SONG_SELECT, 1, PAGE, COMMON, SELECT, 0},
      {PAGE_VC_SONG_SELECT, 2, SONG, COMMON, SONG_BANKSELECT, 1, 8}, // ** Switch 02 **
      {PAGE_VC_SONG_SELECT, 2, PAGE, COMMON, SELECT, 0},
      {PAGE_VC_SONG_SELECT, 3, SONG, COMMON, SONG_BANKSELECT, 2, 8}, // ** Switch 03 **
      {PAGE_VC_SONG_SELECT, 3, PAGE, COMMON, SELECT, 0},
      {PAGE_VC_SONG_SELECT, 4, SONG, COMMON, SONG_BANKSELECT, 3, 8}, // ** Switch 04 **
      {PAGE_VC_SONG_SELECT, 4, PAGE, COMMON, SELECT, 0},
      {PAGE_VC_SONG_SELECT, 5, SONG, COMMON, SONG_BANKSELECT, 4, 8}, // ** Switch 05 **
      {PAGE_VC_SONG_SELECT, 5, PAGE, COMMON, SELECT, 0},
      {PAGE_VC_SONG_SELECT, 6, SONG, COMMON, SONG_BANKSELECT, 5, 8}, // ** Switch 06 **
      {PAGE_VC_SONG_SELECT, 6, PAGE, COMMON, SELECT, 0},
      {PAGE_VC_SONG_SELECT, 7, SONG, COMMON, SONG_BANKSELECT, 6, 8}, // ** Switch 07 **
      {PAGE_VC_SONG_SELECT, 7, PAGE, COMMON, SELECT, 0},
      {PAGE_VC_SONG_SELECT, 8, SONG, COMMON, SONG_BANKSELECT, 7, 8}, // ** Switch 08 **
      {PAGE_VC_SONG_SELECT, 8, PAGE, COMMON, SELECT, 0},
      {PAGE_VC_SONG_SELECT, 9, MODE, COMMON, SELECT, SONG_MODE}, // ** Switch 09 **
      {PAGE_VC_SONG_SELECT, 10, MODE, COMMON, SELECT, PAGE_MODE}, // ** Switch 10 **
      {PAGE_VC_SONG_SELECT, 11, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 11 **
      {PAGE_VC_SONG_SELECT, 12, SONG, COMMON, SONG_EDIT}, // ** Switch 12 **
      {PAGE_VC_SONG_SELECT, 13, SONG, COMMON, SONG_BANKDOWN, 8}, // ** Switch 13 **
      {PAGE_VC_SONG_SELECT, 14, SONG, COMMON, SONG_BANKUP, 8}, // ** Switch 14 **
      {PAGE_VC_SONG_SELECT, 15, PAGE, COMMON, SELECT, 0}, // ** Switch 15 **
      {PAGE_VC_SONG_SELECT, 16, PAGE, COMMON, SELECT, 0 },

      // ******************************* PAGE 232: SETLIST SELECT *************************************************
      {PAGE_VC_SETLIST_SELECT, LABEL, 'S', 'E', 'T', 'L', 'I', 'S', 'T', ' ' },
      {PAGE_VC_SETLIST_SELECT, LABEL, 'S', 'E', 'L', 'E', 'C', 'T', ' ', ' ' },
      {PAGE_VC_SETLIST_SELECT, 1, SETLIST, COMMON, SL_BANKSELECT, 0, 8}, // ** Switch 01 **
      {PAGE_VC_SETLIST_SELECT, 2, SETLIST, COMMON, SL_BANKSELECT, 1, 8}, // ** Switch 02 **
      {PAGE_VC_SETLIST_SELECT, 3, SETLIST, COMMON, SL_BANKSELECT, 2, 8}, // ** Switch 03 **
      {PAGE_VC_SETLIST_SELECT, 4, SETLIST, COMMON, SL_BANKSELECT, 3, 8}, // ** Switch 04 **
      {PAGE_VC_SETLIST_SELECT, 5, SETLIST, COMMON, SL_BANKSELECT, 4, 8}, // ** Switch 05 **
      {PAGE_VC_SETLIST_SELECT, 6, SETLIST, COMMON, SL_BANKSELECT, 5, 8}, // ** Switch 06 **
      {PAGE_VC_SETLIST_SELECT, 7, SETLIST, COMMON, SL_BANKSELECT, 6, 8}, // ** Switch 07 **
      {PAGE_VC_SETLIST_SELECT, 8, SETLIST, COMMON, SL_BANKSELECT, 7, 8}, // ** Switch 08 **
      {PAGE_VC_SETLIST_SELECT, 9, MODE, COMMON, SELECT, SONG_MODE}, // ** Switch 09 **
      {PAGE_VC_SETLIST_SELECT, 10, MODE, COMMON, SELECT, PAGE_MODE}, // ** Switch 10 **
      {PAGE_VC_SETLIST_SELECT, 11, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 11 **
      {PAGE_VC_SETLIST_SELECT, 12, SETLIST, COMMON, SL_EDIT}, // ** Switch 12 **
      {PAGE_VC_SETLIST_SELECT, 13, SETLIST, COMMON, SL_BANKDOWN, 8}, // ** Switch 13 **
      {PAGE_VC_SETLIST_SELECT, 14, SETLIST, COMMON, SL_BANKUP, 8}, // ** Switch 14 **
      {PAGE_VC_SETLIST_SELECT, 15, PAGE, COMMON, SELECT, 0}, // ** Switch 15 **
      {PAGE_VC_SETLIST_SELECT, 16, PAGE, COMMON, SELECT, 0},
};

const uint16_t number_of_fixed_VC_commands = VC_fixed_commands.size();


// ********************************* Section 3: VController default configuration for programmable pages ********************************************

const QVector<Cmd_struct> VC_commands = {
    // ******************************* PAGE 00: Default page *************************************************
    // Page, Switch, Type, Device, Data1, Data2, Value1, Value2, Value3, Value4, Value5
    //{PAGE_VC_DEFAULT, LABEL, 'D', 'E', 'F', 'A', 'U', 'L', 'T', ' ' },
    {PAGE_VC_DEFAULT, 1, NOTHING, COMMON}, // ** Switch 01 **
    {PAGE_VC_DEFAULT, 2, NOTHING, COMMON}, // ** Switch 02 **
    {PAGE_VC_DEFAULT, 3, NOTHING, COMMON}, // ** Switch 03 **
    {PAGE_VC_DEFAULT, 4, NOTHING, COMMON}, // ** Switch 04 **
    {PAGE_VC_DEFAULT, 5, NOTHING, COMMON}, // ** Switch 05 **
    {PAGE_VC_DEFAULT, 6, NOTHING, COMMON}, // ** Switch 06 **
    {PAGE_VC_DEFAULT, 7, NOTHING, COMMON}, // ** Switch 07 **
    {PAGE_VC_DEFAULT, 8, NOTHING, COMMON}, // ** Switch 08 **
    {PAGE_VC_DEFAULT, 9, NOTHING, COMMON}, // ** Switch 09 **
    {PAGE_VC_DEFAULT, 10, NOTHING, COMMON}, // ** Switch 10 **
    {PAGE_VC_DEFAULT, 11, TOGGLE_EXP_PEDAL, CURRENT}, // ** Switch 11 **
    {PAGE_VC_DEFAULT, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
    {PAGE_VC_DEFAULT, 12 | ON_LONG_PRESS, GLOBAL_TUNER, COMMON}, // ** Switch 12 **
    {PAGE_VC_DEFAULT, 13, NOTHING, COMMON}, // ** Switch 13 **
    {PAGE_VC_DEFAULT, 14, NOTHING, COMMON}, // ** Switch 14 **
    {PAGE_VC_DEFAULT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
    {PAGE_VC_DEFAULT, 15 | ON_LONG_PRESS, PAGE, COMMON, SELECT, PAGE_VC_DEVICE_MODE}, // ** Switch 15 **
    {PAGE_VC_DEFAULT, 16, PAGE, COMMON, NEXT}, // ** Switch 16 **
    {PAGE_VC_DEFAULT, 16 | ON_LONG_PRESS, PAGE, COMMON, SELECT, PAGE_VC_DEVICE_MODE}, // ** Switch 15 **
    {PAGE_VC_DEFAULT, 17, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 1 or expr pedal 1
    {PAGE_VC_DEFAULT, 18, NOTHING, COMMON}, // External switch 2
    {PAGE_VC_DEFAULT, 19, NOTHING, COMMON}, // External switch 3 or expr pedal 2
    {PAGE_VC_DEFAULT, 20, NOTHING, COMMON}, // External switch 4
    {PAGE_VC_DEFAULT, 21, NOTHING, COMMON}, // External switch 5 or expr pedal 3
    {PAGE_VC_DEFAULT, 22, NOTHING, COMMON}, // External switch 6
    {PAGE_VC_DEFAULT, 23, PATCH, CURRENT, PREV}, // External switch 7 or expr pedal 4
    {PAGE_VC_DEFAULT, 24, PATCH, CURRENT, NEXT}, // External switch 8

    // ******************************* PAGE 01: GP+GR *************************************************
    {PAGE_VC_COMBO1, LABEL, 'G', 'P', '+', 'G', 'R', ' ', ' ', ' ' },
    {PAGE_VC_COMBO1, 1, PATCH, GP10, BANKSELECT, 1, 5}, // ** Switch 01 **
    {PAGE_VC_COMBO1, 2, PATCH, GP10, BANKSELECT, 2, 5}, // ** Switch 02 **
    {PAGE_VC_COMBO1, 3, PATCH, GP10, BANKSELECT, 3, 5}, // ** Switch 03 **
    {PAGE_VC_COMBO1, 4, PATCH, GP10, BANKSELECT, 4, 5}, // ** Switch 04 **
    {PAGE_VC_COMBO1, 5, PATCH, GR55, BANKSELECT, 1, 6}, // ** Switch 05 **
    {PAGE_VC_COMBO1, 6, PATCH, GR55, BANKSELECT, 2, 6}, // ** Switch 06 **
    {PAGE_VC_COMBO1, 7, PATCH, GR55, BANKSELECT, 3, 6}, // ** Switch 07 **
    {PAGE_VC_COMBO1, 8, PATCH, GP10, BANKSELECT, 5, 5}, // ** Switch 08 **
    {PAGE_VC_COMBO1, 9, PATCH, GR55, BANKSELECT, 4, 6}, // ** Switch 09 **
    {PAGE_VC_COMBO1, 10, PATCH, GR55, BANKSELECT, 5, 6}, // ** Switch 10 **
    {PAGE_VC_COMBO1, 11, PATCH, GR55, BANKSELECT, 6, 6}, // ** Switch 11 **
    //{PAGE_VC_COMBO1, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
    {PAGE_VC_COMBO1, 13, PATCH, CURRENT, BANKDOWN, 5}, // ** Switch 13 **
    {PAGE_VC_COMBO1, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
    {PAGE_VC_COMBO1, 14, PATCH, CURRENT, BANKUP, 5}, // ** Switch 14 **
    {PAGE_VC_COMBO1, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 14 **
    //Switch 15 and 16 are BANK_DOWN and BANK_UP

    // ******************************* PAGE 02: GPVGGR *************************************************
    {PAGE_VC_COMBO2, LABEL, 'G', 'P', 'V', 'G', 'G', 'R', ' ', ' ' },
    {PAGE_VC_COMBO2, 1, PATCH, GP10, BANKSELECT, 1, 4}, // ** Switch 01 **
    {PAGE_VC_COMBO2, 2, PATCH, GP10, BANKSELECT, 2, 4}, // ** Switch 02 **
    {PAGE_VC_COMBO2, 3, PATCH, GP10, BANKSELECT, 3, 4}, // ** Switch 03 **
    {PAGE_VC_COMBO2, 4, PATCH, GP10, BANKSELECT, 4, 4}, // ** Switch 04 **
    {PAGE_VC_COMBO2, 5, PATCH, VG99, BANKSELECT, 1, 4}, // ** Switch 05 **
    {PAGE_VC_COMBO2, 6, PATCH, VG99, BANKSELECT, 2, 4}, // ** Switch 06 **
    {PAGE_VC_COMBO2, 7, PATCH, VG99, BANKSELECT, 3, 4}, // ** Switch 07 **
    {PAGE_VC_COMBO2, 8, PATCH, VG99, BANKSELECT, 4, 4}, // ** Switch 08 **
    {PAGE_VC_COMBO2, 9, PATCH, GR55, BANKSELECT, 1, 3}, // ** Switch 09 **
    {PAGE_VC_COMBO2, 10, PATCH, GR55, BANKSELECT, 2, 3}, // ** Switch 10 **
    {PAGE_VC_COMBO2, 11, PATCH, GR55, BANKSELECT, 3, 3}, // ** Switch 11 **
    //{PAGE_VC_COMBO2, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
    {PAGE_VC_COMBO2, 13, PATCH, CURRENT, BANKDOWN, 4}, // ** Switch 13 **
    {PAGE_VC_COMBO2, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
    {PAGE_VC_COMBO2, 14, PATCH, CURRENT, BANKUP, 4}, // ** Switch 14 **
    {PAGE_VC_COMBO2, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VC_CURRENT_DIRECT_SELECT}, // ** Switch 14 **
    //Switch 15 and 16 are BANK_DOWN and BANK_UP

    // ******************************* PAGE 03: FUNCTION TEST *************************************************
    {PAGE_VC_FUNCTIONS_TEST, LABEL, 'F', 'U', 'N', 'C', 'T', 'I', 'O', 'N' },
    {PAGE_VC_FUNCTIONS_TEST, LABEL, ' ', 'T', 'E', 'S', 'T', ' ', ' ', ' ' },
    {PAGE_VC_FUNCTIONS_TEST, 1, PATCH, CURRENT, SELECT, 1, 0}, // ** Switch 01 **
    {PAGE_VC_FUNCTIONS_TEST, 2, PATCH, CURRENT, BANKDOWN, 1}, // ** Switch 02 **
    {PAGE_VC_FUNCTIONS_TEST, 3, PATCH, CURRENT, BANKSELECT, 1, 1}, // ** Switch 03 **
    {PAGE_VC_FUNCTIONS_TEST, 4, PATCH, CURRENT, BANKUP, 1}, // ** Switch 04 **
    {PAGE_VC_FUNCTIONS_TEST, 5, PARAMETER, CURRENT, 0, TOGGLE, 1, 0}, // ** Switch 05 **
    {PAGE_VC_FUNCTIONS_TEST, 6, PARAMETER, CURRENT, 1, TOGGLE, 1, 0}, // ** Switch 06 **
    {PAGE_VC_FUNCTIONS_TEST, 7, PATCH, CURRENT, NEXT}, // ** Switch 07 **
    {PAGE_VC_FUNCTIONS_TEST, 8, MUTE, CURRENT}, // ** Switch 08 **
    {PAGE_VC_FUNCTIONS_TEST, 9, GLOBAL_TUNER, COMMON}, // ** Switch 09 **
    {PAGE_VC_FUNCTIONS_TEST, 10, SET_TEMPO, COMMON, 95}, // ** Switch 10 **
    {PAGE_VC_FUNCTIONS_TEST, 11, SET_TEMPO, COMMON, 120}, // ** Switch 11 **
    //{PAGE_VC_FUNCTIONS_TEST, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
    //{PAGE_VC_FUNCTIONS_TEST, 13, PATCH, CURRENT, BANKDOWN, 4}, // ** Switch 13 **
    //{PAGE_VC_FUNCTIONS_TEST, 14, PATCH, CURRENT, BANKUP, 4}, // ** Switch 14 **
    //Switch 15 and 16 are BANK_DOWN and BANK_UP

    // ******************************* PAGE 04: GM TEST *************************************************
    {PAGE_VC_GM_TEST, LABEL, 'G', 'E', 'N', '.', 'M', 'I', 'D', 'I' },
    {PAGE_VC_GM_TEST, LABEL, ' ', 'T', 'E', 'S', 'T', ' ', ' ', ' ' },
    {PAGE_VC_GM_TEST, 1, MIDI_PC, COMMON, 1, 1, PORT3_NUMBER}, // ** Switch 01 **
    {PAGE_VC_GM_TEST, 2, MIDI_PC, COMMON, 2, 1, PORT3_NUMBER}, // ** Switch 02 **
    {PAGE_VC_GM_TEST, 3, MIDI_PC, COMMON, 3, 1, PORT3_NUMBER}, // ** Switch 03 **
    {PAGE_VC_GM_TEST, 3 | LABEL, 'C', 'U', 'S', 'T', 'O', 'M', ' ', 'L'}, // ** Switch 03 **
    {PAGE_VC_GM_TEST, 3 | LABEL, 'A', 'B', 'E', 'L', ' ', 'P', 'C', '3'}, // ** Switch 03 **
    {PAGE_VC_GM_TEST, 4, MIDI_NOTE, COMMON, 52, 100, 1, PORT3_NUMBER}, // ** Switch 04 **
    {PAGE_VC_GM_TEST, 5, MIDI_CC, COMMON, 30, CC_ONE_SHOT, 127, 0, 1, PORT3_NUMBER}, // ** Switch 05 **
    {PAGE_VC_GM_TEST, 5 | LABEL, 'O', 'N', 'E', ' ', 'S', 'H', 'O', 'T'}, // ** Switch 05 **
    {PAGE_VC_GM_TEST, 6, MIDI_CC, COMMON, 31, CC_MOMENTARY, 127, 0, 1, PORT3_NUMBER}, // ** Switch 06 **
    {PAGE_VC_GM_TEST, 6 | LABEL, 'M', 'O', 'M', 'E', 'N', 'T', 'A', 'R'}, // ** Switch 06 **
    {PAGE_VC_GM_TEST, 6 | LABEL, 'Y', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // ** Switch 06 **
    {PAGE_VC_GM_TEST, 7, MIDI_CC, COMMON, 30, CC_TOGGLE, 127, 0, 1, PORT3_NUMBER}, // ** Switch 07 **
    {PAGE_VC_GM_TEST, 7 | LABEL, 'T', 'O', 'G', 'G', 'L', 'E', ' ', ' '}, // ** Switch 07 **
    {PAGE_VC_GM_TEST, 8, MIDI_CC, COMMON, 31, CC_TOGGLE_ON, 127, 0, 1, PORT3_NUMBER}, // ** Switch 08 **
    {PAGE_VC_GM_TEST, 8 | LABEL, 'T', 'O', 'G', 'G', 'L', 'E', ' ', 'O'}, // ** Switch 08 **
    {PAGE_VC_GM_TEST, 8 | LABEL, 'N', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // ** Switch 08 **
    {PAGE_VC_GM_TEST, 9, MIDI_CC, COMMON, 32, CC_UPDOWN, 127, 0, 1, PORT3_NUMBER}, // ** Switch 09 **
    {PAGE_VC_GM_TEST, 10, MIDI_CC, COMMON, 33, CC_STEP, 3, 0, 1, PORT3_NUMBER}, // ** Switch 10 **
    {PAGE_VC_GM_TEST, 11, SET_TEMPO, COMMON, 120}, // ** Switch 11 **
    //{PAGE_VC_GM_TEST, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
    //{PAGE_VC_GM_TEST, 13, PATCH, CURRENT, BANKDOWN, 4}, // ** Switch 13 **
    //{PAGE_VC_GM_TEST, 14, PATCH, CURRENT, BANKUP, 4}, // ** Switch 14 **
    //Switch 15 and 16 are BANK_DOWN and BANK_UP
};

// ********************************* Section 4: VC-mini configuration for fixed command pages ********************************************

const QVector<Cmd_struct> VCmini_fixed_commands = {
    // ******************************* PAGE 201: MENU *************************************************
      {PAGE_VCMINI_MENU, LABEL, 'M', 'E', 'N', 'U', ' ', ' ', ' ', ' ' },
      {PAGE_VCMINI_MENU, 1, MENU, COMMON, MENU_PREV}, // ** Switch 01 **
      {PAGE_VCMINI_MENU, 2, MENU, COMMON, MENU_NEXT}, // ** Switch 02 **
      {PAGE_VCMINI_MENU, 3, MENU, COMMON, MENU_SET_VALUE}, // ** Switch 03 *
      {PAGE_VCMINI_MENU, 4, MENU, COMMON, MENU_SELECT }, // Encoder 1
      {PAGE_VCMINI_MENU, 5, MENU, COMMON, MENU_BACK }, // Encoder 1 button
      {PAGE_VCMINI_MENU, 6, MENU, COMMON, MENU_SET_VALUE }, // Encoder 2
      {PAGE_VCMINI_MENU, 7, MENU, COMMON, MENU_SET_VALUE }, // Encoder 2 button
      {PAGE_VCMINI_MENU, 8, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 1 or expr pedal 1
      {PAGE_VCMINI_MENU, 9, NOTHING, COMMON}, // External switch 2

      // ******************************* PAGE 202: Current device Direct Select *************************************************
      {PAGE_VCMINI_CURRENT_DIRECT_SELECT, LABEL, 'D', 'I', 'R', 'E', 'C', 'T', ' ', 'S' },
      {PAGE_VCMINI_CURRENT_DIRECT_SELECT, LABEL, 'E', 'L', 'E', 'C', 'T', ' ', ' ', ' ' },
      {PAGE_VCMINI_CURRENT_DIRECT_SELECT, 1, DIRECT_SELECT, CURRENT, 1, 10}, // ** Switch 01 **
      {PAGE_VCMINI_CURRENT_DIRECT_SELECT, 2, DIRECT_SELECT, CURRENT, 2, 10}, // ** Switch 02 **
      {PAGE_VCMINI_CURRENT_DIRECT_SELECT, 3, DIRECT_SELECT, CURRENT, 3, 10}, // ** Switch 03 **

      // ******************************* PAGE 203: DEVICE MODE *************************************************
      {PAGE_VCMINI_DEVICE_MODE, LABEL, 'M', 'O', 'D', 'E', ' ', 'S', 'E', 'L' },
      {PAGE_VCMINI_DEVICE_MODE, LABEL, 'E', 'C', 'T', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VCMINI_DEVICE_MODE, 1 | ON_RELEASE, MODE, COMMON, SELECT, SONG_MODE}, // ** Switch 01 **
      {PAGE_VCMINI_DEVICE_MODE, 2 | ON_RELEASE, MODE, COMMON, SELECT, PAGE_MODE}, // ** Switch 02 **
      {PAGE_VCMINI_DEVICE_MODE, 2 | ON_LONG_PRESS, PAGE, COMMON, SELECT, PAGE_VCMINI_MENU}, // ** Switch 02 **
      {PAGE_VCMINI_DEVICE_MODE, 3 | ON_RELEASE, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 03 **
      //{PAGE_VCMINI_DEVICE_MODE, 3 | ON_RELEASE, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 03 **
      {PAGE_VCMINI_DEVICE_MODE, 3 | ON_LONG_PRESS, PAGE, COMMON, SELECT, PAGE_VCMINI_SETLIST_SELECT}, // ** Switch 03 **
      //{PAGE_VCMINI_DEVICE_MODE, 5, PAGE, COMMON, SELECT, PAGE_VCMINI_MENU}, // ** Switch 05 - ENC #1 press **
      //{PAGE_VCMINI_DEVICE_MODE, 7, PAGE, COMMON, SELECT, PAGE_VCMINI_MENU}, // ** Switch 07 - ENC #2 press **

      // ******************************* PAGE 204: PAGE MODE *************************************************
      {PAGE_VCMINI_PAGE_MODE, LABEL, 'P', 'A', 'G', 'E', ' ', 'M', 'O', 'D' },
      {PAGE_VCMINI_PAGE_MODE, LABEL, 'E', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VCMINI_PAGE_MODE, 1 | ON_RELEASE, PAGE, COMMON, BANKSELECT, 1, 3}, // ** Switch 01 **
      {PAGE_VCMINI_PAGE_MODE, 1 | ON_LONG_PRESS, PAGE, COMMON, SELECT, PAGE_VCMINI_DEVICE_MODE},
      {PAGE_VCMINI_PAGE_MODE, 2 | ON_RELEASE, PAGE, COMMON, BANKSELECT, 2, 3}, // ** Switch 02 **
      {PAGE_VCMINI_PAGE_MODE, 3 | ON_RELEASE, PAGE, COMMON, BANKSELECT, 3, 3}, // ** Switch 03 **
      {PAGE_VCMINI_PAGE_MODE, 3 | ON_LONG_PRESS, PAGE, COMMON, SELECT, PAGE_VCMINI_SETLIST_SELECT}, // ** Switch 03 **
      {PAGE_VCMINI_PAGE_MODE, 1 | ON_DUAL_PRESS, PAGE, COMMON, BANKDOWN, 3}, // ** Switch 01 + 02 **
      {PAGE_VCMINI_PAGE_MODE, 2 | ON_DUAL_PRESS, PAGE, COMMON, BANKUP, 3}, // ** Switch 02 + 03 **
      {PAGE_VCMINI_PAGE_MODE, 4, PAGE, COMMON, BANKUP, 3}, // ** Switch 07 - ENC #1 turn **
      {PAGE_VCMINI_PAGE_MODE, 6, SET_TEMPO, COMMON, 120}, // ** Switch 06 - ENC #2 turn **

      // ******************************* PAGE 205: Current_patch_bank (3 buttons per page) *************************************************
      {PAGE_VCMINI_CURRENT_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
      {PAGE_VCMINI_CURRENT_PATCH_BANK, LABEL, 'N', 'K', ' ', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VCMINI_CURRENT_PATCH_BANK, 1 | ON_RELEASE, PATCH, CURRENT, BANKSELECT, 1, 3}, // ** Switch 01 **
      {PAGE_VCMINI_CURRENT_PATCH_BANK, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
      {PAGE_VCMINI_CURRENT_PATCH_BANK, 2 | ON_RELEASE, PATCH, CURRENT, BANKSELECT, 2, 3}, // ** Switch 02 **
      {PAGE_VCMINI_CURRENT_PATCH_BANK, 2 | ON_LONG_PRESS, SAVE_PATCH, CURRENT}, // ** Switch 02 long press
      {PAGE_VCMINI_CURRENT_PATCH_BANK, 3 | ON_RELEASE, PATCH, CURRENT, BANKSELECT, 3, 3}, // ** Switch 03 **
      {PAGE_VCMINI_CURRENT_PATCH_BANK, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT},
      {PAGE_VCMINI_CURRENT_PATCH_BANK, 1 | ON_DUAL_PRESS, PATCH, CURRENT, BANKDOWN, 3}, // ** Switch 01 + 02 **
      {PAGE_VCMINI_CURRENT_PATCH_BANK, 2 | ON_DUAL_PRESS, PATCH, CURRENT, BANKUP, 3}, // ** Switch 02 + 03 **
      {PAGE_VCMINI_CURRENT_PATCH_BANK, 4, PATCH, CURRENT, BANKUP, 3}, // ** Switch 07 - ENC #1 turn **
      {PAGE_VCMINI_CURRENT_PATCH_BANK, 6, SET_TEMPO, COMMON, 120}, // ** Switch 06 - ENC #2 turn **

      // ******************************* PAGE 206: Page up/down tap *************************************************
      {PAGE_VCMINI_UP_DOWN_TAP, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', '+', '/' },
      {PAGE_VCMINI_UP_DOWN_TAP, LABEL, '-', ' ', '+', ' ', 'T', 'A', 'P', ' ' },
      {PAGE_VCMINI_UP_DOWN_TAP, 1 | ON_RELEASE, PATCH, CURRENT, PREV}, // ** Switch 01 **
      {PAGE_VCMINI_UP_DOWN_TAP, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
      {PAGE_VCMINI_UP_DOWN_TAP, 2 | ON_RELEASE, PATCH, CURRENT, NEXT}, // ** Switch 02 **
      {PAGE_VCMINI_UP_DOWN_TAP, 2 | ON_LONG_PRESS, PAGE, COMMON, SELECT, PAGE_VCMINI_SETLIST_SELECT}, // ** Switch 03 **
      {PAGE_VCMINI_UP_DOWN_TAP, 3 | ON_RELEASE, TAP_TEMPO, COMMON}, // ** Switch 03 **
      {PAGE_VCMINI_UP_DOWN_TAP, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 03 - on long press **
      {PAGE_VCMINI_UP_DOWN_TAP, 4, PATCH, CURRENT, NEXT}, // ** Switch 04 - ENC #1 turn **
      {PAGE_VCMINI_UP_DOWN_TAP, 6, SET_TEMPO, COMMON, 120}, // ** Switch 06 - ENC #2 turn **

      // ******************************* PAGE 207: Parameters current device *************************************************
      {PAGE_VCMINI_CURRENT_PARAMETER, LABEL, 'P', 'A', 'R', ' ', 'B', 'A', 'N', 'K' },
      {PAGE_VCMINI_CURRENT_PARAMETER, 1 | ON_RELEASE, PAR_BANK, CURRENT, 1, 3}, // ** Switch 01 **
      {PAGE_VCMINI_CURRENT_PARAMETER, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
      {PAGE_VCMINI_CURRENT_PARAMETER, 2 | ON_RELEASE, PAR_BANK, CURRENT, 2, 3}, // ** Switch 02 **
      {PAGE_VCMINI_CURRENT_PARAMETER, 3 | ON_RELEASE, PAR_BANK, CURRENT, 3, 3}, // ** Switch 03 **
      {PAGE_VCMINI_CURRENT_PARAMETER, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT},
      {PAGE_VCMINI_CURRENT_PARAMETER, 1 | ON_DUAL_PRESS, PAR_BANK_DOWN, CURRENT, 3}, // ** Switch 01 + 02
      {PAGE_VCMINI_CURRENT_PARAMETER, 2 | ON_DUAL_PRESS, PAR_BANK_UP, CURRENT, 3}, // ** Switch 02 + 03
      {PAGE_VCMINI_CURRENT_PARAMETER, 4, PAR_BANK_UP, CURRENT, 3}, // ** Switch 04 - ENC #1 turn **

      // ******************************* PAGE 208: Assigns current device *************************************************
      {PAGE_VCMINI_CURRENT_ASSIGN, LABEL, 'A', 'S', 'S', 'I', 'G', 'N', ' ', 'B' },
      {PAGE_VCMINI_CURRENT_ASSIGN, LABEL, 'A', 'N', 'K', ' ', 'S', 'E', 'L', ' ' },
      {PAGE_VCMINI_CURRENT_ASSIGN, 1 | ON_RELEASE, ASSIGN, CURRENT, BANKSELECT, 1, 3}, // ** Switch 01 **
      {PAGE_VCMINI_CURRENT_ASSIGN, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
      {PAGE_VCMINI_CURRENT_ASSIGN, 2 | ON_RELEASE, ASSIGN, CURRENT, BANKSELECT, 2, 3}, // ** Switch 02 **
      {PAGE_VCMINI_CURRENT_ASSIGN, 3 | ON_RELEASE, ASSIGN, CURRENT, BANKSELECT, 3, 3}, // ** Switch 03 **
      {PAGE_VCMINI_CURRENT_ASSIGN, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT},
      {PAGE_VCMINI_CURRENT_ASSIGN, 1 | ON_DUAL_PRESS, ASSIGN, CURRENT, BANKDOWN, 3}, // ** Switch 01 + 02
      {PAGE_VCMINI_CURRENT_ASSIGN, 2 | ON_DUAL_PRESS, ASSIGN, CURRENT, BANKUP, 3}, // ** Switch 02 + 03
      {PAGE_VCMINI_CURRENT_ASSIGN, 4, ASSIGN, CURRENT, BANKUP, 3}, // ** Switch 04 - ENC #1 turn **

      // ******************************* PAGE 209: Snapshots (Helix) /scenes (AxeFX) *************************************************
      {PAGE_VCMINI_SNAPSCENE, LABEL, 'S', 'N', 'A', 'P', 'S', 'C', 'E', 'N' },
      {PAGE_VCMINI_SNAPSCENE, LABEL, 'E', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VCMINI_SNAPSCENE, 1 | ON_RELEASE, SNAPSCENE, CURRENT, 1, 4, 7}, // ** Switch 01 **
      {PAGE_VCMINI_SNAPSCENE, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
      {PAGE_VCMINI_SNAPSCENE, 2 | ON_RELEASE, SNAPSCENE, CURRENT, 2, 5, 8}, // ** Switch 02 **
      {PAGE_VCMINI_SNAPSCENE, 2 | ON_LONG_PRESS, SAVE_PATCH, CURRENT}, // ** Switch 02 **
      {PAGE_VCMINI_SNAPSCENE, 3 | ON_RELEASE, SNAPSCENE, CURRENT, 3, 6, 0}, // ** Switch 03 **
      {PAGE_VCMINI_SNAPSCENE, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 03 - on long press **

      // ******************************* PAGE 210: Looper control *************************************************
      {PAGE_VCMINI_LOOPER, LABEL, 'L', 'O', 'O', 'P', 'E', 'R', ' ', 'C' },
      {PAGE_VCMINI_LOOPER, LABEL, 'O', 'N', 'T', 'R', 'O', 'L', ' ', ' ' },
      {PAGE_VCMINI_LOOPER, 1, LOOPER, CURRENT, LOOPER_REC_OVERDUB}, // ** Switch 01 **
      {PAGE_VCMINI_LOOPER, 2, LOOPER, CURRENT, LOOPER_PLAY_STOP}, // ** Switch 02 **
      {PAGE_VCMINI_LOOPER, 3 | ON_RELEASE, LOOPER, CURRENT, LOOPER_UNDO_REDO}, // ** Switch 03 **
      {PAGE_VCMINI_LOOPER, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 03 - on long press **

      // ******************************* PAGE 211: KPA Looper control *************************************************
      {PAGE_VCMINI_KPA_LOOPER, LABEL, 'K', 'P', 'A', ' ', 'L', 'O', 'O', 'P' },
      {PAGE_VCMINI_KPA_LOOPER, LABEL, 'E', 'R', ' ', 'C', 'T', 'L', ' ', ' ' },
      {PAGE_VCMINI_KPA_LOOPER, 1, LOOPER, CURRENT, LOOPER_REC_PLAY_OVERDUB}, // ** Switch 01 **
      {PAGE_VCMINI_KPA_LOOPER, 2, LOOPER, CURRENT, LOOPER_STOP_ERASE}, // ** Switch 02 **
      {PAGE_VCMINI_KPA_LOOPER, 3 | ON_RELEASE, LOOPER, CURRENT, LOOPER_UNDO_REDO}, // ** Switch 03 **
      {PAGE_VCMINI_KPA_LOOPER, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 03 - on long press **


      // ******************************* PAGE 212: KATANA FX CTRL #1  *************************************************
      {PAGE_VCMINI_KTN_FX1, LABEL, 'K', 'A', 'T', 'A', 'N', 'A', ' ', 'F'},
      {PAGE_VCMINI_KTN_FX1, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '1'},
      {PAGE_VCMINI_KTN_FX1, 1 | ON_RELEASE, PARAMETER, KTN, 8, TOGGLE, 1, 0}, // ** Switch 01 **
      {PAGE_VCMINI_KTN_FX1, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
      {PAGE_VCMINI_KTN_FX1, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_VCMINI_KTN_FX3}, // ** Switch 01 + 02 **
      {PAGE_VCMINI_KTN_FX1, 2 | ON_RELEASE, PARAMETER, KTN, 23, TOGGLE, 1, 0}, // ** Switch 02 **
      {PAGE_VCMINI_KTN_FX1, 2 | ON_LONG_PRESS, SAVE_PATCH, KTN}, // ** Switch 02 long press
      {PAGE_VCMINI_KTN_FX1, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_VCMINI_KTN_FX2}, // ** Switch 02 +| 03 **
      {PAGE_VCMINI_KTN_FX1, 3 | ON_RELEASE, PARAMETER, KTN, 70, TOGGLE, 1, 0}, // ** Switch 03 **
      {PAGE_VCMINI_KTN_FX1, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_VCMINI_PAGE1}, // ** Switch 03 - on long press **
      {PAGE_VCMINI_KTN_FX1, 4, PAR_BANK_UP, KTN, 1}, // ** Switch 04 - ENC #1 turn **
      {PAGE_VCMINI_KTN_FX1, 6, PAR_BANK, KTN, 1, 1}, // ** Switch 06 - ENC #2 turn **
      {PAGE_VCMINI_KTN_FX1, 7, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_VCMINI_PAGE1}, // ** Switch 07 - ENC #2 press **

      // ******************************* PAGE 213: KATANA FX CTRL #2  *************************************************
      {PAGE_VCMINI_KTN_FX2, LABEL, 'K', 'A', 'T', 'A', 'N', 'A', ' ', 'F'},
      {PAGE_VCMINI_KTN_FX2, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '2'},
      {PAGE_VCMINI_KTN_FX2, 1 | ON_RELEASE, PARAMETER, KTN, 61, TOGGLE, 1, 0}, // ** Switch 01 **
      {PAGE_VCMINI_KTN_FX2, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
      {PAGE_VCMINI_KTN_FX2, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_VCMINI_KTN_FX1}, // ** Switch 01 + 02 **
      {PAGE_VCMINI_KTN_FX2, 2 | ON_RELEASE, PARAMETER, KTN, 87, TOGGLE, 1, 0}, // ** Switch 02 **
      {PAGE_VCMINI_KTN_FX2, 2 | ON_LONG_PRESS, SAVE_PATCH, KTN}, // ** Switch 02 long press
      {PAGE_VCMINI_KTN_FX2, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_VCMINI_KTN_FX3}, // ** Switch 02 +| 03 **
      {PAGE_VCMINI_KTN_FX2, 3, TAP_TEMPO, COMMON}, // ** Switch 03 **
      {PAGE_VCMINI_KTN_FX2, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_VCMINI_PAGE1}, // ** Switch 03 - on long press **
      {PAGE_VCMINI_KTN_FX2, 4, PAR_BANK_UP, KTN, 1}, // ** Switch 04 - ENC #1 turn **
      {PAGE_VCMINI_KTN_FX2, 6, PAR_BANK, KTN, 1, 1}, // ** Switch 06 - ENC #2 turn **
      {PAGE_VCMINI_KTN_FX2, 7, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_VCMINI_PAGE1}, // ** Switch 07 - ENC #2 press **

      // ******************************* PAGE 214: KATANA FX CTRL #3  *************************************************
      {PAGE_VCMINI_KTN_FX3, LABEL, 'K', 'A', 'T', 'A', 'N', 'A', ' ', 'F'},
      {PAGE_VCMINI_KTN_FX3, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '3'},
      {PAGE_VCMINI_KTN_FX3, 1 | ON_RELEASE, PARAMETER, KTN, 96, TOGGLE, 1, 0}, // ** Switch 01 **
      {PAGE_VCMINI_KTN_FX3, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
      {PAGE_VCMINI_KTN_FX3, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_VCMINI_KTN_FX2}, // ** Switch 01 + 02 **
      {PAGE_VCMINI_KTN_FX3, 2 | ON_RELEASE, PARAMETER, KTN, 48, TOGGLE, 1, 0}, // ** Switch 02 **
      {PAGE_VCMINI_KTN_FX3, 2 | ON_LONG_PRESS, SAVE_PATCH, KTN}, // ** Switch 02 long press
      {PAGE_VCMINI_KTN_FX3, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_VCMINI_KTN_FX1}, // ** Switch 02 +| 03 **
      {PAGE_VCMINI_KTN_FX3, 3 | ON_RELEASE, PARAMETER, KTN, 106, TOGGLE, 1, 0}, // ** Switch 03 **
      {PAGE_VCMINI_KTN_FX3, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_VCMINI_PAGE1}, // ** Switch 03 - on long press **
      {PAGE_VCMINI_KTN_FX3, 4, PAR_BANK_UP, KTN, 1}, // ** Switch 04 - ENC #1 turn **
      {PAGE_VCMINI_KTN_FX3, 6, PAR_BANK, KTN, 1, 1}, // ** Switch 06 - ENC #2 turn **
      {PAGE_VCMINI_KTN_FX3, 7, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_VCMINI_PAGE1}, // ** Switch 07 - ENC #2 press **

      // ******************************* PAGE 215: KATANA4 FX CTRL #1  *************************************************
      {PAGE_VCMINI_KTN4_FX1, LABEL, 'K', 'T', 'N', '_', 'V', '4', ' ', 'F'},
      {PAGE_VCMINI_KTN4_FX1, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '1'},
      {PAGE_VCMINI_KTN4_FX1, 1 | ON_RELEASE, PARAMETER, KTN, 0, TOGGLE, 1, 0}, // ** Switch 01 **
      {PAGE_VCMINI_KTN4_FX1, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
      {PAGE_VCMINI_KTN4_FX1, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_VCMINI_KTN4_FX3}, // ** Switch 01 + 02 **
      {PAGE_VCMINI_KTN4_FX1, 2 | ON_RELEASE, PARAMETER, KTN, 8, TOGGLE, 1, 0}, // ** Switch 02 **
      {PAGE_VCMINI_KTN4_FX1, 2 | ON_LONG_PRESS, SAVE_PATCH, KTN}, // ** Switch 02 long press
      {PAGE_VCMINI_KTN4_FX1, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_VCMINI_KTN4_FX2}, // ** Switch 02 +| 03 **
      {PAGE_VCMINI_KTN4_FX1, 3 | ON_RELEASE, PARAMETER, KTN, 23, TOGGLE, 1, 0}, // ** Switch 03 **
      {PAGE_VCMINI_KTN4_FX1, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_VCMINI_PAGE1}, // ** Switch 03 - on long press **
      {PAGE_VCMINI_KTN4_FX1, 4, PAR_BANK_UP, KTN, 1}, // ** Switch 04 - ENC #1 turn **
      {PAGE_VCMINI_KTN4_FX1, 6, PAR_BANK, KTN, 1, 1}, // ** Switch 06 - ENC #2 turn **
      {PAGE_VCMINI_KTN4_FX1, 7, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_VCMINI_PAGE1}, // ** Switch 07 - ENC #2 press **

      // ******************************* PAGE 216: KATANA4 FX CTRL #2  *************************************************
      {PAGE_VCMINI_KTN4_FX2, LABEL, 'K', 'T', 'N', '_', 'V', '4', ' ', 'F'},
      {PAGE_VCMINI_KTN4_FX2, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '2'},
      {PAGE_VCMINI_KTN4_FX2, 1 | ON_RELEASE, PARAMETER, KTN, 61, TOGGLE, 1, 0}, // ** Switch 01 **
      {PAGE_VCMINI_KTN4_FX2, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
      {PAGE_VCMINI_KTN4_FX2, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_VCMINI_KTN4_FX1}, // ** Switch 01 + 02 **
      {PAGE_VCMINI_KTN4_FX2, 2 | ON_RELEASE, PARAMETER, KTN, 87, TOGGLE, 1, 0}, // ** Switch 02 **
      {PAGE_VCMINI_KTN4_FX2, 2 | ON_LONG_PRESS, SAVE_PATCH, KTN}, // ** Switch 02 long press
      {PAGE_VCMINI_KTN4_FX2, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_VCMINI_KTN4_FX3}, // ** Switch 02 +| 03 **
      {PAGE_VCMINI_KTN4_FX2, 3, TAP_TEMPO, COMMON}, // ** Switch 03 **
      {PAGE_VCMINI_KTN4_FX2, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_VCMINI_PAGE1}, // ** Switch 03 - on long press **
      {PAGE_VCMINI_KTN4_FX2, 4, PAR_BANK_UP, KTN, 1}, // ** Switch 04 - ENC #1 turn **
      {PAGE_VCMINI_KTN4_FX2, 6, PAR_BANK, KTN, 1, 1}, // ** Switch 06 - ENC #2 turn **
      {PAGE_VCMINI_KTN4_FX2, 7, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_VCMINI_PAGE1}, // ** Switch 07 - ENC #2 press **

      // ******************************* PAGE 217: KATANA4 FX CTRL #3  *************************************************
      {PAGE_VCMINI_KTN4_FX3, LABEL, 'K', 'T', 'N', '_', 'V', '4', ' ', 'F'},
      {PAGE_VCMINI_KTN4_FX3, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '3'},
      {PAGE_VCMINI_KTN4_FX3, 1 | ON_RELEASE, PARAMETER, KTN, 70, TOGGLE, 1, 0}, // ** Switch 01 **
      {PAGE_VCMINI_KTN4_FX3, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
      {PAGE_VCMINI_KTN4_FX3, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_VCMINI_KTN4_FX2}, // ** Switch 01 + 02 **
      {PAGE_VCMINI_KTN4_FX3, 2 | ON_RELEASE, PARAMETER, KTN, 96, TOGGLE, 1, 0}, // ** Switch 02 **
      {PAGE_VCMINI_KTN4_FX3, 2 | ON_LONG_PRESS, SAVE_PATCH, KTN}, // ** Switch 02 long press
      {PAGE_VCMINI_KTN4_FX3, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_VCMINI_KTN4_FX1}, // ** Switch 02 +| 03 **
      {PAGE_VCMINI_KTN4_FX3, 3 | ON_RELEASE, PARAMETER, KTN, 106, TOGGLE, 1, 0}, // ** Switch 03 **
      {PAGE_VCMINI_KTN4_FX3, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_VCMINI_PAGE1}, // ** Switch 03 - on long press **
      {PAGE_VCMINI_KTN4_FX3, 4, PAR_BANK_UP, KTN, 1}, // ** Switch 04 - ENC #1 turn **
      {PAGE_VCMINI_KTN4_FX3, 6, PAR_BANK, KTN, 1, 1}, // ** Switch 06 - ENC #2 turn **
      {PAGE_VCMINI_KTN4_FX3, 7, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_VCMINI_PAGE1}, // ** Switch 07 - ENC #2 press **

      // ******************************* PAGE 218: MG-300 FX CTRL #1  *************************************************
      {PAGE_VCMINI_MG300_FX1, LABEL, 'M', 'G', '-', '3', '0', '0', ' ', 'F'},
      {PAGE_VCMINI_MG300_FX1, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '1'},
      {PAGE_VCMINI_MG300_FX1, 1 | ON_RELEASE, PARAMETER, MG300, 1, TOGGLE, 1, 0}, // ** Switch 01 **
      {PAGE_VCMINI_MG300_FX1, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
      {PAGE_VCMINI_MG300_FX1, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, MG300, PAGE_VCMINI_MG300_FX3}, // ** Switch 01 + 02 **
      {PAGE_VCMINI_MG300_FX1, 2 | ON_RELEASE, PARAMETER, MG300, 2, TOGGLE, 1, 0}, // ** Switch 02 **
      {PAGE_VCMINI_MG300_FX1, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, MG300, PAGE_VCMINI_MG300_FX2}, // ** Switch 02 +| 03 **
      {PAGE_VCMINI_MG300_FX1, 3 | ON_RELEASE, PARAMETER, MG300, 6, TOGGLE, 1, 0}, // ** Switch 03 **
      {PAGE_VCMINI_MG300_FX1, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, MG300, MG300_DEFAULT_VCMINI_PAGE1}, // ** Switch 03 - on long press **
      {PAGE_VCMINI_MG300_FX1, 4, PATCH, MG300, NEXT}, // ** Switch 04 - ENC #1 turn **
      {PAGE_VCMINI_MG300_FX1, 6, SET_TEMPO, COMMON, 120}, // ** Switch 06 - ENC #2 turn **
      {PAGE_VCMINI_MG300_FX1, 7, OPEN_PAGE_DEVICE, MG300, MG300_DEFAULT_VCMINI_PAGE1}, // ** Switch 07 - ENC #2 press **

      // ******************************* PAGE 219: MG-300 FX CTRL #2  *************************************************
      {PAGE_VCMINI_MG300_FX2, LABEL, 'M', 'G', '-', '3', '0', '0', ' ', 'F'},
      {PAGE_VCMINI_MG300_FX2, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '2'},
      {PAGE_VCMINI_MG300_FX2, 1 | ON_RELEASE, PARAMETER, MG300, 7, TOGGLE, 1, 0}, // ** Switch 01 **
      {PAGE_VCMINI_MG300_FX2, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
      {PAGE_VCMINI_MG300_FX2, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, MG300, PAGE_VCMINI_MG300_FX1}, // ** Switch 01 + 02 **
      {PAGE_VCMINI_MG300_FX2, 2 | ON_RELEASE, PARAMETER, MG300, 8, TOGGLE, 1, 0}, // ** Switch 02 **
      {PAGE_VCMINI_MG300_FX2, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, MG300, PAGE_VCMINI_MG300_FX3}, // ** Switch 02 +| 03 **
      {PAGE_VCMINI_MG300_FX2, 3, TAP_TEMPO, COMMON}, // ** Switch 03 **
      {PAGE_VCMINI_MG300_FX2, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, MG300, MG300_DEFAULT_VCMINI_PAGE1}, // ** Switch 03 - on long press **
      {PAGE_VCMINI_MG300_FX2, 4, PATCH, MG300, NEXT}, // ** Switch 04 - ENC #1 turn **
      {PAGE_VCMINI_MG300_FX2, 6, SET_TEMPO, COMMON, 120}, // ** Switch 06 - ENC #2 turn **
      {PAGE_VCMINI_MG300_FX2, 7, OPEN_PAGE_DEVICE, MG300, MG300_DEFAULT_VCMINI_PAGE1}, // ** Switch 07 - ENC #2 press **

      // ******************************* PAGE 220: MG-300 FX CTRL #3  *************************************************
      {PAGE_VCMINI_MG300_FX3, LABEL, 'M', 'G', '-', '3', '0', '0', ' ', 'F'},
      {PAGE_VCMINI_MG300_FX3, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '3'},
      {PAGE_VCMINI_MG300_FX3, 1 | ON_RELEASE, PARAMETER, MG300, 3, TOGGLE, 1, 0}, // ** Switch 01 **
      {PAGE_VCMINI_MG300_FX3, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
      {PAGE_VCMINI_MG300_FX3, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, MG300, PAGE_VCMINI_MG300_FX2}, // ** Switch 01 + 02 **
      {PAGE_VCMINI_MG300_FX3, 2 | ON_RELEASE, PARAMETER, MG300, 4, TOGGLE, 1, 0}, // ** Switch 02 **
      {PAGE_VCMINI_MG300_FX3, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, MG300, PAGE_VCMINI_MG300_FX1}, // ** Switch 02 +| 03 **
      {PAGE_VCMINI_MG300_FX3, 3 | ON_RELEASE, PARAMETER, MG300, 5, TOGGLE, 1, 0}, // ** Switch 03 **
      {PAGE_VCMINI_MG300_FX3, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, MG300, MG300_DEFAULT_VCMINI_PAGE1}, // ** Switch 03 - on long press **
      {PAGE_VCMINI_MG300_FX3, 4, PATCH, MG300, NEXT}, // ** Switch 04 - ENC #1 turn **
      {PAGE_VCMINI_MG300_FX3, 6, SET_TEMPO, COMMON, 120}, // ** Switch 06 - ENC #2 turn **
      {PAGE_VCMINI_MG300_FX3, 7, OPEN_PAGE_DEVICE, MG300, MG300_DEFAULT_VCMINI_PAGE1}, // ** Switch 07 - ENC #2 press **

      // ******************************* PAGE 221: SY1000 MODE SELECT  *************************************************
      {PAGE_VCMINI_SY1000_MODE_SEL1, LABEL, 'M', 'O', 'D', 'E', ' ', 'S', 'E', 'L'},
      {PAGE_VCMINI_SY1000_MODE_SEL1, LABEL, '1', ' ', 'S', 'Y', '1', '0', '0', '0'},
      {PAGE_VCMINI_SY1000_MODE_SEL1, 1 | LABEL, 'P', 'A', 'T', 'C', 'H', ' ', ' ', ' '},
      {PAGE_VCMINI_SY1000_MODE_SEL1, 1 | ON_RELEASE, PARAMETER, SY1000, 0, TOGGLE, 2, 2}, // ** Switch 01 **
      {PAGE_VCMINI_SY1000_MODE_SEL1, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
      {PAGE_VCMINI_SY1000_MODE_SEL1, 2 | LABEL, 'S', 'C', 'E', 'N', 'E', ' ', ' ', ' '},
      {PAGE_VCMINI_SY1000_MODE_SEL1, 2 | ON_RELEASE, PARAMETER, SY1000, 0, TOGGLE, 3, 4}, // ** Switch 02 **
      {PAGE_VCMINI_SY1000_MODE_SEL1, 2 | ON_LONG_PRESS, SAVE_PATCH, SY1000},
      {PAGE_VCMINI_SY1000_MODE_SEL1, 3 | LABEL, 'M', 'A', 'N', 'U', 'A', 'L', ' ', ' '},
      {PAGE_VCMINI_SY1000_MODE_SEL1, 3 | ON_RELEASE, PARAMETER, SY1000, 0, TOGGLE, 1, 0}, // ** Switch 03 **
      {PAGE_VCMINI_SY1000_MODE_SEL1, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT},
      {PAGE_VCMINI_SY1000_MODE_SEL1, 4, PATCH, SY1000, NEXT}, // ** Switch 04 - ENC #1 turn **
      {PAGE_VCMINI_SY1000_MODE_SEL1, 6, SET_TEMPO, COMMON, 120}, // ** Switch 06 - ENC #2 turn **
      //{PAGE_VCMINI_SY1000_MODE_SEL1, 7, TAP_TEMPO, COMMON}, // ** Switch 07 - ENC #2 press **

      // ******************************* PAGE 222: SY1000 MODE SELECT (TOP ROW)  *************************************************
      {PAGE_VCMINI_SY1000_MODE_SEL2, LABEL, 'M', 'O', 'D', 'E', ' ', 'S', 'E', 'L'},
      {PAGE_VCMINI_SY1000_MODE_SEL2, LABEL, '2', ' ', 'S', 'Y', '1', '0', '0', '0'},
      {PAGE_VCMINI_SY1000_MODE_SEL2, 1 | LABEL, 'P', 'A', 'T', 'C', 'H', ' ', ' ', ' '},
      {PAGE_VCMINI_SY1000_MODE_SEL2, 1 | ON_RELEASE, PARAMETER, SY1000, 0, TOGGLE, 2, 2}, // ** Switch 01 **
      {PAGE_VCMINI_SY1000_MODE_SEL2, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
      {PAGE_VCMINI_SY1000_MODE_SEL2, 2 | LABEL, 'S', 'C', 'E', 'N', 'E', ' ', ' ', ' '},
      {PAGE_VCMINI_SY1000_MODE_SEL2, 2 | ON_RELEASE, PARAMETER, SY1000, 0, TOGGLE, 6, 4}, // ** Switch 02 **
      {PAGE_VCMINI_SY1000_MODE_SEL2, 2 | ON_LONG_PRESS, SAVE_PATCH, SY1000},
      {PAGE_VCMINI_SY1000_MODE_SEL2, 3 | LABEL, 'M', 'A', 'N', 'U', 'A', 'L', ' ', ' '},
      {PAGE_VCMINI_SY1000_MODE_SEL2, 3 | ON_RELEASE, PARAMETER, SY1000, 0, TOGGLE, 5, 0}, // ** Switch 03 **
      {PAGE_VCMINI_SY1000_MODE_SEL2, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT},
      {PAGE_VCMINI_SY1000_MODE_SEL2, 4, PATCH, SY1000, NEXT}, // ** Switch 04 - ENC #1 turn **
      {PAGE_VCMINI_SY1000_MODE_SEL2, 6, SET_TEMPO, COMMON, 120}, // ** Switch 06 - ENC #2 turn **
      //{PAGE_VCMINI_SY1000_MODE_SEL2, 7, TAP_TEMPO, COMMON}, // ** Switch 07 - ENC #2 press **

      // ******************************* PAGE 223: KPA RIG SELECT  *************************************************
      {PAGE_VCMINI_KPA_RIG_SELECT, LABEL, 'K', 'P', 'A', ' ', 'R', 'I', 'G', ' '},
      {PAGE_VCMINI_KPA_RIG_SELECT, LABEL, 'S', 'E', 'L', 'E', 'C', 'T', ' ', ' '},
      {PAGE_VCMINI_KPA_RIG_SELECT, 1 | ON_RELEASE, PARAMETER, KPA, 18, ONE_SHOT, 0, 0}, // ** Switch 01 **
      {PAGE_VCMINI_KPA_RIG_SELECT, 1 | ON_DUAL_PRESS, PATCH, KPA, PREV}, // ** Switch 01 **
      {PAGE_VCMINI_KPA_RIG_SELECT, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
      {PAGE_VCMINI_KPA_RIG_SELECT, 2 | ON_RELEASE, PARAMETER, KPA, 17, ONE_SHOT, 0, 0}, // ** Switch 02 **
      {PAGE_VCMINI_KPA_RIG_SELECT, 2 | ON_DUAL_PRESS, PATCH, KPA, NEXT}, // ** Switch 02 **
      {PAGE_VCMINI_KPA_RIG_SELECT, 3, TAP_TEMPO, COMMON}, // ** Switch 03 **
      {PAGE_VCMINI_KPA_RIG_SELECT, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT},
      {PAGE_VCMINI_KPA_RIG_SELECT, 4, PATCH, KPA, NEXT}, // ** Switch 04 - ENC #1 turn **
      {PAGE_VCMINI_KPA_RIG_SELECT, 6, SET_TEMPO, COMMON, 120}, // ** Switch 06 - ENC #2 turn **
      //{PAGE_VCMINI_KPA_RIG_SELECT, 7, TAP_TEMPO, COMMON}, // ** Switch 07 - ENC #2 press **

    // ******************************* PAGE 224: SONG SELECT *************************************************
      {PAGE_VCMINI_SONG_MODE, LABEL, 'S', 'O', 'N', 'G', ' ', 'M', 'O', 'D' },
      {PAGE_VCMINI_SONG_MODE, LABEL, 'E', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VCMINI_SONG_MODE, 1 | ON_RELEASE, SONG, COMMON, SONG_PREV, SONG_PREVNEXT_SONGPART}, // ** Switch 01 **
      {PAGE_VCMINI_SONG_MODE, 1 | ON_LONG_PRESS, PAGE, COMMON, SELECT, PAGE_VCMINI_DEVICE_MODE},
      {PAGE_VCMINI_SONG_MODE, 2 | ON_RELEASE, SONG, COMMON, SONG_NEXT, SONG_PREVNEXT_SONGPART}, // ** Switch 02 **
      {PAGE_VCMINI_SONG_MODE, 2 | ON_LONG_PRESS, SONG, COMMON, SONG_EDIT}, // ** Switch 02 **
      {PAGE_VCMINI_SONG_MODE, 3 | ON_RELEASE, TAP_TEMPO, COMMON}, // ** Switch 03 **
      {PAGE_VCMINI_SONG_MODE, 3 | ON_LONG_PRESS, PAGE, COMMON, SELECT, PAGE_VCMINI_SETLIST_SELECT}, // ** Switch 03 **
      {PAGE_VCMINI_SONG_MODE, 1 | ON_DUAL_PRESS, SONG, COMMON, SONG_PREV, SONG_PREVNEXT_SONG}, // ** Switch 01 + 02 **
      {PAGE_VCMINI_SONG_MODE, 2 | ON_DUAL_PRESS, SONG, COMMON, SONG_NEXT, SONG_PREVNEXT_SONG}, // ** Switch 02 + 03 **
      {PAGE_VCMINI_SONG_MODE, 4, SONG, COMMON, SONG_NEXT, SONG_PREVNEXT_SONG}, // ** Switch 07 - ENC #1 turn **
      {PAGE_VCMINI_SONG_MODE, 6, SET_TEMPO, COMMON, 120}, // ** Switch 06 - ENC #2 turn **

      // ******************************* PAGE 225: SETLIST SELECT *************************************************
      {PAGE_VCMINI_SETLIST_SELECT, LABEL, 'S', 'E', 'T', 'L', 'I', 'S', 'T', ' ' },
      {PAGE_VCMINI_SETLIST_SELECT, LABEL, 'S', 'E', 'L', 'E', 'C', 'T', ' ', ' ' },
      {PAGE_VCMINI_SETLIST_SELECT, 1 | ON_RELEASE, SETLIST, COMMON, SL_PREV}, // ** Switch 01 **
      {PAGE_VCMINI_SETLIST_SELECT, 1 | ON_LONG_PRESS, PAGE, COMMON, SELECT, PAGE_VCMINI_DEVICE_MODE},
      {PAGE_VCMINI_SETLIST_SELECT, 2 | ON_RELEASE, SETLIST, COMMON, SL_NEXT}, // ** Switch 02 **
      {PAGE_VCMINI_SETLIST_SELECT, 2 | ON_LONG_PRESS, SETLIST, COMMON, SL_EDIT}, // ** Switch 02 **
      {PAGE_VCMINI_SETLIST_SELECT, 3 | ON_RELEASE, PAGE, COMMON, SELECT, 0}, // ** Switch 15 **
      {PAGE_VCMINI_SETLIST_SELECT, 3 | LABEL, 'E', 'X', 'I', 'T', ' ', ' ', ' ', ' ' },
      {PAGE_VCMINI_SETLIST_SELECT, 4, SETLIST, COMMON, SL_NEXT}, // ** Switch 07 - ENC #1 turn **
};

const uint16_t number_of_fixed_VCmini_commands = VCmini_fixed_commands.size();

// ********************************* Section 5: VC-mini default configuration for programmable pages ********************************************

const QVector<Cmd_struct> VCmini_commands = {
    // ******************************* PAGE 00: Default page *************************************************
    // Page, Switch, Type, Device, Data1, Data2, Value1, Value2, Value3, Value4, Value5
    //{PAGE_VCMINI_DEFAULT, LABEL, 'D', 'E', 'F', 'A', 'U', 'L', 'T', ' ' },
    {PAGE_VCMINI_DEFAULT, 1, NOTHING, COMMON}, // ** Switch 01 **
    {PAGE_VCMINI_DEFAULT, 2, NOTHING, COMMON}, // ** Switch 02 **
    {PAGE_VCMINI_DEFAULT, 3, NOTHING, COMMON}, // ** Switch 03 **
    {PAGE_VCMINI_DEFAULT, 4, PAGE, COMMON, NEXT }, // ** ENC01 turn **
    {PAGE_VCMINI_DEFAULT, 5 | ON_RELEASE, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 05 - ENC #1 press **
    {PAGE_VCMINI_DEFAULT, 6, NOTHING, COMMON}, // ** ENC02 turn **
    {PAGE_VCMINI_DEFAULT, 7 | ON_RELEASE, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 07 - ENC #2 press **
    {PAGE_VCMINI_DEFAULT, 8, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 1 or expr pedal 1
    {PAGE_VCMINI_DEFAULT, 9, NOTHING, COMMON}, // External switch 2
    {PAGE_VCMINI_DEFAULT, 10, NOTHING, COMMON}, // MIDI switch 01
    {PAGE_VCMINI_DEFAULT, 11, NOTHING, COMMON}, // MIDI switch 02
    {PAGE_VCMINI_DEFAULT, 12, NOTHING, COMMON}, // MIDI switch 03
    {PAGE_VCMINI_DEFAULT, 13, NOTHING, COMMON}, // MIDI switch 04
    {PAGE_VCMINI_DEFAULT, 14, NOTHING, COMMON}, // MIDI switch 05
    {PAGE_VCMINI_DEFAULT, 15, NOTHING, COMMON}, // MIDI switch 06
    {PAGE_VCMINI_DEFAULT, 16, NOTHING, COMMON}, // MIDI switch 07
    {PAGE_VCMINI_DEFAULT, 17, NOTHING, COMMON}, // MIDI switch 08
    {PAGE_VCMINI_DEFAULT, 18, NOTHING, COMMON}, // MIDI switch 09
    {PAGE_VCMINI_DEFAULT, 19, NOTHING, COMMON}, // MIDI switch 10
    {PAGE_VCMINI_DEFAULT, 20, NOTHING, COMMON}, // MIDI switch 11
    {PAGE_VCMINI_DEFAULT, 21, NOTHING, COMMON}, // MIDI switch 12
    {PAGE_VCMINI_DEFAULT, 22, NOTHING, COMMON}, // MIDI switch 13
    {PAGE_VCMINI_DEFAULT, 23, NOTHING, COMMON}, // MIDI switch 14
    {PAGE_VCMINI_DEFAULT, 24, NOTHING, COMMON}, // MIDI switch 15

    // ******************************* PAGE 01: GM TEST *************************************************
    {PAGE_VCMINI_GM_TEST, LABEL, 'G', 'E', 'N', '.', 'M', 'I', 'D', 'I' },
    {PAGE_VCMINI_GM_TEST, LABEL, ' ', 'T', 'E', 'S', 'T', ' ', ' ', ' ' },
    {PAGE_VCMINI_GM_TEST, 1, MIDI_PC, COMMON, 1, 1, PORT3_NUMBER}, // ** Switch 01 **
    {PAGE_VCMINI_GM_TEST, 1 | ON_LONG_PRESS, PAGE, COMMON, PREV },
    {PAGE_VCMINI_GM_TEST, 2, MIDI_PC, COMMON, 2, 1, PORT3_NUMBER}, // ** Switch 02 **
    {PAGE_VCMINI_GM_TEST, 3, MIDI_NOTE, COMMON, 52, 100, 1, PORT3_NUMBER}, // ** Switch 03 **
    {PAGE_VCMINI_GM_TEST, 3 | ON_LONG_PRESS, PAGE, COMMON, NEXT }, // Switch 03

    {PAGE_VCMINI_COMBO1, LABEL, 'G', 'P', 'V', 'G', 'G', 'R', ' ', ' ' },
    {PAGE_VCMINI_COMBO1, 1 | ON_RELEASE, PATCH, GP10, BANKSELECT, 1, 1}, // ** Switch 01 **
    {PAGE_VCMINI_COMBO1, 1 | ON_LONG_PRESS, PAGE, COMMON, PREV},
    {PAGE_VCMINI_COMBO1, 2 | ON_RELEASE, PATCH, GR55, BANKSELECT, 1, 1}, // ** Switch 02 **
    {PAGE_VCMINI_COMBO1, 3 | ON_RELEASE, PATCH, VG99, BANKSELECT, 1, 1}, // ** Switch 03 **
    {PAGE_VCMINI_COMBO1, 3 | ON_LONG_PRESS, PAGE, COMMON, NEXT},
    {PAGE_VCMINI_COMBO1, 1 | ON_DUAL_PRESS, PATCH, CURRENT, BANKDOWN, 1}, // ** Switch 1 + 2 **
    {PAGE_VCMINI_COMBO1, 2 | ON_DUAL_PRESS, PATCH, CURRENT, BANKUP, 1}, // ** Switch 1 + 2 **

    {PAGE_VCMINI_FUNC, 1 | ON_RELEASE, TOGGLE_EXP_PEDAL, CURRENT}, // ** Switch 1 **
    {PAGE_VCMINI_FUNC, 1 | ON_LONG_PRESS, PAGE, COMMON, PREV},
    {PAGE_VCMINI_FUNC, 2 | ON_RELEASE, TAP_TEMPO, COMMON}, // ** Switch 2 **
    {PAGE_VCMINI_FUNC, 2 | ON_LONG_PRESS, GLOBAL_TUNER, COMMON}, // ** Switch 2 **
    {PAGE_VCMINI_FUNC, 3 | ON_RELEASE, SET_TEMPO, COMMON, 120 },
    {PAGE_VCMINI_FUNC, 3 | ON_LONG_PRESS, PAGE, COMMON, NEXT},
};


// ********************************* Section 6: VC-touch configuration for fixed command pages ********************************************

const QVector<Cmd_struct> VCtouch_fixed_commands = {
    // ******************************* PAGE 201: MENU *************************************************
      {PAGE_VCTOUCH_MENU, LABEL, 'M', 'E', 'N', 'U', ' ', ' ', ' ', ' ' },
      {PAGE_VCTOUCH_MENU, 1, MENU, COMMON, 6}, // ** Switch 01 **
      {PAGE_VCTOUCH_MENU, 2, MENU, COMMON, 7}, // ** Switch 02 **
      {PAGE_VCTOUCH_MENU, 3, MENU, COMMON, 8}, // ** Switch 03 **
      {PAGE_VCTOUCH_MENU, 4, MENU, COMMON, 9}, // ** Switch 04 **
      {PAGE_VCTOUCH_MENU, 5, MENU, COMMON, 10}, // ** Switch 05 **
      {PAGE_VCTOUCH_MENU, 6, MENU, COMMON, 1}, // ** Switch 06 **
      {PAGE_VCTOUCH_MENU, 7, MENU, COMMON, 2}, // ** Switch 07 **
      {PAGE_VCTOUCH_MENU, 8, MENU, COMMON, 3}, // ** Switch 08 **
      {PAGE_VCTOUCH_MENU, 9, MENU, COMMON, 4}, // ** Switch 09 **
      {PAGE_VCTOUCH_MENU, 10, MENU, COMMON, 5}, // ** Switch 10 **
      {PAGE_VCTOUCH_MENU, 11, NOTHING, COMMON}, // ** Switch 11 **
      {PAGE_VCTOUCH_MENU, 12, MENU, COMMON, 13}, // ** Switch 12 **
      {PAGE_VCTOUCH_MENU, 13, MENU, COMMON, 14}, // ** Switch 13 **
      {PAGE_VCTOUCH_MENU, 14, MENU, COMMON, 11}, // ** Switch 14 **
      {PAGE_VCTOUCH_MENU, 15, MENU, COMMON, 12}, // ** Switch 15 **
      {PAGE_VCTOUCH_MENU, 16, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 1 or expr pedal 1
      {PAGE_VCTOUCH_MENU, 17, NOTHING, COMMON}, // External switch 2
      {PAGE_VCTOUCH_MENU, 18, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 3 or expr pedal 2
      {PAGE_VCTOUCH_MENU, 19, NOTHING, COMMON}, // External switch 4
      {PAGE_VCTOUCH_MENU, 20, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 5 or expr pedal 3
      {PAGE_VCTOUCH_MENU, 21, NOTHING, COMMON}, // External switch 6
      {PAGE_VCTOUCH_MENU, 22, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 7 or expr pedal 4
      {PAGE_VCTOUCH_MENU, 23, NOTHING, COMMON}, // External switch 8

      // ******************************* PAGE 202: Current device Direct Select *************************************************
      {PAGE_VCTOUCH_CURRENT_DIRECT_SELECT, LABEL, 'D', 'I', 'R', 'E', 'C', 'T', ' ', 'S' },
      {PAGE_VCTOUCH_CURRENT_DIRECT_SELECT, LABEL, 'E', 'L', 'E', 'C', 'T', ' ', ' ', ' ' },
      {PAGE_VCTOUCH_CURRENT_DIRECT_SELECT, 1, DIRECT_SELECT, CURRENT, 1, 10}, // ** Switch 01 **
      {PAGE_VCTOUCH_CURRENT_DIRECT_SELECT, 2, DIRECT_SELECT, CURRENT, 2, 10}, // ** Switch 02 **
      {PAGE_VCTOUCH_CURRENT_DIRECT_SELECT, 3, DIRECT_SELECT, CURRENT, 3, 10}, // ** Switch 03 **
      {PAGE_VCTOUCH_CURRENT_DIRECT_SELECT, 4, DIRECT_SELECT, CURRENT, 4, 10}, // ** Switch 04 **
      {PAGE_VCTOUCH_CURRENT_DIRECT_SELECT, 5, DIRECT_SELECT, CURRENT, 5, 10}, // ** Switch 05 **
      {PAGE_VCTOUCH_CURRENT_DIRECT_SELECT, 6, DIRECT_SELECT, CURRENT, 6, 10}, // ** Switch 06 **
      {PAGE_VCTOUCH_CURRENT_DIRECT_SELECT, 7, DIRECT_SELECT, CURRENT, 7, 10}, // ** Switch 07 **
      {PAGE_VCTOUCH_CURRENT_DIRECT_SELECT, 8, DIRECT_SELECT, CURRENT, 8, 10}, // ** Switch 08 **
      {PAGE_VCTOUCH_CURRENT_DIRECT_SELECT, 9, DIRECT_SELECT, CURRENT, 9, 10}, // ** Switch 09 **
      {PAGE_VCTOUCH_CURRENT_DIRECT_SELECT, 10, DIRECT_SELECT, CURRENT, 0, 10}, // ** Switch 10 **
      {PAGE_VCTOUCH_CURRENT_DIRECT_SELECT, 11, PATCH, CURRENT, BANKDOWN, 100}, // ** Switch 11 **
      {PAGE_VCTOUCH_CURRENT_DIRECT_SELECT, 12, PATCH, CURRENT, BANKUP, 100}, // ** Switch 12 **
      //{PAGE_VCTOUCH_CURRENT_DIRECT_SELECT, 14, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 14 **
      //{PAGE_VCTOUCH_CURRENT_DIRECT_SELECT, 15, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 15 **

      // ******************************* PAGE 203: DEVICE MODE *************************************************
      {PAGE_VCTOUCH_DEVICE_MODE, LABEL, 'D', 'E', 'V', 'I', 'C', 'E', ' ', 'M' },
      {PAGE_VCTOUCH_DEVICE_MODE, LABEL, 'O', 'D', 'E', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VCTOUCH_DEVICE_MODE, 1, NOTHING, COMMON}, // ** Switch 01 **
      {PAGE_VCTOUCH_DEVICE_MODE, 2, NOTHING, COMMON}, // ** Switch 02 **
      {PAGE_VCTOUCH_DEVICE_MODE, 3, NOTHING, COMMON}, // ** Switch 03 **
      {PAGE_VCTOUCH_DEVICE_MODE, 4, NOTHING, COMMON}, // ** Switch 04 **
      {PAGE_VCTOUCH_DEVICE_MODE, 5, NOTHING, COMMON}, // ** Switch 05 **
      {PAGE_VCTOUCH_DEVICE_MODE, 6, NOTHING, COMMON}, // ** Switch 06 **
      {PAGE_VCTOUCH_DEVICE_MODE, 7, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 07 **
      {PAGE_VCTOUCH_DEVICE_MODE, 8, PAGE, COMMON, SELECT, PAGE_VCTOUCH_CURRENT_DIRECT_SELECT}, // ** Switch 08 **
      {PAGE_VCTOUCH_DEVICE_MODE, 9, NOTHING, COMMON}, // ** Switch 09 **
      {PAGE_VCTOUCH_DEVICE_MODE, 10, NOTHING, COMMON}, // ** Switch 10 **
      {PAGE_VCTOUCH_DEVICE_MODE, 11, MODE, COMMON, SELECT, SONG_MODE}, // ** Switch 11 **
      {PAGE_VCTOUCH_DEVICE_MODE, 12, MODE, COMMON, SELECT, PAGE_MODE}, // ** Switch 12 **
      {PAGE_VCTOUCH_DEVICE_MODE, 13, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 13 **
      {PAGE_VCTOUCH_DEVICE_MODE, 14, PAGE, COMMON, SELECT, PAGE_VCTOUCH_SETLIST_SELECT}, // ** Switch 14 **
      {PAGE_VCTOUCH_DEVICE_MODE, 15, PAGE, COMMON, SELECT, PAGE_VCTOUCH_MENU}, // ** Switch 15 **

      // ******************************* PAGE 204: PAGE MODE *************************************************
      {PAGE_VCTOUCH_PAGE_MODE, LABEL, 'P', 'A', 'G', 'E', ' ', 'M', 'O', 'D' },
      {PAGE_VCTOUCH_PAGE_MODE, LABEL, 'E', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VCTOUCH_PAGE_MODE, 1, PAGE, COMMON, BANKDOWN, 8}, // ** Switch 01 **
      {PAGE_VCTOUCH_PAGE_MODE, 2, PAGE, COMMON, BANKSELECT, 1, 8}, // ** Switch 02 **
      {PAGE_VCTOUCH_PAGE_MODE, 3, PAGE, COMMON, BANKSELECT, 2, 8}, // ** Switch 03 **
      {PAGE_VCTOUCH_PAGE_MODE, 4, PAGE, COMMON, BANKSELECT, 3, 8}, // ** Switch 04 **
      {PAGE_VCTOUCH_PAGE_MODE, 5, PAGE, COMMON, BANKSELECT, 4, 8}, // ** Switch 05 **
      {PAGE_VCTOUCH_PAGE_MODE, 6, PAGE, COMMON, BANKUP, 8}, // ** Switch 06 **
      {PAGE_VCTOUCH_PAGE_MODE, 7, PAGE, COMMON, BANKSELECT, 5, 8}, // ** Switch 07 **
      {PAGE_VCTOUCH_PAGE_MODE, 8, PAGE, COMMON, BANKSELECT, 6, 8}, // ** Switch 08 **
      {PAGE_VCTOUCH_PAGE_MODE, 9, PAGE, COMMON, BANKSELECT, 7, 8}, // ** Switch 09 **
      {PAGE_VCTOUCH_PAGE_MODE, 10, PAGE, COMMON, BANKSELECT, 8, 8}, // ** Switch 10 **
      {PAGE_VCTOUCH_PAGE_MODE, 11, MODE, COMMON, SELECT, SONG_MODE}, // ** Switch 11 **
      {PAGE_VCTOUCH_PAGE_MODE, 12, MODE, COMMON, SELECT, PAGE_MODE}, // ** Switch 12 **
      {PAGE_VCTOUCH_PAGE_MODE, 13, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 13 **
      {PAGE_VCTOUCH_PAGE_MODE, 14, PAGE, COMMON, SELECT, PAGE_VCTOUCH_SETLIST_SELECT}, // ** Switch 14 **
      {PAGE_VCTOUCH_PAGE_MODE, 15, PAGE, COMMON, SELECT, PAGE_VCTOUCH_MENU}, // ** Switch 15 **

      // ******************************* PAGE 205: Current_patch_bank (10 buttons per page) *************************************************
      {PAGE_VCTOUCH_CURRENT_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
      {PAGE_VCTOUCH_CURRENT_PATCH_BANK, LABEL, 'N', 'K', ' ', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VCTOUCH_CURRENT_PATCH_BANK, 1, PATCH, CURRENT, BANKSELECT, 1, 10}, // ** Switch 01 **
      {PAGE_VCTOUCH_CURRENT_PATCH_BANK, 2, PATCH, CURRENT, BANKSELECT, 2, 10}, // ** Switch 02 **
      {PAGE_VCTOUCH_CURRENT_PATCH_BANK, 3, PATCH, CURRENT, BANKSELECT, 3, 10}, // ** Switch 03 **
      {PAGE_VCTOUCH_CURRENT_PATCH_BANK, 4, PATCH, CURRENT, BANKSELECT, 4, 10}, // ** Switch 04 **
      {PAGE_VCTOUCH_CURRENT_PATCH_BANK, 5, PATCH, CURRENT, BANKSELECT, 5, 10}, // ** Switch 05 **
      {PAGE_VCTOUCH_CURRENT_PATCH_BANK, 6, PATCH, CURRENT, BANKSELECT, 6, 10}, // ** Switch 06 **
      {PAGE_VCTOUCH_CURRENT_PATCH_BANK, 7, PATCH, CURRENT, BANKSELECT, 7, 10}, // ** Switch 07 **
      {PAGE_VCTOUCH_CURRENT_PATCH_BANK, 8, PATCH, CURRENT, BANKSELECT, 8, 10}, // ** Switch 08 **
      {PAGE_VCTOUCH_CURRENT_PATCH_BANK, 9, PATCH, CURRENT, BANKSELECT, 9, 10}, // ** Switch 09 **
      {PAGE_VCTOUCH_CURRENT_PATCH_BANK, 10, PATCH, CURRENT, BANKSELECT, 10, 10}, // ** Switch 10 **
      {PAGE_VCTOUCH_CURRENT_PATCH_BANK, 11, PATCH, CURRENT, BANKDOWN, 10}, // ** Switch 13 **
      {PAGE_VCTOUCH_CURRENT_PATCH_BANK, 11 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 13 **
      {PAGE_VCTOUCH_CURRENT_PATCH_BANK, 12, PATCH, CURRENT, BANKUP, 10}, // ** Switch 14 **
      {PAGE_VCTOUCH_CURRENT_PATCH_BANK, 12 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 14 **

      // ******************************* PAGE 206: GR55 select *************************************************
      {PAGE_VCTOUCH_GR55_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
      {PAGE_VCTOUCH_GR55_PATCH_BANK, LABEL, 'N', 'K', ' ', 'G', 'R', ' ', ' ', ' ' },
      {PAGE_VCTOUCH_GR55_PATCH_BANK, 1, PATCH, GR55, BANKDOWN, 6}, // ** Switch 01 **
      {PAGE_VCTOUCH_GR55_PATCH_BANK, 2, PATCH, GR55, BANKSELECT, 1, 6}, // ** Switch 02 **
      {PAGE_VCTOUCH_GR55_PATCH_BANK, 3, PATCH, GR55, BANKSELECT, 2, 6}, // ** Switch 03 **
      {PAGE_VCTOUCH_GR55_PATCH_BANK, 4, PATCH, GR55, BANKSELECT, 3, 6}, // ** Switch 04 **
      {PAGE_VCTOUCH_GR55_PATCH_BANK, 5, ASSIGN, GR55, SELECT, 0}, // ** Switch 05 **
      {PAGE_VCTOUCH_GR55_PATCH_BANK, 6, PATCH, GR55, BANKUP, 6}, // ** Switch 06 **
      {PAGE_VCTOUCH_GR55_PATCH_BANK, 7, PATCH, GR55, BANKSELECT, 4, 6}, // ** Switch 07 **
      {PAGE_VCTOUCH_GR55_PATCH_BANK, 8, PATCH, GR55, BANKSELECT, 5, 6}, // ** Switch 08 **
      {PAGE_VCTOUCH_GR55_PATCH_BANK, 9, PATCH, GR55, BANKSELECT, 6, 6}, // ** Switch 09 **
      {PAGE_VCTOUCH_GR55_PATCH_BANK, 10, ASSIGN, GR55, SELECT, 1}, // ** Switch 10 **
      {PAGE_VCTOUCH_GR55_PATCH_BANK, 11, PATCH, GR55, PREV}, // ** Switch 11 **
      {PAGE_VCTOUCH_GR55_PATCH_BANK, 11 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 11 **
      {PAGE_VCTOUCH_GR55_PATCH_BANK, 12, PATCH, GR55, NEXT}, // ** Switch 12 **
      {PAGE_VCTOUCH_GR55_PATCH_BANK, 12 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 12 **

      // ******************************* PAGE 207: Zoom patch bank *************************************************
      {PAGE_VCTOUCH_ZOOM_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', '+', ' ' },
      {PAGE_VCTOUCH_ZOOM_PATCH_BANK, LABEL, 'F', 'X', ' ', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VCTOUCH_ZOOM_PATCH_BANK, 1, PATCH, CURRENT, BANKSELECT, 1, 5}, // ** Switch 01 **
      {PAGE_VCTOUCH_ZOOM_PATCH_BANK, 2, PATCH, CURRENT, BANKSELECT, 2, 5}, // ** Switch 02 **
      {PAGE_VCTOUCH_ZOOM_PATCH_BANK, 3, PATCH, CURRENT, BANKSELECT, 3, 5}, // ** Switch 03 **
      {PAGE_VCTOUCH_ZOOM_PATCH_BANK, 4, PATCH, CURRENT, BANKSELECT, 4, 5}, // ** Switch 04 **
      {PAGE_VCTOUCH_ZOOM_PATCH_BANK, 5, PATCH, CURRENT, BANKSELECT, 5, 5}, // ** Switch 05 **
      {PAGE_VCTOUCH_ZOOM_PATCH_BANK, 6, PARAMETER, CURRENT, 0, TOGGLE, 1, 0}, // ** Switch 06 **
      {PAGE_VCTOUCH_ZOOM_PATCH_BANK, 7, PARAMETER, CURRENT, 1, TOGGLE, 1, 0}, // ** Switch 07 **
      {PAGE_VCTOUCH_ZOOM_PATCH_BANK, 8, PARAMETER, CURRENT, 2, TOGGLE, 1, 0}, // ** Switch 08 **
      {PAGE_VCTOUCH_ZOOM_PATCH_BANK, 9, PARAMETER, CURRENT, 3, TOGGLE, 1, 0}, // ** Switch 09 **
      {PAGE_VCTOUCH_ZOOM_PATCH_BANK, 10, PARAMETER, CURRENT, 4, TOGGLE, 1, 0}, // ** Switch 10 **
      {PAGE_VCTOUCH_ZOOM_PATCH_BANK, 11, PATCH, CURRENT, BANKDOWN, 5}, // ** Switch 11 **
      {PAGE_VCTOUCH_ZOOM_PATCH_BANK, 11 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 11 **
      {PAGE_VCTOUCH_ZOOM_PATCH_BANK, 12, PATCH, CURRENT, BANKUP, 5}, // ** Switch 12 **
      {PAGE_VCTOUCH_ZOOM_PATCH_BANK, 12 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 12 **

      // ******************************* PAGE 208: Parameters current device *************************************************
      {PAGE_VCTOUCH_CURRENT_PARAMETER, LABEL, 'P', 'A', 'R', ' ', 'B', 'A', 'N', 'K' },
      {PAGE_VCTOUCH_CURRENT_PARAMETER, 1, PAR_BANK, CURRENT, 1, 10}, // ** Switch 01 **
      {PAGE_VCTOUCH_CURRENT_PARAMETER, 2, PAR_BANK, CURRENT, 2, 10}, // ** Switch 02 **
      {PAGE_VCTOUCH_CURRENT_PARAMETER, 3, PAR_BANK, CURRENT, 3, 10}, // ** Switch 03 **
      {PAGE_VCTOUCH_CURRENT_PARAMETER, 4, PAR_BANK, CURRENT, 4, 10}, // ** Switch 04 **
      {PAGE_VCTOUCH_CURRENT_PARAMETER, 5, PAR_BANK, CURRENT, 5, 10}, // ** Switch 05 **
      {PAGE_VCTOUCH_CURRENT_PARAMETER, 6, PAR_BANK, CURRENT, 6, 10}, // ** Switch 06 **
      {PAGE_VCTOUCH_CURRENT_PARAMETER, 7, PAR_BANK, CURRENT, 7, 10}, // ** Switch 07 **
      {PAGE_VCTOUCH_CURRENT_PARAMETER, 8, PAR_BANK, CURRENT, 8, 10}, // ** Switch 08 **
      {PAGE_VCTOUCH_CURRENT_PARAMETER, 9, PAR_BANK, CURRENT, 9, 10}, // ** Switch 09 **
      {PAGE_VCTOUCH_CURRENT_PARAMETER, 10, PAR_BANK, CURRENT, 10, 10}, // ** Switch 10 **
      {PAGE_VCTOUCH_CURRENT_PARAMETER, 11, PAR_BANK_DOWN, CURRENT, 10}, // ** Switch 11 **
      {PAGE_VCTOUCH_CURRENT_PARAMETER, 12, PAR_BANK_UP, CURRENT, 10}, // ** Switch 12 **

      // ******************************* PAGE 209: Parameters current device *************************************************
      {PAGE_VCTOUCH_EDIT_PARAMETER, LABEL, 'E', 'D', 'I', 'T', ' ', 'P', 'A', 'R' },
      {PAGE_VCTOUCH_EDIT_PARAMETER, LABEL, 'A', 'M', 'E', 'T', 'E', 'R', 'S', ' ' },
      {PAGE_VCTOUCH_EDIT_PARAMETER, 1, PAR_BANK, CURRENT, 1, 10}, // ** Switch 01 **
      {PAGE_VCTOUCH_EDIT_PARAMETER, 2, PAR_BANK, CURRENT, 2, 10}, // ** Switch 02 **
      {PAGE_VCTOUCH_EDIT_PARAMETER, 3, PAR_BANK, CURRENT, 3, 10}, // ** Switch 03 **
      {PAGE_VCTOUCH_EDIT_PARAMETER, 4, PAR_BANK, CURRENT, 4, 10}, // ** Switch 04 **
      {PAGE_VCTOUCH_EDIT_PARAMETER, 5, PAR_BANK, CURRENT, 5, 10}, // ** Switch 05 **
      {PAGE_VCTOUCH_EDIT_PARAMETER, 6, PAR_BANK, CURRENT, 6, 10}, // ** Switch 06 **
      {PAGE_VCTOUCH_EDIT_PARAMETER, 7, PAR_BANK, CURRENT, 7, 10}, // ** Switch 07 **
      {PAGE_VCTOUCH_EDIT_PARAMETER, 8, PAR_BANK, CURRENT, 8, 10}, // ** Switch 08 **
      {PAGE_VCTOUCH_EDIT_PARAMETER, 9, PAR_BANK, CURRENT, 9, 10}, // ** Switch 09 **
      {PAGE_VCTOUCH_EDIT_PARAMETER, 10, PAR_BANK, CURRENT, 10, 10}, // ** Switch 10 **
      {PAGE_VCTOUCH_EDIT_PARAMETER, 11, PAR_BANK_DOWN, CURRENT, 10}, // ** Switch 11 **
      {PAGE_VCTOUCH_EDIT_PARAMETER, 12, PAR_BANK_UP, CURRENT, 10}, // ** Switch 12 **
      {PAGE_VCTOUCH_EDIT_PARAMETER, 14, NOTHING, COMMON}, // ** Switch 14 **
      {PAGE_VCTOUCH_EDIT_PARAMETER, 15, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 15 **
      {PAGE_VCTOUCH_EDIT_PARAMETER, 15 | LABEL, 'B', 'A', 'C', 'K', ' ', ' ', ' ', ' '}, // ** Switch 15 **

      // ******************************* PAGE 210: GP10 assign *************************************************
      {PAGE_VCTOUCH_GP10_ASSIGNS, LABEL, 'A', 'S', 'S', 'G', 'N', ' ', 'G', 'P' },
      {PAGE_VCTOUCH_GP10_ASSIGNS, LABEL, '1', '0', ' ', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VCTOUCH_GP10_ASSIGNS, 1, ASSIGN, GP10, SELECT, 0, 21}, // ** Switch 01 **
      {PAGE_VCTOUCH_GP10_ASSIGNS, 2, ASSIGN, GP10, SELECT, 1, 22}, // ** Switch 02 **
      {PAGE_VCTOUCH_GP10_ASSIGNS, 3, ASSIGN, GP10, SELECT, 2, 23}, // ** Switch 03 **
      {PAGE_VCTOUCH_GP10_ASSIGNS, 4, ASSIGN, GP10, SELECT, 3, 24}, // ** Switch 04 **
      {PAGE_VCTOUCH_GP10_ASSIGNS, 5, NOTHING, COMMON}, // ** Switch 05 **
      {PAGE_VCTOUCH_GP10_ASSIGNS, 6, ASSIGN, GP10, SELECT, 4, 25}, // ** Switch 06 **
      {PAGE_VCTOUCH_GP10_ASSIGNS, 7, ASSIGN, GP10, SELECT, 5, 26}, // ** Switch 07 **
      {PAGE_VCTOUCH_GP10_ASSIGNS, 8, ASSIGN, GP10, SELECT, 6, 27}, // ** Switch 08 **
      {PAGE_VCTOUCH_GP10_ASSIGNS, 9, ASSIGN, GP10, SELECT, 7, 28}, // ** Switch 09 **
      {PAGE_VCTOUCH_GP10_ASSIGNS, 10, NOTHING, COMMON}, // ** Switch 10 **
      {PAGE_VCTOUCH_GP10_ASSIGNS, 11, PATCH, GP10, PREV}, // ** Switch 13 **
      {PAGE_VCTOUCH_GP10_ASSIGNS, 12, PATCH, GP10, NEXT}, // ** Switch 14 **

      // ******************************* PAGE 211: GR55 Assign *************************************************
      {PAGE_VCTOUCH_GR55_ASSIGNS, LABEL, 'A', 'S', 'S', 'G', 'N', ' ', 'G', 'R'},
      {PAGE_VCTOUCH_GR55_ASSIGNS, LABEL, '5', '5', ' ', ' ', ' ', ' ', ' ', ' '},
      {PAGE_VCTOUCH_GR55_ASSIGNS, 1, PARAMETER, GR55, 4, TOGGLE, 1, 0}, // ** Switch 01 **
      {PAGE_VCTOUCH_GR55_ASSIGNS, 2, PARAMETER, GR55, 6, TOGGLE, 1, 0}, // ** Switch 02 **
      {PAGE_VCTOUCH_GR55_ASSIGNS, 3, PARAMETER, GR55, 8, TOGGLE, 1, 0}, // ** Switch 03 **
      {PAGE_VCTOUCH_GR55_ASSIGNS, 4, PARAMETER, GR55, 9, TOGGLE, 1, 0}, // ** Switch 04 **
      {PAGE_VCTOUCH_GR55_ASSIGNS, 5, ASSIGN, GR55, SELECT, 0}, // ** Switch 05 **
      {PAGE_VCTOUCH_GR55_ASSIGNS, 6, ASSIGN, GR55, SELECT, 4}, // ** Switch 06 **
      {PAGE_VCTOUCH_GR55_ASSIGNS, 7, ASSIGN, GR55, SELECT, 5}, // ** Switch 07 **
      {PAGE_VCTOUCH_GR55_ASSIGNS, 8, ASSIGN, GR55, SELECT, 6}, // ** Switch 08 **
      {PAGE_VCTOUCH_GR55_ASSIGNS, 9, ASSIGN, GR55, SELECT, 7}, // ** Switch 09 **
      {PAGE_VCTOUCH_GR55_ASSIGNS, 10, ASSIGN, GR55, SELECT, 1}, // ** Switch 10 **
      {PAGE_VCTOUCH_GR55_ASSIGNS, 11, PATCH, GR55, PREV}, // ** Switch 11 **
      {PAGE_VCTOUCH_GR55_ASSIGNS, 12, PATCH, GR55, NEXT}, // ** Switch 12 **

      // ******************************* PAGE 212: VG99 Edit *************************************************
      {PAGE_VCTOUCH_VG99_EDIT, LABEL, 'V', 'G', '9', '9', ' ', 'E', 'D', 'I'},
      {PAGE_VCTOUCH_VG99_EDIT, LABEL, 'T', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
      {PAGE_VCTOUCH_VG99_EDIT, 1, PAR_BANK_CATEGORY, VG99, 1}, // ** Switch 01 **
      {PAGE_VCTOUCH_VG99_EDIT, 2, PAR_BANK_CATEGORY, VG99, 9}, // ** Switch 02 **
      {PAGE_VCTOUCH_VG99_EDIT, 3, PAR_BANK_CATEGORY, VG99, 2}, // ** Switch 03 **
      {PAGE_VCTOUCH_VG99_EDIT, 4, PAR_BANK_CATEGORY, VG99, 3}, // ** Switch 04 **
      {PAGE_VCTOUCH_VG99_EDIT, 5, PAR_BANK_CATEGORY, VG99, 4}, // ** Switch 05 **
      {PAGE_VCTOUCH_VG99_EDIT, 6, PAR_BANK_CATEGORY, VG99, 5}, // ** Switch 06 **
      {PAGE_VCTOUCH_VG99_EDIT, 7, PAR_BANK_CATEGORY, VG99, 10}, // ** Switch 07 **
      {PAGE_VCTOUCH_VG99_EDIT, 8, PAR_BANK_CATEGORY, VG99, 6}, // ** Switch 08 **
      {PAGE_VCTOUCH_VG99_EDIT, 9, PAR_BANK_CATEGORY, VG99, 7}, // ** Switch 09 **
      {PAGE_VCTOUCH_VG99_EDIT, 10, PAR_BANK_CATEGORY, VG99, 8}, // ** Switch 10 **
      {PAGE_VCTOUCH_VG99_EDIT, 11, PAR_BANK_CATEGORY, VG99, 11 }, // ** Switch 11 **
      {PAGE_VCTOUCH_VG99_EDIT, 12, PAR_BANK_CATEGORY, VG99, 12}, // ** Switch 12 **
      {PAGE_VCTOUCH_VG99_EDIT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      //{PAGE_VCTOUCH_VG99_EDIT, 15, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 12 **

      // ******************************* PAGE 213: Line 6 M13 effects *************************************************
      {PAGE_VCTOUCH_M13_PARAMETER, LABEL, 'L', 'I', 'N', 'E', '6', ' ', 'M', '1' },
      {PAGE_VCTOUCH_M13_PARAMETER, LABEL, '3', ' ', 'F', 'X', ' ', ' ', ' ', ' ' },
      {PAGE_VCTOUCH_M13_PARAMETER, 1, PARAMETER, M13, 0, TOGGLE, 1, 0}, // ** Switch 01 **
      {PAGE_VCTOUCH_M13_PARAMETER, 2, PARAMETER, M13, 3, TOGGLE, 1, 0}, // ** Switch 02 **
      {PAGE_VCTOUCH_M13_PARAMETER, 3, PARAMETER, M13, 6, TOGGLE, 1, 0}, // ** Switch 03 **
      {PAGE_VCTOUCH_M13_PARAMETER, 4, PARAMETER, M13, 9, TOGGLE, 1, 0}, // ** Switch 04 **
      {PAGE_VCTOUCH_M13_PARAMETER, 5, PATCH, M13, PREV}, // ** Switch 5 **
      {PAGE_VCTOUCH_M13_PARAMETER, 6, PARAMETER, M13, 1, TOGGLE, 1, 0}, // ** Switch 06 **
      {PAGE_VCTOUCH_M13_PARAMETER, 7, PARAMETER, M13, 4, TOGGLE, 1, 0}, // ** Switch 07 **
      {PAGE_VCTOUCH_M13_PARAMETER, 8, PARAMETER, M13, 7, TOGGLE, 1, 0}, // ** Switch 08 **
      {PAGE_VCTOUCH_M13_PARAMETER, 9, PARAMETER, M13, 10, TOGGLE, 1, 0}, // ** Switch 09 **
      {PAGE_VCTOUCH_M13_PARAMETER, 10, PATCH, M13, NEXT}, // ** Switch 10 **
      {PAGE_VCTOUCH_M13_PARAMETER, 11, PARAMETER, M13, 2, TOGGLE, 1, 0}, // ** Switch 11 **
      {PAGE_VCTOUCH_M13_PARAMETER, 12, PARAMETER, M13, 5, TOGGLE, 1, 0}, // ** Switch 12 **
      {PAGE_VCTOUCH_M13_PARAMETER, 13, PARAMETER, M13, 8, TOGGLE, 1, 0}, // ** Switch 13 **
      {PAGE_VCTOUCH_M13_PARAMETER, 14, PARAMETER, M13, 11, TOGGLE, 1, 0}, // ** Switch 14 **
      {PAGE_VCTOUCH_M13_PARAMETER, 15, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 15 **

      // ******************************* PAGE 214: Looper control *************************************************
      {PAGE_VCTOUCH_FULL_LOOPER, LABEL, 'L', 'O', 'O', 'P', 'E', 'R', ' ', 'C' },
      {PAGE_VCTOUCH_FULL_LOOPER, LABEL, 'O', 'N', 'T', 'R', 'O', 'L', ' ', ' ' },
      {PAGE_VCTOUCH_FULL_LOOPER, 1, LOOPER, CURRENT, LOOPER_REC_OVERDUB}, // ** Switch 01 **
      {PAGE_VCTOUCH_FULL_LOOPER, 2, LOOPER, CURRENT, LOOPER_PLAY_STOP}, // ** Switch 02 **
      {PAGE_VCTOUCH_FULL_LOOPER, 3, LOOPER, CURRENT, LOOPER_HALF_SPEED}, // ** Switch 03 **
      {PAGE_VCTOUCH_FULL_LOOPER, 4, LOOPER, CURRENT, LOOPER_REVERSE}, // ** Switch 04 **
      {PAGE_VCTOUCH_FULL_LOOPER, 6, LOOPER, CURRENT, LOOPER_UNDO_REDO}, // ** Switch 06 **
      {PAGE_VCTOUCH_FULL_LOOPER, 7, LOOPER, CURRENT, LOOPER_PLAY_ONCE}, // ** Switch 07 **
      {PAGE_VCTOUCH_FULL_LOOPER, 8, LOOPER, CURRENT, LOOPER_PRE_POST}, // ** Switch 08 **
      {PAGE_VCTOUCH_FULL_LOOPER, 9, LOOPER, CURRENT, LOOPER_SHOW_HIDE}, // ** Switch 09 **
      {PAGE_VCTOUCH_FULL_LOOPER, 11, PATCH, CURRENT, PREV}, // ** Switch 13 **
      {PAGE_VCTOUCH_FULL_LOOPER, 12, PATCH, CURRENT, NEXT}, // ** Switch 14 **
      //{PAGE_VCTOUCH_FULL_LOOPER, 15, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

      // ******************************* PAGE 215: Helix_patch_bank (8 buttons per page) *************************************************
      {PAGE_VCTOUCH_HLX_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
      {PAGE_VCTOUCH_HLX_PATCH_BANK, LABEL, 'N', 'K', ' ', 'H', 'E', 'L', 'I', 'X' },
      {PAGE_VCTOUCH_HLX_PATCH_BANK, 1, PATCH, HLX, BANKSELECT, 1, 8}, // ** Switch 01 **
      {PAGE_VCTOUCH_HLX_PATCH_BANK, 2, PATCH, HLX, BANKSELECT, 2, 8}, // ** Switch 02 **
      {PAGE_VCTOUCH_HLX_PATCH_BANK, 3, PATCH, HLX, BANKSELECT, 3, 8}, // ** Switch 03 **
      {PAGE_VCTOUCH_HLX_PATCH_BANK, 4, PATCH, HLX, BANKSELECT, 4, 8}, // ** Switch 04 **
      {PAGE_VCTOUCH_HLX_PATCH_BANK, 5, PAGE, COMMON, SELECT, PAGE_VCTOUCH_FULL_LOOPER}, // ** Switch 05 **
      {PAGE_VCTOUCH_HLX_PATCH_BANK, 6, PATCH, HLX, BANKSELECT, 5, 8}, // ** Switch 06 **
      {PAGE_VCTOUCH_HLX_PATCH_BANK, 7, PATCH, HLX, BANKSELECT, 6, 8}, // ** Switch 07 **
      {PAGE_VCTOUCH_HLX_PATCH_BANK, 8, PATCH, HLX, BANKSELECT, 7, 8}, // ** Switch 08 **
      {PAGE_VCTOUCH_HLX_PATCH_BANK, 9, PATCH, HLX, BANKSELECT, 8, 8}, // ** Switch 09 **
      {PAGE_VCTOUCH_HLX_PATCH_BANK, 10, PARAMETER, HLX, 14, STEP, 0, 6, 1}, // ** Switch 10 **
      {PAGE_VCTOUCH_HLX_PATCH_BANK, 11, PATCH, HLX, BANKDOWN, 8}, // ** Switch 11 **
      {PAGE_VCTOUCH_HLX_PATCH_BANK, 11 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 11 **
      {PAGE_VCTOUCH_HLX_PATCH_BANK, 12, PATCH, HLX, BANKUP, 8}, // ** Switch 12 **
      {PAGE_VCTOUCH_HLX_PATCH_BANK, 12 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 12 **
      //{PAGE_VCTOUCH_HLX_PATCH_BANK, 15, OPEN_NEXT_PAGE_OF_DEVICE, HLX}, // ** Switch 15 **

      // ******************************* PAGE 216: Helix_parameter *************************************************
      {PAGE_VCTOUCH_HLX_PARAMETER, LABEL, 'P', 'A', 'R', 'A', 'M', 'E', 'T', 'E' },
      {PAGE_VCTOUCH_HLX_PARAMETER, LABEL, 'R', 'S', ' ', 'H', 'E', 'L', 'I', 'X' },
      {PAGE_VCTOUCH_HLX_PARAMETER, 1, PARAMETER, HLX, 6, MOMENTARY, 127, 0}, // ** Switch 01 **
      {PAGE_VCTOUCH_HLX_PARAMETER, 2, PARAMETER, HLX, 7, MOMENTARY, 127, 0}, // ** Switch 02 **
      {PAGE_VCTOUCH_HLX_PARAMETER, 3, PARAMETER, HLX, 8, MOMENTARY, 127, 0}, // ** Switch 03 **
      {PAGE_VCTOUCH_HLX_PARAMETER, 4, PARAMETER, HLX, 9, MOMENTARY, 127, 0}, // ** Switch 04 **
      {PAGE_VCTOUCH_HLX_PARAMETER, 5, LOOPER, HLX, LOOPER_REC_OVERDUB}, // ** Switch 05 **
      {PAGE_VCTOUCH_HLX_PARAMETER, 6, PARAMETER, HLX, 1, MOMENTARY, 127, 0}, // ** Switch 06 **
      {PAGE_VCTOUCH_HLX_PARAMETER, 7, PARAMETER, HLX, 2, MOMENTARY, 127, 0}, // ** Switch 07 **
      {PAGE_VCTOUCH_HLX_PARAMETER, 8, PARAMETER, HLX, 3, MOMENTARY, 127, 0}, // ** Switch 08 **
      {PAGE_VCTOUCH_HLX_PARAMETER, 9, PARAMETER, HLX, 4, MOMENTARY, 127, 0}, // ** Switch 09 **
      {PAGE_VCTOUCH_HLX_PARAMETER, 10, LOOPER, HLX, LOOPER_PLAY_STOP}, // ** Switch 10 **
      {PAGE_VCTOUCH_HLX_PARAMETER, 11, PATCH, HLX, PREV}, // ** Switch 11 **
      {PAGE_VCTOUCH_HLX_PARAMETER, 12, PATCH, HLX, NEXT}, // ** Switch 12 **
      //{PAGE_VCTOUCH_HLX_PARAMETER, 15, OPEN_NEXT_PAGE_OF_DEVICE, HLX}, // ** Switch 15 **

      // ******************************* PAGE 217: Snapshots (Helix) /scenes (AxeFX) *************************************************
      {PAGE_VCTOUCH_SNAPSCENE_LOOPER, LABEL, 'S', 'N', 'A', 'P', 'S', 'C', 'E', 'N' },
      {PAGE_VCTOUCH_SNAPSCENE_LOOPER, LABEL, 'E', '/', 'L', 'O', 'O', 'P', 'E', 'R' },
      {PAGE_VCTOUCH_SNAPSCENE_LOOPER, 1, SNAPSCENE, CURRENT, 1, 0, 0}, // ** Switch 01 **
      {PAGE_VCTOUCH_SNAPSCENE_LOOPER, 2, SNAPSCENE, CURRENT, 2, 0, 0}, // ** Switch 02 **
      {PAGE_VCTOUCH_SNAPSCENE_LOOPER, 3, SNAPSCENE, CURRENT, 3, 0, 0}, // ** Switch 03 **
      {PAGE_VCTOUCH_SNAPSCENE_LOOPER, 4, SNAPSCENE, CURRENT, 4, 0, 0}, // ** Switch 04 **
      {PAGE_VCTOUCH_SNAPSCENE_LOOPER, 5, LOOPER, CURRENT, LOOPER_REC_OVERDUB}, // ** Switch 05 **
      {PAGE_VCTOUCH_SNAPSCENE_LOOPER, 6, SNAPSCENE, CURRENT, 5, 0, 0}, // ** Switch 06 **
      {PAGE_VCTOUCH_SNAPSCENE_LOOPER, 7, SNAPSCENE, CURRENT, 6, 0, 0}, // ** Switch 07 **
      {PAGE_VCTOUCH_SNAPSCENE_LOOPER, 8, SNAPSCENE, CURRENT, 7, 0, 0}, // ** Switch 08 **
      {PAGE_VCTOUCH_SNAPSCENE_LOOPER, 9, SNAPSCENE, CURRENT, 8, 0, 0}, // ** Switch 09 **
      {PAGE_VCTOUCH_SNAPSCENE_LOOPER, 10, LOOPER, CURRENT, LOOPER_PLAY_STOP}, // ** Switch 10 **
      {PAGE_VCTOUCH_SNAPSCENE_LOOPER, 11, PATCH, CURRENT, PREV}, // ** Switch 11 **
      {PAGE_VCTOUCH_SNAPSCENE_LOOPER, 12, PATCH, CURRENT, NEXT}, // ** Switch 12 **
      //{PAGE_VCTOUCH_SNAPSCENE_LOOPER, 15, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 15 **

      // ******************************* PAGE 218: KATANA_patch_bank (8 buttons per page) *************************************************
      {PAGE_VCTOUCH_KTN_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', 'B', 'A', 'N' },
      {PAGE_VCTOUCH_KTN_PATCH_BANK, LABEL, 'K', ' ', 'K', 'A', 'T', 'A', 'N', 'A' },
      {PAGE_VCTOUCH_KTN_PATCH_BANK, 1, PATCH, KTN, BANKSELECT, 2, 8}, // ** Switch 01 **
      {PAGE_VCTOUCH_KTN_PATCH_BANK, 2, PATCH, KTN, BANKSELECT, 3, 8}, // ** Switch 02 **
      {PAGE_VCTOUCH_KTN_PATCH_BANK, 3, PATCH, KTN, BANKSELECT, 4, 8}, // ** Switch 03 **
      {PAGE_VCTOUCH_KTN_PATCH_BANK, 4, PATCH, KTN, BANKSELECT, 5, 8}, // ** Switch 04 **
      {PAGE_VCTOUCH_KTN_PATCH_BANK, 5, PATCH, KTN, SELECT, 0, 0}, // ** Switch 05 **
      {PAGE_VCTOUCH_KTN_PATCH_BANK, 6, PATCH, KTN, BANKSELECT, 6, 8}, // ** Switch 06 **
      {PAGE_VCTOUCH_KTN_PATCH_BANK, 7, PATCH, KTN, BANKSELECT, 7, 8}, // ** Switch 07 **
      {PAGE_VCTOUCH_KTN_PATCH_BANK, 8, PATCH, KTN, BANKSELECT, 8, 8}, // ** Switch 08 **
      {PAGE_VCTOUCH_KTN_PATCH_BANK, 9, PATCH, KTN, BANKSELECT, 9, 8}, // ** Switch 09 **
      {PAGE_VCTOUCH_KTN_PATCH_BANK, 10, OPEN_PAGE_DEVICE, KTN, PAGE_VCTOUCH_KTN_EDIT}, // ** Switch 10 **
      {PAGE_VCTOUCH_KTN_PATCH_BANK, 11, PATCH, KTN, BANKDOWN, 8}, // ** Switch 11 **
      {PAGE_VCTOUCH_KTN_PATCH_BANK, 11 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 11 **
      {PAGE_VCTOUCH_KTN_PATCH_BANK, 12, PATCH, KTN, BANKUP, 8}, // ** Switch 12 **
      {PAGE_VCTOUCH_KTN_PATCH_BANK, 12 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 12 **
      //{PAGE_VCTOUCH_KTN_PATCH_BANK, 15, OPEN_NEXT_PAGE_OF_DEVICE, KTN}, // ** Switch 15 **

      // ******************************* PAGE 219: KATANA Edit *************************************************
      {PAGE_VCTOUCH_KTN_EDIT, LABEL, 'K', 'A', 'T', 'A', 'N', 'A', ' ', 'E'},
      {PAGE_VCTOUCH_KTN_EDIT, LABEL, 'D', 'I', 'T', ' ', ' ', ' ', ' ', ' '},
      {PAGE_VCTOUCH_KTN_EDIT, 1, PAR_BANK_CATEGORY, KTN, 1}, // ** Switch 01 **
      {PAGE_VCTOUCH_KTN_EDIT, 2, PAR_BANK_CATEGORY, KTN, 2}, // ** Switch 02 **
      {PAGE_VCTOUCH_KTN_EDIT, 3, PAR_BANK_CATEGORY, KTN, 3}, // ** Switch 03 **
      {PAGE_VCTOUCH_KTN_EDIT, 4, PAR_BANK_CATEGORY, KTN, 4}, // ** Switch 04 **
      {PAGE_VCTOUCH_KTN_EDIT, 5, PAR_BANK_CATEGORY, KTN, 5}, // ** Switch 05 **
      {PAGE_VCTOUCH_KTN_EDIT, 6, PAR_BANK_CATEGORY, KTN, 6}, // ** Switch 06 **
      {PAGE_VCTOUCH_KTN_EDIT, 7, PAR_BANK_CATEGORY, KTN, 7}, // ** Switch 07 **
      {PAGE_VCTOUCH_KTN_EDIT, 8, PAR_BANK_CATEGORY, KTN, 8}, // ** Switch 08 **
      {PAGE_VCTOUCH_KTN_EDIT, 9, PAR_BANK_CATEGORY, KTN, 9}, // ** Switch 09 **
      {PAGE_VCTOUCH_KTN_EDIT, 10, PAR_BANK_CATEGORY, KTN, 10}, // ** Switch 10 **
      {PAGE_VCTOUCH_KTN_EDIT, 11, PAR_BANK_CATEGORY, KTN, 11 }, // ** Switch 11 **
      {PAGE_VCTOUCH_KTN_EDIT, 12, SAVE_PATCH, KTN }, // ** Switch 11 **
      //{PAGE_VCTOUCH_KTN_EDIT, 15, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 15 **

      // ******************************* PAGE 220: KATANA FX CTRL  *************************************************
      {PAGE_VCTOUCH_KTN_FX, LABEL, 'K', 'A', 'T', 'A', 'N', 'A', ' ', 'F'},
      {PAGE_VCTOUCH_KTN_FX, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', ' '},
      {PAGE_VCTOUCH_KTN_FX, 1, PARAMETER, KTN, 0, TOGGLE, 1, 0}, // ** Switch 01 **
      {PAGE_VCTOUCH_KTN_FX, 2, PARAMETER, KTN, 8, TOGGLE, 1, 0}, // ** Switch 02 **
      {PAGE_VCTOUCH_KTN_FX, 3, PARAMETER, KTN, 23, TOGGLE, 1, 0}, // ** Switch 03 **
      {PAGE_VCTOUCH_KTN_FX, 4, PARAMETER, KTN, 70, TOGGLE, 1, 0}, // ** Switch 04 **
      {PAGE_VCTOUCH_KTN_FX, 5, PARAMETER, KTN, 61, TOGGLE, 1, 0}, // ** Switch 05 **
      {PAGE_VCTOUCH_KTN_FX, 6, PARAMETER, KTN, 87, TOGGLE, 1, 0}, // ** Switch 06 **
      {PAGE_VCTOUCH_KTN_FX, 7, PARAMETER, KTN, 96, TOGGLE, 1, 0}, // ** Switch 07 **
      {PAGE_VCTOUCH_KTN_FX, 8, PARAMETER, KTN, 48, TOGGLE, 1, 0}, // ** Switch 08 **
      {PAGE_VCTOUCH_KTN_FX, 9, PARAMETER, KTN, 106, TOGGLE, 1, 0}, // ** Switch 09 **
      {PAGE_VCTOUCH_KTN_FX, 10, OPEN_PAGE_DEVICE, KTN, PAGE_VCTOUCH_KTN_EDIT}, // ** Switch 10 **
      {PAGE_VCTOUCH_KTN_FX, 11, PATCH, KTN, PREV}, // ** Switch 11 **
      {PAGE_VCTOUCH_KTN_FX, 12, PATCH, KTN, NEXT}, // ** Switch 12 **
      //{PAGE_VCTOUCH_KTN_FX, 15, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 15 **

      // ******************************* PAGE 221: KPA Rig select *************************************************
      {PAGE_VCTOUCH_KPA_RIG_SELECT, LABEL, 'K', 'P', 'A', ' ', 'R', 'I', 'G', ' ' },
      {PAGE_VCTOUCH_KPA_RIG_SELECT, LABEL, 'S', 'E', 'L', 'E', 'C', 'T', ' ', ' ' },
      {PAGE_VCTOUCH_KPA_RIG_SELECT, 1, SNAPSCENE, CURRENT, 1, 6, 0}, // ** Switch 01 **
      {PAGE_VCTOUCH_KPA_RIG_SELECT, 2, SNAPSCENE, CURRENT, 2, 7, 0}, // ** Switch 02 **
      {PAGE_VCTOUCH_KPA_RIG_SELECT, 3, SNAPSCENE, CURRENT, 3, 8, 0}, // ** Switch 03 **
      {PAGE_VCTOUCH_KPA_RIG_SELECT, 4, SNAPSCENE, CURRENT, 4, 9, 0}, // ** Switch 04 **
      {PAGE_VCTOUCH_KPA_RIG_SELECT, 5, SNAPSCENE, CURRENT, 5, 10, 0}, // ** Switch 08 **
      {PAGE_VCTOUCH_KPA_RIG_SELECT, 6, PATCH, KPA, BANKSELECT, 1, 5}, // ** Switch 05 **
      {PAGE_VCTOUCH_KPA_RIG_SELECT, 7, PATCH, KPA, BANKSELECT, 2, 5}, // ** Switch 06 **
      {PAGE_VCTOUCH_KPA_RIG_SELECT, 8, PATCH, KPA, BANKSELECT, 3, 5}, // ** Switch 07 **
      {PAGE_VCTOUCH_KPA_RIG_SELECT, 9, PATCH, KPA, BANKSELECT, 4, 5}, // ** Switch 09 **
      {PAGE_VCTOUCH_KPA_RIG_SELECT, 10, PATCH, KPA, BANKSELECT, 5, 5}, // ** Switch 10 **
      {PAGE_VCTOUCH_KPA_RIG_SELECT, 11, PATCH, KPA, BANKDOWN, 5}, // ** Switch 11 **
      {PAGE_VCTOUCH_KPA_RIG_SELECT, 11 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 11 **
      {PAGE_VCTOUCH_KPA_RIG_SELECT, 12, PATCH, KPA, BANKUP, 5}, // ** Switch 12 **
      {PAGE_VCTOUCH_KPA_RIG_SELECT, 12 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 12 **
      //{PAGE_VCTOUCH_KPA_RIG_SELECT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      //{PAGE_VCTOUCH_KPA_RIG_SELECT, 15, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 15 **

      // ******************************* PAGE 222: KPA FX control *************************************************
      {PAGE_VCTOUCH_KPA_FX_CONTROL, LABEL, 'K', 'P', 'A', ' ', 'F', 'X', ' ', 'C' },
      {PAGE_VCTOUCH_KPA_FX_CONTROL, LABEL, 'O', 'N', 'T', 'R', 'O', 'L', ' ', ' ' },
      {PAGE_VCTOUCH_KPA_FX_CONTROL, 1, PARAMETER, KPA, 0, TOGGLE, 1, 0}, // ** Switch 01 **
      {PAGE_VCTOUCH_KPA_FX_CONTROL, 2, PARAMETER, KPA, 1, TOGGLE, 1, 0}, // ** Switch 02 **
      {PAGE_VCTOUCH_KPA_FX_CONTROL, 3, PARAMETER, KPA, 2, TOGGLE, 1, 0}, // ** Switch 03 **
      {PAGE_VCTOUCH_KPA_FX_CONTROL, 4, PARAMETER, KPA, 3, TOGGLE, 1, 0}, // ** Switch 04 **
      {PAGE_VCTOUCH_KPA_FX_CONTROL, 5, LOOPER, CURRENT, LOOPER_REC_PLAY_OVERDUB}, // ** Switch 08 **
      {PAGE_VCTOUCH_KPA_FX_CONTROL, 6, PARAMETER, KPA, 4, TOGGLE, 1, 0}, // ** Switch 05 **
      {PAGE_VCTOUCH_KPA_FX_CONTROL, 7, PARAMETER, KPA, 5, TOGGLE, 1, 0}, // ** Switch 06 **
      {PAGE_VCTOUCH_KPA_FX_CONTROL, 8, PARAMETER, KPA, 6, TOGGLE, 1, 0}, // ** Switch 07 **
      {PAGE_VCTOUCH_KPA_FX_CONTROL, 9, PARAMETER, KPA, 7, TOGGLE, 1, 0}, // ** Switch 09 **
      {PAGE_VCTOUCH_KPA_FX_CONTROL, 10 | ON_RELEASE, LOOPER, CURRENT, LOOPER_STOP_ERASE}, // ** Switch 10 **
      {PAGE_VCTOUCH_KPA_FX_CONTROL, 10 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_VCTOUCH_KPA_LOOPER},
      {PAGE_VCTOUCH_KPA_FX_CONTROL, 11, PARAMETER, KPA, 18, ONE_SHOT, 0, 0}, // ** Switch 11 **
      {PAGE_VCTOUCH_KPA_FX_CONTROL, 12, PARAMETER, KPA, 17, ONE_SHOT, 0, 0}, // ** Switch 12 **
      //{PAGE_VCTOUCH_KPA_FX_CONTROL, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      //{PAGE_VCTOUCH_KPA_FX_CONTROL, 15, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 15 **

      // ******************************* PAGE 223: KPA Looper control *************************************************
      {PAGE_VCTOUCH_KPA_LOOPER, LABEL, 'K', 'P', 'A', ' ', 'L', 'O', 'O', 'P' },
      {PAGE_VCTOUCH_KPA_LOOPER, LABEL, 'E', 'R', ' ', 'C', 'T', 'L', ' ', ' ' },
      {PAGE_VCTOUCH_KPA_LOOPER, 1, LOOPER, CURRENT, LOOPER_REC_PLAY_OVERDUB}, // ** Switch 01 **
      {PAGE_VCTOUCH_KPA_LOOPER, 2, LOOPER, CURRENT, LOOPER_STOP_ERASE}, // ** Switch 02 **
      {PAGE_VCTOUCH_KPA_LOOPER, 3, LOOPER, CURRENT, LOOPER_UNDO_REDO}, // ** Switch 03 **
      {PAGE_VCTOUCH_KPA_LOOPER, 4, PARAMETER, KPA, 0, TOGGLE, 1, 0}, // ** Switch 04 **
      {PAGE_VCTOUCH_KPA_LOOPER, 5, PARAMETER, KPA, 1, TOGGLE, 1, 0}, // ** Switch 05 **
      {PAGE_VCTOUCH_KPA_LOOPER, 6, LOOPER, CURRENT, LOOPER_PLAY_ONCE}, // ** Switch 06 **
      {PAGE_VCTOUCH_KPA_LOOPER, 7, LOOPER, CURRENT, LOOPER_HALF_SPEED}, // ** Switch 07 **
      {PAGE_VCTOUCH_KPA_LOOPER, 8, LOOPER, CURRENT, LOOPER_REVERSE}, // ** Switch 08 **
      {PAGE_VCTOUCH_KPA_LOOPER, 9, PARAMETER, KPA, 19, TOGGLE, 1, 0}, // ** Switch 09 **
      {PAGE_VCTOUCH_KPA_LOOPER, 10, PARAMETER, KPA, 5, TOGGLE, 1, 0}, // ** Switch 10 **
      {PAGE_VCTOUCH_KPA_LOOPER, 11, PATCH, KPA, PREV}, // ** Switch 13 **
      {PAGE_VCTOUCH_KPA_LOOPER, 12, PATCH, KPA, NEXT}, // ** Switch 14 **
      //{PAGE_VCTOUCH_KPA_LOOPER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
      //{PAGE_VCTOUCH_KPA_LOOPER, 15, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 15 **

      // ******************************* PAGE 224: Parameters current device *************************************************
      {PAGE_VCTOUCH_CURRENT_ASSIGN, LABEL, 'A', 'S', 'G', ' ', 'B', 'A', 'N', 'K' },
      {PAGE_VCTOUCH_CURRENT_ASSIGN, 1, ASSIGN, CURRENT, BANKSELECT, 1, 10}, // ** Switch 01 **
      {PAGE_VCTOUCH_CURRENT_ASSIGN, 2, ASSIGN, CURRENT, BANKSELECT, 2, 10}, // ** Switch 02 **
      {PAGE_VCTOUCH_CURRENT_ASSIGN, 3, ASSIGN, CURRENT, BANKSELECT, 3, 10}, // ** Switch 03 **
      {PAGE_VCTOUCH_CURRENT_ASSIGN, 4, ASSIGN, CURRENT, BANKSELECT, 4, 10}, // ** Switch 04 **
      {PAGE_VCTOUCH_CURRENT_ASSIGN, 5, ASSIGN, CURRENT, BANKSELECT, 5, 10}, // ** Switch 05 **
      {PAGE_VCTOUCH_CURRENT_ASSIGN, 6, ASSIGN, CURRENT, BANKSELECT, 6, 10}, // ** Switch 06 **
      {PAGE_VCTOUCH_CURRENT_ASSIGN, 7, ASSIGN, CURRENT, BANKSELECT, 7, 10}, // ** Switch 07 **
      {PAGE_VCTOUCH_CURRENT_ASSIGN, 8, ASSIGN, CURRENT, BANKSELECT, 8, 10}, // ** Switch 08 **
      {PAGE_VCTOUCH_CURRENT_ASSIGN, 9, ASSIGN, CURRENT, BANKSELECT, 9, 10}, // ** Switch 09 **
      {PAGE_VCTOUCH_CURRENT_ASSIGN, 10, ASSIGN, CURRENT, BANKSELECT, 10, 10}, // ** Switch 10 **
      {PAGE_VCTOUCH_CURRENT_ASSIGN, 11, ASSIGN, CURRENT, BANKDOWN, 10}, // ** Switch 11 **
      {PAGE_VCTOUCH_CURRENT_ASSIGN, 12, ASSIGN, CURRENT, BANKUP, 10}, // ** Switch 12 **
      //{PAGE_VCTOUCH_CURRENT_ASSIGN, 15, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 15 **

      // ******************************* PAGE 225: SY1000_patch_bank (8 buttons per page) *************************************************
      {PAGE_VCTOUCH_SY1000_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'N' },
      {PAGE_VCTOUCH_SY1000_PATCH_BANK, LABEL, 'K', ' ', 'S', 'Y', '1', '0', '0', '0' },
      {PAGE_VCTOUCH_SY1000_PATCH_BANK, 1, PATCH, SY1000, BANKSELECT, 1, 8}, // ** Switch 01 **
      {PAGE_VCTOUCH_SY1000_PATCH_BANK, 2, PATCH, SY1000, BANKSELECT, 2, 8}, // ** Switch 02 **
      {PAGE_VCTOUCH_SY1000_PATCH_BANK, 3, PATCH, SY1000, BANKSELECT, 3, 8}, // ** Switch 03 **
      {PAGE_VCTOUCH_SY1000_PATCH_BANK, 4, PATCH, SY1000, BANKSELECT, 4, 8}, // ** Switch 04 **
      {PAGE_VCTOUCH_SY1000_PATCH_BANK, 5, ASSIGN, SY1000, SELECT, 1, 1}, // ** Switch 05 **
      {PAGE_VCTOUCH_SY1000_PATCH_BANK, 6, PATCH, SY1000, BANKSELECT, 5, 8}, // ** Switch 06 **
      {PAGE_VCTOUCH_SY1000_PATCH_BANK, 7, PATCH, SY1000, BANKSELECT, 6, 8}, // ** Switch 07 **
      {PAGE_VCTOUCH_SY1000_PATCH_BANK, 8, PATCH, SY1000, BANKSELECT, 7, 8}, // ** Switch 08 **
      {PAGE_VCTOUCH_SY1000_PATCH_BANK, 9, PATCH, SY1000, BANKSELECT, 8, 8}, // ** Switch 09 **
      {PAGE_VCTOUCH_SY1000_PATCH_BANK, 10, ASSIGN, SY1000, SELECT, 0, 0}, // ** Switch 10 **
      {PAGE_VCTOUCH_SY1000_PATCH_BANK, 11, PATCH, SY1000, BANKDOWN, 8}, // ** Switch 11 **
      {PAGE_VCTOUCH_SY1000_PATCH_BANK, 11 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 11 **
      {PAGE_VCTOUCH_SY1000_PATCH_BANK, 12, PATCH, SY1000, BANKUP, 8}, // ** Switch 12 **
      {PAGE_VCTOUCH_SY1000_PATCH_BANK, 12 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 12 **
      //{PAGE_VCTOUCH_SY1000_PATCH_BANK, 15, OPEN_NEXT_PAGE_OF_DEVICE, SY1000}, // ** Switch 15 **

      // ******************************* PAGE 226: SY1000 assign *************************************************
      {PAGE_VCTOUCH_SY1000_ASSIGNS, LABEL, 'A', 'S', 'S', 'I', 'G', 'N', 'S', ' ' },
      {PAGE_VCTOUCH_SY1000_ASSIGNS, LABEL, 'S', 'Y', '1', '0', '0', '0', ' ', ' ' },
      {PAGE_VCTOUCH_SY1000_ASSIGNS, 1, ASSIGN, SY1000, BANKSELECT, 1, 8}, // ** Switch 01 **
      {PAGE_VCTOUCH_SY1000_ASSIGNS, 2, ASSIGN, SY1000, BANKSELECT, 2, 8}, // ** Switch 02 **
      {PAGE_VCTOUCH_SY1000_ASSIGNS, 3, ASSIGN, SY1000, BANKSELECT, 3, 8}, // ** Switch 03 **
      {PAGE_VCTOUCH_SY1000_ASSIGNS, 4, ASSIGN, SY1000, BANKSELECT, 4, 8}, // ** Switch 04 **
      {PAGE_VCTOUCH_SY1000_ASSIGNS, 5, ASSIGN, SY1000, BANKDOWN, 8}, // ** Switch 05 **
      {PAGE_VCTOUCH_SY1000_ASSIGNS, 6, ASSIGN, SY1000, BANKSELECT, 5, 8}, // ** Switch 06 **
      {PAGE_VCTOUCH_SY1000_ASSIGNS, 7, ASSIGN, SY1000, BANKSELECT, 6, 8}, // ** Switch 08 **
      {PAGE_VCTOUCH_SY1000_ASSIGNS, 8, ASSIGN, SY1000, BANKSELECT, 7, 8}, // ** Switch 09 **
      {PAGE_VCTOUCH_SY1000_ASSIGNS, 9, ASSIGN, SY1000, BANKSELECT, 8, 8}, // ** Switch 00 **
      {PAGE_VCTOUCH_SY1000_ASSIGNS, 10, ASSIGN, SY1000, BANKUP, 8}, // ** Switch 10 **
      {PAGE_VCTOUCH_SY1000_ASSIGNS, 11, PATCH, SY1000, PREV, 1}, // ** Switch 11 **
      {PAGE_VCTOUCH_SY1000_ASSIGNS, 12, PATCH, SY1000, NEXT, 1}, // ** Switch 12 **
      //{PAGE_VCTOUCH_SY1000_ASSIGNS, 15, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 15 **

      // ******************************* PAGE 227: Scenes (SY1000) *************************************************
      {PAGE_VCTOUCH_SY1000_SCENES, LABEL, 'S', 'C', 'E', 'N', 'E', 'S', ' ', 'G' },
      {PAGE_VCTOUCH_SY1000_SCENES, LABEL, 'R', '/', 'S', 'Y', ' ', ' ', ' ', ' ' },
      {PAGE_VCTOUCH_SY1000_SCENES, 1, SNAPSCENE, CURRENT, 1, 0, 0}, // ** Switch 01 **
      {PAGE_VCTOUCH_SY1000_SCENES, 2, SNAPSCENE, CURRENT, 2, 0, 0}, // ** Switch 02 **
      {PAGE_VCTOUCH_SY1000_SCENES, 3, SNAPSCENE, CURRENT, 3, 0, 0}, // ** Switch 03 **
      {PAGE_VCTOUCH_SY1000_SCENES, 4, SNAPSCENE, CURRENT, 4, 0, 0}, // ** Switch 04 **
      {PAGE_VCTOUCH_SY1000_SCENES, 5, ASSIGN, CURRENT, SELECT, 0}, // ** Switch 05 **
      {PAGE_VCTOUCH_SY1000_SCENES, 6, SNAPSCENE, CURRENT, 5, 0, 0}, // ** Switch 06 **
      {PAGE_VCTOUCH_SY1000_SCENES, 7, SNAPSCENE, CURRENT, 6, 0, 0}, // ** Switch 07 **
      {PAGE_VCTOUCH_SY1000_SCENES, 8, SNAPSCENE, CURRENT, 7, 0, 0}, // ** Switch 08 **
      {PAGE_VCTOUCH_SY1000_SCENES, 9, SNAPSCENE, CURRENT, 8, 0, 0}, // ** Switch 09 **
      {PAGE_VCTOUCH_SY1000_SCENES, 10, SAVE_PATCH, CURRENT}, // ** Switch 10 **
      {PAGE_VCTOUCH_SY1000_SCENES, 11, PATCH, CURRENT, PREV}, // ** Switch 11 **
      {PAGE_VCTOUCH_SY1000_SCENES, 12, PATCH, CURRENT, NEXT}, // ** Switch 12 **
      //{PAGE_VCTOUCH_SY1000_SCENES, 15, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 15 **

      // ******************************* PAGE 228: MG300_patch_bank (8 buttons per page) *************************************************
      {PAGE_VCTOUCH_MG300_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
      {PAGE_VCTOUCH_MG300_PATCH_BANK, LABEL, 'N', 'K', ' ', 'M', 'G', '3', '0', '0' },
      {PAGE_VCTOUCH_MG300_PATCH_BANK, 1, PATCH, MG300, BANKSELECT, 1, 8}, // ** Switch 01 **
      {PAGE_VCTOUCH_MG300_PATCH_BANK, 2, PATCH, MG300, BANKSELECT, 2, 8}, // ** Switch 02 **
      {PAGE_VCTOUCH_MG300_PATCH_BANK, 3, PATCH, MG300, BANKSELECT, 3, 8}, // ** Switch 03 **
      {PAGE_VCTOUCH_MG300_PATCH_BANK, 4, PATCH, MG300, BANKSELECT, 4, 8}, // ** Switch 04 **
      {PAGE_VCTOUCH_MG300_PATCH_BANK, 6, PATCH, MG300, BANKSELECT, 5, 8}, // ** Switch 06 **
      {PAGE_VCTOUCH_MG300_PATCH_BANK, 7, PATCH, MG300, BANKSELECT, 6, 8}, // ** Switch 07 **
      {PAGE_VCTOUCH_MG300_PATCH_BANK, 8, PATCH, MG300, BANKSELECT, 7, 8}, // ** Switch 08 **
      {PAGE_VCTOUCH_MG300_PATCH_BANK, 9, PATCH, MG300, BANKSELECT, 8, 8}, // ** Switch 09 **
      {PAGE_VCTOUCH_MG300_PATCH_BANK, 11, PATCH, MG300, BANKDOWN, 8}, // ** Switch 11 **
      {PAGE_VCTOUCH_MG300_PATCH_BANK, 11 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 11 **
      {PAGE_VCTOUCH_MG300_PATCH_BANK, 12, PATCH, MG300, BANKUP, 8}, // ** Switch 12 **
      {PAGE_VCTOUCH_MG300_PATCH_BANK, 12 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 12 **
      //{PAGE_VCTOUCH_MG300_PATCH_BANK, 15, OPEN_NEXT_PAGE_OF_DEVICE, MG300}, // ** Switch 15 **

      // ******************************* PAGE 229: MIDI PC BANK SELECT *************************************************
      {PAGE_VCTOUCH_MIDI_PC_BANK_SELECT, LABEL, 'S', 'E', 'L', 'E', 'C', 'T', ' ', 'M' },
      {PAGE_VCTOUCH_MIDI_PC_BANK_SELECT, LABEL, 'I', 'D', 'I', ' ', 'P', 'C', ' ', ' ' },
      {PAGE_VCTOUCH_MIDI_PC_BANK_SELECT, 1, MIDI_PC, COMMON, BANKSELECT, 1, 10, 1, PORT3_NUMBER}, // ** Switch 01 **
      {PAGE_VCTOUCH_MIDI_PC_BANK_SELECT, 2, MIDI_PC, COMMON, BANKSELECT, 2, 10, 1, PORT3_NUMBER}, // ** Switch 02 **
      {PAGE_VCTOUCH_MIDI_PC_BANK_SELECT, 3, MIDI_PC, COMMON, BANKSELECT, 3, 10, 1, PORT3_NUMBER}, // ** Switch 03 **
      {PAGE_VCTOUCH_MIDI_PC_BANK_SELECT, 4, MIDI_PC, COMMON, BANKSELECT, 4, 10, 1, PORT3_NUMBER}, // ** Switch 04 **
      {PAGE_VCTOUCH_MIDI_PC_BANK_SELECT, 5, MIDI_PC, COMMON, BANKSELECT, 5, 10, 1, PORT3_NUMBER}, // ** Switch 05 **
      {PAGE_VCTOUCH_MIDI_PC_BANK_SELECT, 6, MIDI_PC, COMMON, BANKSELECT, 6, 10, 1, PORT3_NUMBER}, // ** Switch 06 **
      {PAGE_VCTOUCH_MIDI_PC_BANK_SELECT, 7, MIDI_PC, COMMON, BANKSELECT, 7, 10, 1, PORT3_NUMBER}, // ** Switch 07 **
      {PAGE_VCTOUCH_MIDI_PC_BANK_SELECT, 8, MIDI_PC, COMMON, BANKSELECT, 8, 10, 1, PORT3_NUMBER}, // ** Switch 08 **
      {PAGE_VCTOUCH_MIDI_PC_BANK_SELECT, 9, MIDI_PC, COMMON, BANKSELECT, 9, 10, 1, PORT3_NUMBER}, // ** Switch 09 **
      {PAGE_VCTOUCH_MIDI_PC_BANK_SELECT, 10, MIDI_PC, COMMON, BANKSELECT, 10, 10, 1, PORT3_NUMBER}, // ** Switch 10 **
      {PAGE_VCTOUCH_MIDI_PC_BANK_SELECT, 11, MIDI_PC, COMMON, BANKDOWN, 10, 1, PORT3_NUMBER}, // ** Switch 11 **
      {PAGE_VCTOUCH_MIDI_PC_BANK_SELECT, 12, MIDI_PC, COMMON, BANKUP, 10, 1, PORT3_NUMBER}, // ** Switch 12 **

      // ******************************* PAGE 230: SONG SELECT *************************************************
      {PAGE_VCTOUCH_SONG_MODE, LABEL, 'S', 'O', 'N', 'G', ' ', 'M', 'O', 'D' },
      {PAGE_VCTOUCH_SONG_MODE, LABEL, 'E', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VCTOUCH_SONG_MODE, 1, SONG, COMMON, SONG_PREV, SONG_PREVNEXT_SONG}, // ** Switch 01 **
      {PAGE_VCTOUCH_SONG_MODE, 1 | ON_LONG_PRESS, PAGE, COMMON, SELECT, PAGE_VCTOUCH_SONG_SELECT},
      {PAGE_VCTOUCH_SONG_MODE, 2, SONG, COMMON, SONG_PARTSEL, 0}, // ** Switch 02 **
      {PAGE_VCTOUCH_SONG_MODE, 3, SONG, COMMON, SONG_PARTSEL, 1}, // ** Switch 03 **
      {PAGE_VCTOUCH_SONG_MODE, 4, SONG, COMMON, SONG_PARTSEL, 2}, // ** Switch 04 **
      {PAGE_VCTOUCH_SONG_MODE, 5, SONG, COMMON, SONG_PARTSEL, 3}, // ** Switch 05 **
      {PAGE_VCTOUCH_SONG_MODE, 6, SONG, COMMON, SONG_NEXT, SONG_PREVNEXT_SONG}, // ** Switch 06 **
      {PAGE_VCTOUCH_SONG_MODE, 6 | ON_LONG_PRESS, PAGE, COMMON, SELECT, PAGE_VCTOUCH_SONG_SELECT},
      {PAGE_VCTOUCH_SONG_MODE, 7, SONG, COMMON, SONG_PARTSEL, 4}, // ** Switch 07 **
      {PAGE_VCTOUCH_SONG_MODE, 8, SONG, COMMON, SONG_PARTSEL, 5}, // ** Switch 08 **
      {PAGE_VCTOUCH_SONG_MODE, 9, SONG, COMMON, SONG_PARTSEL, 6}, // ** Switch 09 **
      {PAGE_VCTOUCH_SONG_MODE, 10, SONG, COMMON, SONG_PARTSEL, 7}, // ** Switch 10 **
      {PAGE_VCTOUCH_SONG_MODE, 11, MODE, COMMON, SELECT, SONG_MODE}, // ** Switch 11 **
      {PAGE_VCTOUCH_SONG_MODE, 12, MODE, COMMON, SELECT, PAGE_MODE}, // ** Switch 12 **
      {PAGE_VCTOUCH_SONG_MODE, 13, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 13 **
      {PAGE_VCTOUCH_SONG_MODE, 14, PAGE, COMMON, SELECT, PAGE_VCTOUCH_SETLIST_SELECT}, // ** Switch 14 **

      // ******************************* PAGE 231: SONG SELECT *************************************************
      {PAGE_VCTOUCH_SONG_SELECT, LABEL, 'S', 'O', 'N', 'G', ' ', 'S', 'E', 'L' },
      {PAGE_VCTOUCH_SONG_SELECT, LABEL, 'E', 'C', 'T', ' ', ' ', ' ', ' ', ' ' },
      {PAGE_VCTOUCH_SONG_SELECT, 1, SONG, COMMON, SONG_BANKDOWN, 8}, // ** Switch 01 **
      {PAGE_VCTOUCH_SONG_SELECT, 2, SONG, COMMON, SONG_BANKSELECT, 0, 8}, // ** Switch 02 **
      {PAGE_VCTOUCH_SONG_SELECT, 2, PAGE, COMMON, SELECT, 0},
      {PAGE_VCTOUCH_SONG_SELECT, 3, SONG, COMMON, SONG_BANKSELECT, 1, 8}, // ** Switch 03 **
      {PAGE_VCTOUCH_SONG_SELECT, 3, PAGE, COMMON, SELECT, 0},
      {PAGE_VCTOUCH_SONG_SELECT, 4, SONG, COMMON, SONG_BANKSELECT, 2, 8}, // ** Switch 04 **
      {PAGE_VCTOUCH_SONG_SELECT, 4, PAGE, COMMON, SELECT, 0},
      {PAGE_VCTOUCH_SONG_SELECT, 5, SONG, COMMON, SONG_BANKSELECT, 3, 8}, // ** Switch 05 **
      {PAGE_VCTOUCH_SONG_SELECT, 5, PAGE, COMMON, SELECT, 0},
      {PAGE_VCTOUCH_SONG_SELECT, 6, SONG, COMMON, SONG_BANKUP, 8}, // ** Switch 06 **
      {PAGE_VCTOUCH_SONG_SELECT, 7, SONG, COMMON, SONG_BANKSELECT, 4, 8}, // ** Switch 07 **
      {PAGE_VCTOUCH_SONG_SELECT, 7, PAGE, COMMON, SELECT, 0},
      {PAGE_VCTOUCH_SONG_SELECT, 8, SONG, COMMON, SONG_BANKSELECT, 5, 8}, // ** Switch 08 **
      {PAGE_VCTOUCH_SONG_SELECT, 8, PAGE, COMMON, SELECT, 0},
      {PAGE_VCTOUCH_SONG_SELECT, 9, SONG, COMMON, SONG_BANKSELECT, 6, 8}, // ** Switch 09 **
      {PAGE_VCTOUCH_SONG_SELECT, 9, PAGE, COMMON, SELECT, 0},
      {PAGE_VCTOUCH_SONG_SELECT, 10, SONG, COMMON, SONG_BANKSELECT, 7, 8}, // ** Switch 10 **
      {PAGE_VCTOUCH_SONG_SELECT, 10, PAGE, COMMON, SELECT, 0},
      {PAGE_VCTOUCH_SONG_SELECT, 11, MODE, COMMON, SELECT, SONG_MODE}, // ** Switch 11 **
      {PAGE_VCTOUCH_SONG_SELECT, 12, MODE, COMMON, SELECT, PAGE_MODE}, // ** Switch 12 **
      {PAGE_VCTOUCH_SONG_SELECT, 13, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 13 **
      {PAGE_VCTOUCH_SONG_SELECT, 14, SONG, COMMON, SONG_EDIT}, // ** Switch 14 **
      {PAGE_VCTOUCH_SONG_SELECT, 15, PAGE, COMMON, SELECT, 0}, // ** Switch 15 **
      {PAGE_VCTOUCH_SONG_SELECT, 15 | LABEL, 'E', 'X', 'I', 'T', ' ', ' ', ' ', ' ' },

      // ******************************* PAGE 232: SETLIST SELECT *************************************************
      {PAGE_VCTOUCH_SETLIST_SELECT, LABEL, 'S', 'E', 'T', 'L', 'I', 'S', 'T', ' ' },
      {PAGE_VCTOUCH_SETLIST_SELECT, LABEL, 'S', 'E', 'L', 'E', 'C', 'T', ' ', ' ' },
      {PAGE_VCTOUCH_SETLIST_SELECT, 1, SETLIST, COMMON, SL_BANKDOWN, 8}, // ** Switch 01 **
      {PAGE_VCTOUCH_SETLIST_SELECT, 2, SETLIST, COMMON, SL_BANKSELECT, 0, 8}, // ** Switch 02 **
      {PAGE_VCTOUCH_SETLIST_SELECT, 3, SETLIST, COMMON, SL_BANKSELECT, 1, 8}, // ** Switch 03 **
      {PAGE_VCTOUCH_SETLIST_SELECT, 4, SETLIST, COMMON, SL_BANKSELECT, 2, 8}, // ** Switch 04 **
      {PAGE_VCTOUCH_SETLIST_SELECT, 5, SETLIST, COMMON, SL_BANKSELECT, 3, 8}, // ** Switch 05 **
      {PAGE_VCTOUCH_SETLIST_SELECT, 6, SETLIST, COMMON, SL_BANKUP, 8}, // ** Switch 06 **
      {PAGE_VCTOUCH_SETLIST_SELECT, 7, SETLIST, COMMON, SL_BANKSELECT, 4, 8}, // ** Switch 07 **
      {PAGE_VCTOUCH_SETLIST_SELECT, 8, SETLIST, COMMON, SL_BANKSELECT, 5, 8}, // ** Switch 08 **
      {PAGE_VCTOUCH_SETLIST_SELECT, 9, SETLIST, COMMON, SL_BANKSELECT, 6, 8}, // ** Switch 09 **
      {PAGE_VCTOUCH_SETLIST_SELECT, 10, SETLIST, COMMON, SL_BANKSELECT, 7, 8}, // ** Switch 10 **
      {PAGE_VCTOUCH_SETLIST_SELECT, 11, MODE, COMMON, SELECT, SONG_MODE}, // ** Switch 11 **
      {PAGE_VCTOUCH_SETLIST_SELECT, 12, MODE, COMMON, SELECT, PAGE_MODE}, // ** Switch 12 **
      {PAGE_VCTOUCH_SETLIST_SELECT, 13, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 13 **
      {PAGE_VCTOUCH_SETLIST_SELECT, 14, SETLIST, COMMON, SL_EDIT}, // ** Switch 14 **
      {PAGE_VCTOUCH_SETLIST_SELECT, 15, PAGE, COMMON, SELECT, 0}, // ** Switch 15 **
      {PAGE_VCTOUCH_SETLIST_SELECT, 15 | LABEL, 'E', 'X', 'I', 'T', ' ', ' ', ' ', ' ' },

    // ******************************* PAGE 233: User_patch_bank (8 buttons per page) *************************************************
    {PAGE_VCTOUCH_USER_PATCH_BANK, LABEL,  'U', 'S', 'E', 'R', ' ', 'P', 'A', 'T' },
    {PAGE_VCTOUCH_USER_PATCH_BANK, LABEL, 'C', 'H', ' ', 'S', 'E', 'L', 'C', 'T' },
    {PAGE_VCTOUCH_USER_PATCH_BANK, 1, PATCH, CURRENT, BANKSELECT, 1, 8}, // ** Switch 01 **
    {PAGE_VCTOUCH_USER_PATCH_BANK, 2, PATCH, CURRENT, BANKSELECT, 2, 8}, // ** Switch 02 **
    {PAGE_VCTOUCH_USER_PATCH_BANK, 3, PATCH, CURRENT, BANKSELECT, 3, 8}, // ** Switch 03 **
    {PAGE_VCTOUCH_USER_PATCH_BANK, 4, PATCH, CURRENT, BANKSELECT, 4, 8}, // ** Switch 04 **
    {PAGE_VCTOUCH_USER_PATCH_BANK, 5, TOGGLE_EXP_PEDAL, CURRENT}, // ** Switch 05 **
    {PAGE_VCTOUCH_USER_PATCH_BANK, 6, PATCH, CURRENT, BANKSELECT, 5, 8}, // ** Switch 06 **
    {PAGE_VCTOUCH_USER_PATCH_BANK, 7, PATCH, CURRENT, BANKSELECT, 6, 8}, // ** Switch 07 **
    {PAGE_VCTOUCH_USER_PATCH_BANK, 8, PATCH, CURRENT, BANKSELECT, 7, 8}, // ** Switch 08 **
    {PAGE_VCTOUCH_USER_PATCH_BANK, 9, PATCH, CURRENT, BANKSELECT, 8, 8}, // ** Switch 09 **
    {PAGE_VCTOUCH_USER_PATCH_BANK, 10, SAVE_PATCH, CURRENT}, // ** Switch 10 **
    {PAGE_VCTOUCH_USER_PATCH_BANK, 11, PATCH, CURRENT, BANKDOWN, 8}, // ** Switch 11 **
    {PAGE_VCTOUCH_USER_PATCH_BANK, 11 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 11 **
    {PAGE_VCTOUCH_USER_PATCH_BANK, 12, PATCH, CURRENT, BANKUP, 8}, // ** Switch 12 **
    {PAGE_VCTOUCH_USER_PATCH_BANK, 12 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 12 **
    //{PAGE_VCTOUCH_USER_PATCH_BANK, 15, OPEN_NEXT_PAGE_OF_DEVICE, MG300}, // ** Switch 15 **

    // ******************************* PAGE 233: User_parameter_bank (8 buttons per page) *************************************************
    {PAGE_VCTOUCH_USER_PAR_BANK, LABEL, 'U', 'S', 'E', 'R', ' ', 'F', 'X', ' ' },
    {PAGE_VCTOUCH_USER_PAR_BANK, LABEL, 'S', 'E', 'L', 'E', 'C', 'T', ' ', ' ' },
    {PAGE_VCTOUCH_USER_PAR_BANK, 1, PAR_BANK, CURRENT, 1, 8}, // ** Switch 01 **
    {PAGE_VCTOUCH_USER_PAR_BANK, 2, PAR_BANK, CURRENT, 2, 8}, // ** Switch 02 **
    {PAGE_VCTOUCH_USER_PAR_BANK, 3, PAR_BANK, CURRENT, 3, 8}, // ** Switch 03 **
    {PAGE_VCTOUCH_USER_PAR_BANK, 4, PAR_BANK, CURRENT, 4, 8}, // ** Switch 04 **
    {PAGE_VCTOUCH_USER_PAR_BANK, 5, TOGGLE_EXP_PEDAL, CURRENT}, // ** Switch 05 **
    {PAGE_VCTOUCH_USER_PAR_BANK, 6, PAR_BANK, CURRENT, 5, 8}, // ** Switch 06 **
    {PAGE_VCTOUCH_USER_PAR_BANK, 7, PAR_BANK, CURRENT, 6, 8}, // ** Switch 07 **
    {PAGE_VCTOUCH_USER_PAR_BANK, 8, PAR_BANK, CURRENT, 7, 8}, // ** Switch 08 **
    {PAGE_VCTOUCH_USER_PAR_BANK, 9, PAR_BANK, CURRENT, 8, 8}, // ** Switch 09 **
    {PAGE_VCTOUCH_USER_PAR_BANK, 10, SAVE_PATCH, CURRENT}, // ** Switch 10 **
    {PAGE_VCTOUCH_USER_PAR_BANK, 11, PATCH, CURRENT, PREV}, // ** Switch 11 **
    {PAGE_VCTOUCH_USER_PAR_BANK, 11 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 11 **
    {PAGE_VCTOUCH_USER_PAR_BANK, 12, PATCH, CURRENT, NEXT}, // ** Switch 12 **
    {PAGE_VCTOUCH_USER_PAR_BANK, 12 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 12 **
    //{PAGE_VCTOUCH_USER_PAR_BANK, 15, OPEN_NEXT_PAGE_OF_DEVICE, MG300}, // ** Switch 15 **

    // ******************************* PAGE 234: User_parameter_bank (8 buttons per page) *************************************************
    {PAGE_VCTOUCH_USER_SCENES, LABEL, 'U', 'S', 'E', 'R', ' ', 'S', 'C', 'E' },
    {PAGE_VCTOUCH_USER_SCENES, LABEL, 'N', 'E', ' ', 'S', 'E', 'L', 'C', 'T' },
    {PAGE_VCTOUCH_USER_SCENES, 1, SNAPSCENE, CURRENT, 1, 0, 0}, // ** Switch 01 **
    {PAGE_VCTOUCH_USER_SCENES, 2, SNAPSCENE, CURRENT, 2, 0, 0}, // ** Switch 02 **
    {PAGE_VCTOUCH_USER_SCENES, 3, SNAPSCENE, CURRENT, 3, 0, 0}, // ** Switch 03 **
    {PAGE_VCTOUCH_USER_SCENES, 4, SNAPSCENE, CURRENT, 4, 0, 0}, // ** Switch 04 **
    {PAGE_VCTOUCH_USER_SCENES, 5, TOGGLE_EXP_PEDAL, CURRENT}, // ** Switch 05 **
    {PAGE_VCTOUCH_USER_SCENES, 6, SNAPSCENE, CURRENT, 5, 0, 0}, // ** Switch 06 **
    {PAGE_VCTOUCH_USER_SCENES, 7, SNAPSCENE, CURRENT, 6, 0, 0}, // ** Switch 07 **
    {PAGE_VCTOUCH_USER_SCENES, 8, SNAPSCENE, CURRENT, 7, 0, 0}, // ** Switch 08 **
    {PAGE_VCTOUCH_USER_SCENES, 9, SNAPSCENE, CURRENT, 8, 0, 0}, // ** Switch 09 **
    {PAGE_VCTOUCH_USER_SCENES, 10, SAVE_PATCH, CURRENT}, // ** Switch 10 **
    {PAGE_VCTOUCH_USER_SCENES, 11, PATCH, CURRENT, PREV}, // ** Switch 11 **
    {PAGE_VCTOUCH_USER_SCENES, 11 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 11 **
    {PAGE_VCTOUCH_USER_SCENES, 12, PATCH, CURRENT, NEXT}, // ** Switch 12 **
    {PAGE_VCTOUCH_USER_SCENES, 12 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 12 **
    //{PAGE_VCTOUCH_USER_SCENES, 15, OPEN_NEXT_PAGE_OF_DEVICE, MG300}, // ** Switch 15 **
  };

  const uint16_t NUMBER_OF_INTERNAL_COMMANDS = sizeof(Fixed_commands) / sizeof(Fixed_commands[0]);

  // ********************************* Section 7: VC-touch default configuration for programmable pages ********************************************

  // Default configuration of the switches of the user pages - this configuration will be restored when selecting menu - firmware menu - Init commands

  // Every switch can have any number commands. Only the first command is shown in the display.

  #define LOWEST_USER_PAGE 1 // Minimum value for PAGE DOWN

  #define PAGE_VCTOUCH_DEFAULT 0
  #define PAGE_VCTOUCH_COMBO1 1
  #define PAGE_VCTOUCH_COMBO2 2
  #define PAGE_VCTOUCH_FUNCTIONS_TEST 3
  #define PAGE_VCTOUCH_GM_TEST 4

const QVector<Cmd_struct> VCtouch_commands = {
    // ******************************* PAGE 00: Default page *************************************************
    // Page, Switch, Type, Device, Data1, Data2, Value1, Value2, Value3, Value4, Value5
    //{PAGE_VCTOUCH_DEFAULT, LABEL, 'D', 'E', 'F', 'A', 'U', 'L', 'T', ' ' },
    {PAGE_VCTOUCH_DEFAULT, 1, NOTHING, COMMON}, // ** Switch 01 **
    {PAGE_VCTOUCH_DEFAULT, 2, NOTHING, COMMON}, // ** Switch 02 **
    {PAGE_VCTOUCH_DEFAULT, 3, NOTHING, COMMON}, // ** Switch 03 **
    {PAGE_VCTOUCH_DEFAULT, 4, NOTHING, COMMON}, // ** Switch 04 **
    {PAGE_VCTOUCH_DEFAULT, 5, NOTHING, COMMON}, // ** Switch 05 **
    {PAGE_VCTOUCH_DEFAULT, 6, NOTHING, COMMON}, // ** Switch 06 **
    {PAGE_VCTOUCH_DEFAULT, 7, NOTHING, COMMON}, // ** Switch 07 **
    {PAGE_VCTOUCH_DEFAULT, 8, NOTHING, COMMON}, // ** Switch 08 **
    {PAGE_VCTOUCH_DEFAULT, 9, NOTHING, COMMON}, // ** Switch 09 **
    {PAGE_VCTOUCH_DEFAULT, 10, NOTHING, COMMON}, // ** Switch 10 **
    {PAGE_VCTOUCH_DEFAULT, 11, PAGE, COMMON, PREV}, // ** Switch 13 **
    {PAGE_VCTOUCH_DEFAULT, 11 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 13 **
    {PAGE_VCTOUCH_DEFAULT, 12, PAGE, COMMON, NEXT}, // ** Switch 14 **
    {PAGE_VCTOUCH_DEFAULT, 12 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 14 **
    {PAGE_VCTOUCH_DEFAULT, 13, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 13 **
    {PAGE_VCTOUCH_DEFAULT, 13 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 13 **
    {PAGE_VCTOUCH_DEFAULT, 14, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 14 **
    {PAGE_VCTOUCH_DEFAULT, 14 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 14 **
    {PAGE_VCTOUCH_DEFAULT, 15, TAP_TEMPO, COMMON}, // ** Switch 15 **
    {PAGE_VCTOUCH_DEFAULT, 15 | ON_LONG_PRESS, GLOBAL_TUNER, COMMON}, // ** Switch 15 **
    {PAGE_VCTOUCH_DEFAULT, 16, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 1 or expr pedal 1
    {PAGE_VCTOUCH_DEFAULT, 17, NOTHING, COMMON}, // External switch 2
    {PAGE_VCTOUCH_DEFAULT, 18, NOTHING, COMMON}, // External switch 3 or expr pedal 2
    {PAGE_VCTOUCH_DEFAULT, 19, NOTHING, COMMON}, // External switch 4
    {PAGE_VCTOUCH_DEFAULT, 20, NOTHING, COMMON}, // External switch 5 or expr pedal 3
    {PAGE_VCTOUCH_DEFAULT, 21, NOTHING, COMMON}, // External switch 6
    {PAGE_VCTOUCH_DEFAULT, 22, PATCH, CURRENT, PREV}, // External switch 7 or expr pedal 4
    {PAGE_VCTOUCH_DEFAULT, 23, PATCH, CURRENT, NEXT}, // External switch 8
    {PAGE_VCTOUCH_DEFAULT, 24, NOTHING, COMMON}, // ** Midi Switch 1 **

    // ******************************* PAGE 01: GP+GR *************************************************
    {PAGE_VCTOUCH_COMBO1, LABEL, 'G', 'P', '+', 'G', 'R', ' ', ' ', ' ' },
    {PAGE_VCTOUCH_COMBO1, 1, PATCH, GP10, BANKSELECT, 1, 5}, // ** Switch 01 **
    {PAGE_VCTOUCH_COMBO1, 2, PATCH, GP10, BANKSELECT, 2, 5}, // ** Switch 02 **
    {PAGE_VCTOUCH_COMBO1, 3, PATCH, GP10, BANKSELECT, 3, 5}, // ** Switch 03 **
    {PAGE_VCTOUCH_COMBO1, 4, PATCH, GP10, BANKSELECT, 4, 5}, // ** Switch 04 **
    {PAGE_VCTOUCH_COMBO1, 5, PATCH, GP10, BANKSELECT, 5, 5}, // ** Switch 05 **
    {PAGE_VCTOUCH_COMBO1, 6, PATCH, GR55, BANKSELECT, 1, 5}, // ** Switch 06 **
    {PAGE_VCTOUCH_COMBO1, 7, PATCH, GR55, BANKSELECT, 2, 5}, // ** Switch 07 **
    {PAGE_VCTOUCH_COMBO1, 8, PATCH, GR55, BANKSELECT, 3, 5}, // ** Switch 08 **
    {PAGE_VCTOUCH_COMBO1, 9, PATCH, GR55, BANKSELECT, 4, 5}, // ** Switch 09 **
    {PAGE_VCTOUCH_COMBO1, 10, PATCH, GR55, BANKSELECT, 5, 5}, // ** Switch 10 **
    {PAGE_VCTOUCH_COMBO1, 13, PATCH, CURRENT, BANKDOWN, 5}, // ** Switch 11 **
    {PAGE_VCTOUCH_COMBO1, 13 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 13 **
    {PAGE_VCTOUCH_COMBO1, 14, PATCH, CURRENT, BANKUP, 5}, // ** Switch 12 **
    {PAGE_VCTOUCH_COMBO1, 14 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 13 **

    // ******************************* PAGE 02: GPVGGR *************************************************
    {PAGE_VCTOUCH_COMBO2, LABEL, 'G', 'P', 'V', 'G', 'G', 'R', ' ', ' ' },
    {PAGE_VCTOUCH_COMBO2, 1, PATCH, GP10, BANKSELECT, 1, 3}, // ** Switch 01 **
    {PAGE_VCTOUCH_COMBO2, 2, PATCH, GP10, BANKSELECT, 2, 3}, // ** Switch 02 **
    {PAGE_VCTOUCH_COMBO2, 3, PATCH, GP10, BANKSELECT, 3, 3}, // ** Switch 03 **
    {PAGE_VCTOUCH_COMBO2, 4, PATCH, VG99, BANKSELECT, 1, 4}, // ** Switch 04 **
    {PAGE_VCTOUCH_COMBO2, 5, PATCH, VG99, BANKSELECT, 2, 4}, // ** Switch 05 **
    {PAGE_VCTOUCH_COMBO2, 6, PATCH, GR55, BANKSELECT, 1, 3}, // ** Switch 06 **
    {PAGE_VCTOUCH_COMBO2, 7, PATCH, GR55, BANKSELECT, 2, 3}, // ** Switch 07 **
    {PAGE_VCTOUCH_COMBO2, 8, PATCH, GR55, BANKSELECT, 3, 3}, // ** Switch 08 **
    {PAGE_VCTOUCH_COMBO2, 9, PATCH, VG99, BANKSELECT, 3, 4}, // ** Switch 09 **
    {PAGE_VCTOUCH_COMBO2, 10, PATCH, VG99, BANKSELECT, 4, 4}, // ** Switch 10 **
    {PAGE_VCTOUCH_COMBO2, 13, PATCH, CURRENT, BANKDOWN, 3}, // ** Switch 11 **
    {PAGE_VCTOUCH_COMBO2, 13 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 13 **
    {PAGE_VCTOUCH_COMBO2, 14, PATCH, CURRENT, BANKUP, 3}, // ** Switch 12 **
    {PAGE_VCTOUCH_COMBO2, 14 | ON_LONG_PRESS, MODE, COMMON, SELECT, DEVICE_MODE}, // ** Switch 13 **

    // ******************************* PAGE 03: FUNCTION TEST *************************************************
    {PAGE_VCTOUCH_FUNCTIONS_TEST, LABEL, 'F', 'U', 'N', 'C', 'T', 'I', 'O', 'N' },
    {PAGE_VCTOUCH_FUNCTIONS_TEST, LABEL, ' ', 'T', 'E', 'S', 'T', ' ', ' ', ' ' },
    {PAGE_VCTOUCH_FUNCTIONS_TEST, 1, PATCH, CURRENT, SELECT, 1, 0}, // ** Switch 01 **
    {PAGE_VCTOUCH_FUNCTIONS_TEST, 2, PATCH, CURRENT, BANKDOWN, 1}, // ** Switch 02 **
    {PAGE_VCTOUCH_FUNCTIONS_TEST, 3, PATCH, CURRENT, BANKSELECT, 1, 1}, // ** Switch 03 **
    {PAGE_VCTOUCH_FUNCTIONS_TEST, 4, PATCH, CURRENT, BANKUP, 1}, // ** Switch 04 **
    {PAGE_VCTOUCH_FUNCTIONS_TEST, 5, PATCH, CURRENT, NEXT}, // ** Switch 05 **
    {PAGE_VCTOUCH_FUNCTIONS_TEST, 6, PARAMETER, CURRENT, 0, TOGGLE, 1, 0}, // ** Switch 06 **
    {PAGE_VCTOUCH_FUNCTIONS_TEST, 7, PARAMETER, CURRENT, 1, TOGGLE, 1, 0}, // ** Switch 07 **
    {PAGE_VCTOUCH_FUNCTIONS_TEST, 8, MUTE, CURRENT}, // ** Switch 08 **
    {PAGE_VCTOUCH_FUNCTIONS_TEST, 9, SET_TEMPO, COMMON, 95}, // ** Switch 9 **
    {PAGE_VCTOUCH_FUNCTIONS_TEST, 10, SET_TEMPO, COMMON, 120}, // ** Switch 10 **

    // ******************************* PAGE 04: GM TEST *************************************************
    {PAGE_VCTOUCH_GM_TEST, LABEL, 'G', 'E', 'N', '.', 'M', 'I', 'D', 'I' },
    {PAGE_VCTOUCH_GM_TEST, LABEL, ' ', 'T', 'E', 'S', 'T', ' ', ' ', ' ' },
    {PAGE_VCTOUCH_GM_TEST, 1, MIDI_PC, COMMON, SELECT, 1, 1, PORT3_NUMBER}, // ** Switch 01 **
    {PAGE_VCTOUCH_GM_TEST, 1 | LABEL, 'C', 'U', 'S', 'T', 'O', 'M', ' ', 'L'}, // ** Switch 01 **
    {PAGE_VCTOUCH_GM_TEST, 1 | LABEL, 'A', 'B', 'E', 'L', ' ', 'P', 'C', '1'}, // ** Switch 01 **
    {PAGE_VCTOUCH_GM_TEST, 2, MIDI_PC, COMMON, PREV, 9, 1, PORT3_NUMBER}, // ** Switch 02 **
    {PAGE_VCTOUCH_GM_TEST, 3, MIDI_PC, COMMON, NEXT, 9, 1, PORT3_NUMBER}, // ** Switch 03 **
    {PAGE_VCTOUCH_GM_TEST, 4, MIDI_NOTE, COMMON, 52, 100, 1, PORT3_NUMBER}, // ** Switch 04 **
    {PAGE_VCTOUCH_GM_TEST, 5, MIDI_CC, COMMON, 30, CC_ONE_SHOT, 127, 0, 1, PORT3_NUMBER}, // ** Switch 05 **
    {PAGE_VCTOUCH_GM_TEST, 5 | LABEL, 'O', 'N', 'E', ' ', 'S', 'H', 'O', 'T'}, // ** Switch 05 **
    {PAGE_VCTOUCH_GM_TEST, 6, MIDI_CC, COMMON, 31, CC_MOMENTARY, 127, 0, 1, PORT3_NUMBER}, // ** Switch 06 **
    {PAGE_VCTOUCH_GM_TEST, 6 | LABEL, 'M', 'O', 'M', 'E', 'N', 'T', 'A', 'R'}, // ** Switch 06 **
    {PAGE_VCTOUCH_GM_TEST, 6 | LABEL, 'Y', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // ** Switch 06 **
    {PAGE_VCTOUCH_GM_TEST, 7, MIDI_CC, COMMON, 30, CC_TOGGLE, 127, 0, 1, PORT3_NUMBER}, // ** Switch 07 **
    {PAGE_VCTOUCH_GM_TEST, 7 | LABEL, 'T', 'O', 'G', 'G', 'L', 'E', ' ', ' '}, // ** Switch 07 **
    {PAGE_VCTOUCH_GM_TEST, 8, MIDI_CC, COMMON, 31, CC_TOGGLE_ON, 127, 0, 1, PORT3_NUMBER}, // ** Switch 08 **
    {PAGE_VCTOUCH_GM_TEST, 8 | LABEL, 'T', 'O', 'G', 'G', 'L', 'E', ' ', 'O'}, // ** Switch 08 **
    {PAGE_VCTOUCH_GM_TEST, 8 | LABEL, 'N', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // ** Switch 08 **
    {PAGE_VCTOUCH_GM_TEST, 9, MIDI_CC, COMMON, 32, CC_UPDOWN, 127, 0, 1, PORT3_NUMBER}, // ** Switch 09 **
    {PAGE_VCTOUCH_GM_TEST, 10, MIDI_CC, COMMON, 33, CC_STEP, 3, 0, 1, PORT3_NUMBER}, // ** Switch 10 **
};

QVector<Cmd_struct> Fixed_commands = {};
QVector<Cmd_struct> Commands = {};

int Number_of_pages = 0;
int number_of_cmds = 0;
uint8_t page_default = 0;
uint8_t first_fixed_cmd_page = 0;
uint8_t last_fixed_cmd_page = 0;

const QStringList VC_switch_names = { "On Page Select", "Switch 1", "Switch 2", "Switch 3", "Switch 4", "Switch 5", "Switch 6", "Switch 7", "Switch 8",
                                      "Switch 9", "Switch 10", "Switch 11", "Switch 12", "Switch 13", "Switch 14", "Switch 15", "Switch 16",
                                      "Ext 1 / Exp1", "Ext 2", "Ext 3 / Exp 2", "Ext 4", "Ext 5 / Exp 3", "Ext 6", "Ext 7 / Exp 4", "Ext 8" };
const QStringList VCmini_switch_names = { "On Page Select", "Switch 1", "Switch 2", "Switch 3", "Encoder #1", "Encoder #1 SW", "Encoder #2", "Encoder #2 SW", "Ext 1 / Exp1",
                                          "Ext 2", "MIDI Switch 1", "MIDI Switch 2", "MIDI Switch 3", "MIDI Switch 4", "MIDI Switch 5", "MIDI Switch 6", "MIDI Switch 7",
                                          "MIDI Switch 8", "MIDI Switch 9", "MIDI Switch 10", "MIDI Switch 11", "MIDI Switch 12", "MIDI Switch 13", "MIDI Switch 14", "MIDI Switch 15" };
const QStringList VCtouch_switch_names = { "On Page Select", "Switch 1", "Switch 2", "Switch 3", "Switch 4", "Switch 5", "Switch 6", "Switch 7", "Switch 8",
                                           "Switch 9", "Switch 10", "Switch 11", "Switch 12", "Switch 13", "Switch 14", "Switch 15",
                                           "Ext 1 / Exp1", "Ext 2", "Ext 3 / Exp 2", "Ext 4", "Ext 5 / Exp 3", "Ext 6", "MIDI Switch 1", "MIDI Switch 2", "MIDI Switch 3" };
QStringList switchnames = {};
