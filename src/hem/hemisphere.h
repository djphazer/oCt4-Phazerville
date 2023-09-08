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
const char * const HEM_HELP_SECTION_NAMES[4] = {"Dig", "CV", "Out", "Enc"};

// Simulated fixed floats by multiplying and dividing by powers of 2
#ifndef int2simfloat
#define int2simfloat(x) (x << 14)
#define simfloat2int(x) (x >> 14)
typedef int32_t simfloat;
#endif

// Hemisphere-specific macros
#define BottomAlign(h) (62 - h)
#define ForEachChannel(ch) for(int_fast8_t ch = 0; ch < 2; ch++)
#define ForAllChannels(ch) for(int_fast8_t ch = 0; ch < 4; ch++)
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

class HemisphereApplet;

// actual applet API
typedef struct Applet {
    int id;
    const char * name;
    HemisphereApplet * applet;
} Applet;

#include "hem/HSIOFrame.h"
#include "hem/HSicons.h"
#include "hem/HSClockManager.h"

namespace HS {
  enum ModalEditMode {
    LEGACY = 0, MODAL, MODAL_WRAP, LAST
  };

  static uint32_t ticks_ = 0;
  static ModalEditMode modal_edit_mode = MODAL;
  static IOFrame frame;
  Applet * selected_applet[2];
}

template <typename T>
using AppletRegistrar = util::StaticTypeRegistry<Applet, 32>::Registrar<T>;

#define DECLARE_APPLET(id, name, clazz) \
    clazz clazz ## _instance; \
    Applet clazz ## _applet = { id, name, &clazz ## _instance };
    
//AppletRegistrar<Applet> register_applet( &clazz ## _applet )

using namespace HS;

class HemisphereApplet {
public:
  enum Side { LEFT, RIGHT };

  int cursor_ = 0;
  bool isEditing = 0;
  bool hemisphere;

  static int cursor_countdown[2];
  static weegfx::Graphics *graphics;

