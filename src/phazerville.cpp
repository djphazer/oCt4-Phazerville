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

oct4::api::Processor::ProcRegistrar<PhazervilleApp> register_proc(&instance);
oct4::api::Menu::MenuRegistrar<PhazervilleApp> register_menu(&instance);

// core processing logic
void PhazervilleApp::Process(uint32_t ticks, const api::Processor::Inputs &inputs, api::Processor::Outputs &outputs) 
{
  HS::ticks_ = ticks;

  // --- Load IOFrame inputs
  ForAllChannels(i) {
    HS::frame.gate_high[i] = inputs.digital_inputs[i].raw;
    HS::frame.clocked[i] = inputs.digital_inputs[i].flags;
    HS::frame.inputs[i] = inputs.analog_inputs[i];
  }
  HS::frame.Tick(); // takes care of dynamic things

  // --- Applet processing
  AttenuateOffset_instance.Controller();

  // --- Send IOFrame outputs
  ForAllChannels(i) {
    outputs.dac[i] = HS::frame.outputs[i];
  }

  /* DEMO: sum all inputs to output D
   *
  outputs.dac[3] = 0;
  for (auto v : inputs.analog_inputs) {
    outputs.dac[3] += v;
  }
  */
}

// App activation / suspension ?
void PhazervilleApp::HandleMenuEvent(api::MenuEvent menu_event)
{
  switch (menu_event) {
    case api::MENU_ACTIVATE: break;
    case api::MENU_DEACTIVATE: break;
  }
}

// button/encoder actions
void PhazervilleApp::HandleEvent(const UI::Event &event)
{
    (void)event;
}

// graphics
void PhazervilleApp::Draw(weegfx::Graphics &gfx) const
{
    gfx.setPrintPos(1, 2);
    gfx.print("Phazerville T4 Test");
    gfx.drawLine(0, 10, 62, 10);
    gfx.drawLine(0, 11, 62, 11);

    // TODO: call applet Views
}

}  // namespace phz
