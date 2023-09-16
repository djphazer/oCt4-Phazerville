// Phazerville Suite framework for oCt4drv
// Copyright (C) 2023 Nicholas J. Michalek
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

/* Phazerville Suite Framework for T4
 *
 * Building the basic foundation for porting Hemisphere applets and such
 */
#include "phazerville.h"

#include "hem/hemisphere_config.h"

namespace phz {

static PhazervilleApp instance;

oct4::api::Menu::MenuRegistrar<PhazervilleApp> register_phzview(&instance);
oct4::api::Processor::ProcRegistrar<PhazervilleApp> register_phzproc(&instance);

// placeholders for calibration offsets
const uint16_t kOctaveZero = 0x7fff;
const uint16_t kADCZero = 2750;

// core processing logic
void PhazervilleApp::Process(uint32_t ticks, const api::Processor::Inputs &inputs, api::Processor::Outputs &outputs) 
{
  HS::ticks_ = ticks;

  // --- Load IOFrame inputs
  ForAllChannels(i) {
    HS::frame.gate_high[i] = inputs.digital_inputs[i].raised();
    HS::frame.clocked[i] = inputs.digital_inputs[i].triggered();
    HS::frame.inputs[i] = kADCZero - int(inputs.analog_inputs[i]);
  }
  HS::frame.Tick(); // takes care of dynamic things

  // --- Applet processing
  AttenuateOffset_left.Controller();

  // --- Send IOFrame outputs
  ForAllChannels(i) {
    outputs.dac[i] = UINT16_MAX - uint16_t(HS::frame.outputs[i] + kOctaveZero);
  }

}

// App activation / suspension ?
void PhazervilleApp::HandleMenuEvent(api::MenuEvent menu_event)
{
  switch (menu_event) {
    case api::MENU_ACTIVATE:
        oct4::SystemCore::Execute(this, oct4::SystemCore::CORE_FREQ_OC16Khz);
        break;
    case api::MENU_DEACTIVATE: break;
  }
}

// button/encoder actions
void PhazervilleApp::HandleEvent(const UI::Event &event)
{
    // keepin it simple
    switch (event.type) {
        case UI::EVENT_BUTTON_PRESS:
            if (event.control == UI::CONTROL_BUTTON_R )
                AttenuateOffset_left.OnButtonPress();
            break;
        case UI::EVENT_ENCODER:
            AttenuateOffset_left.OnEncoderMove(event.value);
            break;
        default: break;
    }
    /*
    {UI::EVENT_BUTTON_PRESS, UI::CONTROL_BUTTON_UP, &HWTestApp::evButtonUp},
    {UI::EVENT_BUTTON_PRESS, UI::CONTROL_BUTTON_DOWN, &HWTestApp::evButtonDown},
    {UI::EVENT_BUTTON_PRESS, UI::CONTROL_BUTTON_R, &HWTestApp::evButtonR},
    {UI::EVENT_ENCODER, UI::CONTROL_ENCODER_L, &HWTestApp::evEncoderL},
    {UI::EVENT_ENCODER, UI::CONTROL_ENCODER_R, &HWTestApp::evEncoderR},
    */
}

// graphics
void PhazervilleApp::Draw(weegfx::Graphics &gfx) const
{
    if (!HemisphereApplet::graphics) HemisphereApplet::graphics = &gfx;

    gfx.setPrintPos(1, 2);
    gfx.print("Phazerville T4 Test");
    gfx.drawLine(0, 10, 62, 10);
    gfx.drawLine(0, 11, 62, 11);

    // TODO: call applet Views
    AttenuateOffset_left.View();

    gfx.drawVLine(64, 15, 48);

#ifdef OCT4_ENABLE_DEBUG
    // print ADC values
    weegfx::coord_t y = 15;
    int i = 0;
    for (auto v : HS::frame.inputs) {
      gfx.setPrintPos(65, y);
      gfx.printf("CV%d ", i);
      gfx.printf("%4d", v);

      /*
      gfx.setPrintPos(x, 32);
      gfx.printf("%04x", v);
      */

      ++i;
      y += 10;
    }
#endif
}

}  // namespace phz
