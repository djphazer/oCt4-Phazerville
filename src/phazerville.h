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

#pragma once
#include "api/api_processor.h"
#include "api/api_menu.h"
#include "system/ui/ui_event_dispatcher.h"

namespace oct4 {

class PhazervilleApp : public api::Processor, public api::Menu, public UI::EventDispatch<PhazervilleApp> {
public:
  static constexpr util::FourCC::value_type fourcc = "PHZ1"_4CCV;

  // PROCESSOR
  util::FourCC processor_type() const final { return {fourcc}; }
  void Process(uint32_t ticks, const api::Processor::Inputs &inputs,
               api::Processor::Outputs &outputs) final;
 
  // MENU
  util::FourCC menu_type() const final { return {fourcc}; }
    // increment timers and such here
  void Tick() final { ++ticks_; }

  void HandleMenuEvent(api::MenuEvent menu_event) final;
  void HandleEvent(const UI::Event &event) final;
  void Draw(weegfx::Graphics &gfx) const final;

protected:
  EVENT_DISPATCH_DEFAULT();
  //EVENT_DISPATCH_DECLARE_HANDLER(evButtonUp);

private:
  uint32_t ticks_ = 0;
  int state_ = 0;
};

}  // namespace oct4
