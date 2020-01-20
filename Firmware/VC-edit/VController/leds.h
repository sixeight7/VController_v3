#ifndef LEDS_H
#define LEDS_H

#include <QApplication>
#include <QVector>

#define NUMBER_OF_SELECTABLE_COLOURS 11

// Defining FX colours types - the colours are set in the menu.
// Colour numbers 241 - 255 are reserved for colour sublists.
#define FX_TYPE_OFF 0
#define FX_DEFAULT_TYPE 240 // For parameters that fit no other category
#define FX_GTR_TYPE 239 // For COSM/guitar settings
#define FX_PITCH_TYPE 238 // For pitch FX
#define FX_FILTER_TYPE 237 // For filter FX
#define FX_DIST_TYPE 236 // For distortion FX
#define FX_AMP_TYPE 235 // For amp FX and amp solo
#define FX_MODULATE_TYPE 234 //B For modulation FX
#define FX_DELAY_TYPE 233 // For delays
#define FX_REVERB_TYPE 232// For reverb FX
#define FX_LOOPER_TYPE 231 // For looper
#define FX_WAH_TYPE 230 // For wahs
#define FX_DYNAMICS_TYPE 229 // For gates/etc

#endif // LEDS_H
