#pragma once
#include <cstdint>
#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "bjorklund.h"

// stinkin macros
#define LEFT_HEMISPHERE 0
#define RIGHT_HEMISPHERE 1
#ifdef BUCHLA_4U
#define PULSE_VOLTAGE 8
#define HEMISPHERE_MAX_CV 15360
#define HEMISPHERE_CENTER_CV 7680 // 5V
#define HEMISPHERE_MIN_CV 0
#elif defined(VOR)
#define PULSE_VOLTAGE 8
#define HEMISPHERE_MAX_CV (HS::octave_max * 12 << 7)
#define HEMISPHERE_CENTER_CV 0
#define HEMISPHERE_MIN_CV (HEMISPHERE_MAX_CV - 15360)
#else
#define PULSE_VOLTAGE 5
#define HEMISPHERE_MAX_CV 9216 // 6V
#define HEMISPHERE_CENTER_CV 0
#define HEMISPHERE_MIN_CV -4608 // -3V
#endif
#define HEMISPHERE_3V_CV 4608
#define HEMISPHERE_MAX_INPUT_CV 9216 // 6V
#define HEMISPHERE_CENTER_DETENT 80
#define HEMISPHERE_CLOCK_TICKS 50
#define HEMISPHERE_CURSOR_TICKS 12000
#define HEMISPHERE_ADC_LAG 33
#define HEMISPHERE_CHANGE_THRESHOLD 32

// Codes for help system sections
enum HEM_HELP_SECTIONS {
HEMISPHERE_HELP_DIGITALS = 0,
HEMISPHERE_HELP_CVS = 1,
HEMISPHERE_HELP_OUTS = 2,
HEMISPHERE_HELP_ENCODER = 3
};
const char * HEM_HELP_SECTION_NAMES[4] = {"Dig", "CV", "Out", "Enc"};

// Simulated fixed floats by multiplying and dividing by powers of 2
#ifndef int2simfloat
#define int2simfloat(x) (x << 14)
#define simfloat2int(x) (x >> 14)
typedef int32_t simfloat;
#endif

// Hemisphere-specific macros
#define BottomAlign(h) (62 - h)
#define ForEachChannel(ch) for(int_fast8_t ch = 0; ch < 2; ch++)
#define gfx_offset (hemisphere * 64) // Graphics offset, based on the side
#define io_offset (hemisphere * 2) // Input/Output offset, based on the side

#define constrain(amt, low, high) ({ \
  typeof(amt) _amt = (amt); \
  typeof(low) _low = (low); \
  typeof(high) _high = (high); \
  (_amt < _low) ? _low : ((_amt > _high) ? _high : _amt); \
})

#define HEMISPHERE_SIM_CLICK_TIME 1000
#define HEMISPHERE_DOUBLE_CLICK_TIME 8000
#define HEMISPHERE_PULSE_ANIMATION_TIME 500
#define HEMISPHERE_PULSE_ANIMATION_TIME_LONG 1200

// ok, ok; this actually just needs to be a HemisphereApplet class interface
class HemisphereApplet {
  enum ModalEditMode {
    LEGACY = 0, MODAL, MODAL_WRAP, LAST
  };
  enum Side { LEFT, RIGHT };
  
  int cursor_ = 0;
  bool isEditing = 0;
  ModalEditMode modal_edit_mode = MODAL;
  static int cursor_countdown[2];


  // helper functions
  void ResetCursor(int n = 0) {
    cursor_countdown[n] = HEMISPHERE_CURSOR_TICKS;
  }

  // handle modal edit mode toggle or cursor advance
  void CursorAction(int &cursor, int max) {
    if (modal_edit_mode == LEGACY) {
      ++cursor;
      cursor %= max + 1;
      ResetCursor();
    } else
      isEditing = !isEditing;
  }
  void MoveCursor(int &cursor, int direction, int max) {
        cursor += direction;
        if (modal_edit_mode == MODAL_WRAP) {
            if (cursor < 0) cursor = max;
            else cursor %= max + 1;
        } else {
            cursor = constrain(cursor, 0, max);
        }
        ResetCursor();
  }
  bool EditMode() {
    return (isEditing || !modal_edit_mode);
  }

};

