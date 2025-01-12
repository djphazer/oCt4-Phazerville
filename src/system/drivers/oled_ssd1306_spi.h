// oCt4drv -- Basic drivers for O_C + T4.0
// Copyright (C) 2022 Patrick Dowling (pld@gurkenkiste.com)
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

#include "system/drivers/oct4_spi.h"
#include "system/drivers/ssd1306.h"
#include "util/util_macros.h"

namespace oct4 {
namespace drivers {

class OledSSD1306 {
public:
  enum class TransferStatus {
    BUSY,
    PAGE_COMPLETE,
    FRAME_COMPLETE,
  };

  DELETE_COPY_ASSIGN(OledSSD1306);
  explicit OledSSD1306(SPI &spi) : spi_(spi) {}

  void Init(bool display_on);

  static constexpr size_t kDisplayPixelW = WEEGFX_FRAME_W;
  static constexpr size_t kDisplayPixelH = WEEGFX_FRAME_H;

  static constexpr size_t kFrameSize = kDisplayPixelW * kDisplayPixelH / 8;

  enum TransferMode {
    TRANSFER_MODE_PAGES,
    TRANSFER_MODE_SUBPAGES,
  };

  static constexpr size_t kPageSize = kFrameSize / 8;
  static constexpr size_t kNumPages = kFrameSize / kPageSize;

  static constexpr uint8_t kDefaultOffset = 2;
  static constexpr uint8_t kDefaultContrast = 0xCF;

  void SetupFrame(const uint8_t *frame, TransferMode transfer_mode);
  bool frame_valid() const { return frame_data_; }

  void AsyncTransferBegin();
  TransferStatus AsyncTransferWait();

  void AdjustOffset(uint8_t offset);
  void CmdDisplayOn(bool on);
  void CmdSetContrast(uint8_t contrast);

protected:
  SPI &spi_;

  uint8_t col_offset_ = kDefaultOffset;

  const uint8_t *frame_data_ = nullptr;
  size_t transfer_pos_ = 0;
  size_t transfer_size_ = 0;

  static constexpr size_t kPageHeaderSize = 3;
  static constexpr size_t kCommandBufferSize = 1 + 2;

  uint8_t page_header_[kPageHeaderSize] = {0};
  uint8_t command_buffer_[kCommandBufferSize] = {0};
};

}  // namespace drivers
}  // namespace oct4