  // helper functions
  void ResetCursor() {
    cursor_countdown[hemisphere] = HEMISPHERE_CURSOR_TICKS;
  }
  /* Check cursor blink cycle. */
  bool CursorBlink() {
      return (cursor_countdown[hemisphere] > 0);
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

  //////////////// Offset I/O methods
  ////////////////////////////////////////////////////////////////////////////////
  int In(int ch) {
      return frame.inputs[io_offset + ch];
  }

  // Apply small center detent to input, so it reads zero before a threshold
  int DetentedIn(int ch) {
      return (In(ch) > (HEMISPHERE_CENTER_CV + HEMISPHERE_CENTER_DETENT) || In(ch) < (HEMISPHERE_CENTER_CV - HEMISPHERE_CENTER_DETENT))
          ? In(ch) : HEMISPHERE_CENTER_CV;
  }

  int SmoothedIn(int ch) {
      /* TODO
      ADC_CHANNEL channel = (ADC_CHANNEL)(ch + io_offset);
      return OC::ADC::value(channel);
      */
      return In(ch);
  }

  braids::Quantizer* GetQuantizer(int ch) {
      return &HS::quantizer[io_offset + ch];
  }

  // Standard bi-polar CV modulation scenario
  void Modulate(auto &param, const int ch, const int min = 0, const int max = 255) {
      int cv = DetentedIn(ch);
      param = constrain(param + Proportion(cv, HEMISPHERE_MAX_INPUT_CV, max), min, max);
      //return param_mod;
  }

  void Out(int ch, int value, int octave = 0) {
      frame.Out( ch + io_offset, value + (octave * (12 << 7)));
  }

  void SmoothedOut(int ch, int value, int kSmoothing) {
      int channel = ch + io_offset;
      value = (frame.outputs_smooth[channel] * (kSmoothing - 1) + value) / kSmoothing;
      frame.outputs[channel] = frame.outputs_smooth[channel] = value;
  }

  /*
   * Has the specified Digital input been clocked this cycle?
   *
   * If physical is true, then logical clock types (master clock forwarding and metronome) will
   * not be used.
   */
  bool Clock(int ch, bool physical = 0) {
      bool clocked = 0;
      bool useTock = (!physical && clock_m.IsRunning());

      // clock triggers
      if (useTock && clock_m.GetMultiply(ch + io_offset) != 0)
          clocked = clock_m.Tock(ch + io_offset);
      else
          clocked = frame.clocked[ch + io_offset];

      // Try to eat a boop
      clocked = clocked || clock_m.Beep(io_offset + ch);

      if (clocked) {
          frame.cycle_ticks[io_offset + ch] = HS::ticks_ - frame.last_clock[io_offset + ch];
          frame.last_clock[io_offset + ch] = HS::ticks_;
      }
      return clocked;
  }

  void ClockOut(const int ch) {
      frame.ClockOut( io_offset + ch );
  }

  bool Gate(int ch) {
      return frame.gate_high[ch + io_offset];
  }

  void GateOut(int ch, bool high) {
      Out(ch, 0, (high ? PULSE_VOLTAGE : 0));
  }

  // Buffered I/O functions
  int ViewIn(int ch) {return frame.inputs[io_offset + ch];}
  int ViewOut(int ch) {return frame.outputs[io_offset + ch];}
  int ClockCycleTicks(int ch) {return frame.cycle_ticks[io_offset + ch];}
  bool Changed(int ch) {return frame.changed_cv[io_offset + ch];}

  void StartADCLag(bool ch = 0) {
      frame.adc_lag_countdown[io_offset + ch] = HEMISPHERE_ADC_LAG;
  }

  bool EndOfADCLag(bool ch = 0) {
      if (frame.adc_lag_countdown[io_offset + ch] < 0) return false;
      return (--frame.adc_lag_countdown[io_offset + ch] == 0);
  }

  // --- GFX helpers ---
  //////////////// Offset graphics methods
  ////////////////////////////////////////////////////////////////////////////////
  void gfxCursor(int x, int y, int w, int h = 9) { // assumes standard text height for highlighting
      if (isEditing) gfxInvert(x, y - h, w, h);
      else if (CursorBlink()) gfxLine(x, y, x + w - 1, y);
  }

  void gfxPos(int x, int y) {
      graphics->setPrintPos(x + gfx_offset, y);
  }

  void gfxPrint(int x, int y, const char *str) {
      graphics->setPrintPos(x + gfx_offset, y);
      graphics->print(str);
  }

  void gfxPrint(int x, int y, int num) {
      graphics->setPrintPos(x + gfx_offset, y);
      graphics->print(num);
  }

  void gfxPrint(int x_adv, int num) { // Print number with character padding
      for (int c = 0; c < (x_adv / 6); c++) gfxPrint(" ");
      gfxPrint(num);
  }

  void gfxPrint(const char *str) {
      graphics->print(str);
  }

  void gfxPrint(int num) {
      graphics->print(num);
  }

  /* Convert CV value to voltage level and print  to two decimal places */
  void gfxPrintVoltage(int cv) {
      int v = (cv * 100) / (12 << 7);
      bool neg = v < 0 ? 1 : 0;
      if (v < 0) v = -v;
      int wv = v / 100; // whole volts
      int dv = v - (wv * 100); // decimal
      gfxPrint(neg ? "-" : "+");
      gfxPrint(wv);
      gfxPrint(".");
      if (dv < 10) gfxPrint("0");
      gfxPrint(dv);
      gfxPrint("V");
  }

  void gfxPixel(int x, int y) {
      graphics->setPixel(x + gfx_offset, y);
  }

  void gfxFrame(int x, int y, int w, int h) {
      graphics->drawFrame(x + gfx_offset, y, w, h);
  }

  void gfxRect(int x, int y, int w, int h) {
      graphics->drawRect(x + gfx_offset, y, w, h);
  }

  void gfxInvert(int x, int y, int w, int h) {
      graphics->invertRect(x + gfx_offset, y, w, h);
  }

  void gfxLine(int x, int y, int x2, int y2) {
      graphics->drawLine(x + gfx_offset, y, x2 + gfx_offset, y2);
  }

  /*
  void gfxLine(int x, int y, int x2, int y2, bool dotted) {
      graphics->drawLine(x + gfx_offset, y, x2 + gfx_offset, y2, dotted ? 2 : 1);
  }

  void gfxDottedLine(int x, int y, int x2, int y2, uint8_t p = 2) {
      graphics->drawLine(x + gfx_offset, y, x2 + gfx_offset, y2, p);
  }
  */

  void gfxCircle(int x, int y, int r) {
      graphics->drawCircle(x + gfx_offset, y, r);
  }

  void gfxBitmap(int x, int y, int w, const uint8_t *data) {
      graphics->drawBitmap8(x + gfx_offset, y, w, data);
  }

  void gfxIcon(int x, int y, const uint8_t *data) {
      gfxBitmap(x, y, 8, data);
  }

  uint8_t pad(int range, int number) {
      uint8_t padding = 0;
      while (range > 1)
      {
          if (abs(number) < range) padding += 6;
          range = range / 10;
      }
      return padding;
  }

  //////////////// Hemisphere-specific graphics methods
  ////////////////////////////////////////////////////////////////////////////////

  /* Show channel-grouped bi-lateral display */
  void gfxSkyline() {
      ForEachChannel(ch)
      {
          int height = ProportionCV(ViewIn(ch), 32);
          gfxFrame(23 + (10 * ch), BottomAlign(height), 6, 63);

          height = ProportionCV(ViewOut(ch), 32);
          gfxInvert(3 + (46 * ch), BottomAlign(height), 12, 63);
      }
  }

  void gfxHeader(const char *str) {
      gfxPrint(1, 2, str);
      gfxLine(0, 10, 62, 10);
      gfxLine(0, 11, 62, 11);
  }

};

weegfx::Graphics *HemisphereApplet::graphics = 0;
int HemisphereApplet::cursor_countdown[2] = {0,0};
