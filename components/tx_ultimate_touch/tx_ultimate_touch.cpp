#include "tx_ultimate_touch.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome
{
  namespace tx_ultimate_touch
  {
    static const char *const TAG = "tx_ultimate_touch";

    void TxUltimateTouch::setup()
    {
      this->reset_parser_();
      ESP_LOGI(TAG, "TX Ultimate touch parser initialized");
    }

    void TxUltimateTouch::loop()
    {
      while (this->available())
      {
        if (this->parse_byte_(static_cast<uint8_t>(this->read())))
          this->process_frame_();
      }
    }

    void TxUltimateTouch::dump_config()
    {
      ESP_LOGCONFIG(TAG, "TX Ultimate Touch:");
      ESP_LOGCONFIG(TAG, "  Long press X offset: %u", static_cast<unsigned>(this->long_press_x_offset_));
      ESP_LOGCONFIG(TAG, "  Long press time triggers: %u", static_cast<unsigned>(this->long_press_time_triggers_.size()));
      ESP_LOGCONFIG(TAG, "  CRC validation: %s", this->validate_crc_enabled_ ? "enabled" : "disabled");
      this->check_uart_settings(115200, 1, uart::UART_CONFIG_PARITY_NONE, 8);
    }

    void TxUltimateTouch::reset_parser_()
    {
      this->parser_state_ = ParserState::WAIT_HEADER_1;
      this->frame_ = Frame{};
      this->data_read_ = 0;
    }

    bool TxUltimateTouch::parse_byte_(uint8_t byte)
    {
      switch (this->parser_state_)
      {
        case ParserState::WAIT_HEADER_1:
          if (byte == EN_MAGIC_WORD_BYTE_1)
            this->parser_state_ = ParserState::WAIT_HEADER_2;
          break;

        case ParserState::WAIT_HEADER_2:
          if (byte == EN_MAGIC_WORD_BYTE_2)
          {
            this->frame_ = Frame{};
            this->data_read_ = 0;
            this->parser_state_ = ParserState::VERSION;
          }
          else
          {
            this->parser_state_ = (byte == EN_MAGIC_WORD_BYTE_1) ? ParserState::WAIT_HEADER_2
                                                                 : ParserState::WAIT_HEADER_1;
          }
          break;

        case ParserState::VERSION:
          if (byte != EN_DEVICE_VERSION)
          {
            ESP_LOGV(TAG, "Ignoring frame with unsupported version 0x%02X", byte);
            this->parser_state_ = (byte == EN_MAGIC_WORD_BYTE_1) ? ParserState::WAIT_HEADER_2
                                                                 : ParserState::WAIT_HEADER_1;
            break;
          }
          this->frame_.version = byte;
          this->parser_state_ = ParserState::OPCODE;
          break;

        case ParserState::OPCODE:
          this->frame_.opcode = byte;
          this->parser_state_ = ParserState::LENGTH;
          break;

        case ParserState::LENGTH:
          this->frame_.length = byte;
          this->data_read_ = 0;
          if (this->frame_.length > EN_MAX_DATA_LENGTH)
          {
            ESP_LOGW(TAG, "Ignoring frame with invalid length %u", this->frame_.length);
            this->reset_parser_();
          }
          else if (this->frame_.length == 0)
          {
            this->parser_state_ = ParserState::CRC_HIGH;
          }
          else
          {
            this->parser_state_ = ParserState::DATA;
          }
          break;

        case ParserState::DATA:
          this->frame_.data[this->data_read_++] = byte;
          if (this->data_read_ >= this->frame_.length)
            this->parser_state_ = ParserState::CRC_HIGH;
          break;

        case ParserState::CRC_HIGH:
          this->frame_.crc = static_cast<uint16_t>(byte) << 8;
          this->parser_state_ = ParserState::CRC_LOW;
          break;

        case ParserState::CRC_LOW:
          this->frame_.crc |= byte;
          this->parser_state_ = ParserState::WAIT_HEADER_1;
          return true;
      }

      return false;
    }

    uint16_t TxUltimateTouch::calculate_crc_() const
    {
      uint16_t crc = 0xFFFF;
      auto update_crc = [&crc](uint8_t value)
      {
        crc ^= static_cast<uint16_t>(value) << 8;
        for (uint8_t bit = 0; bit < 8; bit++)
        {
          if ((crc & 0x8000) != 0)
            crc = static_cast<uint16_t>((crc << 1) ^ 0x1021);
          else
            crc = static_cast<uint16_t>(crc << 1);
        }
      };

      update_crc(this->frame_.version);
      update_crc(this->frame_.opcode);
      update_crc(this->frame_.length);
      for (uint8_t i = 0; i < this->frame_.length; i++)
        update_crc(this->frame_.data[i]);

      return crc;
    }

    bool TxUltimateTouch::validate_crc_() const
    {
      if (!this->validate_crc_enabled_)
        return true;

      const uint16_t calculated = this->calculate_crc_();
      if (calculated != this->frame_.crc)
      {
        ESP_LOGW(TAG, "Ignoring frame with invalid CRC: expected 0x%04X, got 0x%04X",
                 calculated, this->frame_.crc);
        return false;
      }
      return true;
    }

    bool TxUltimateTouch::decode_touch_frame_(TouchPoint *tp, TouchGesture *gesture) const
    {
      if (this->frame_.opcode != EN_OPT_CODE_TOUCH)
        return false;

      tp->x = 0;
      tp->state = 0;
      gesture->from = 0;
      gesture->to = 0;
      gesture->distance = 0;
      gesture->raw_mid = 0;
      gesture->state = 0;

      switch (this->frame_.length)
      {
        case 1:
          if (this->frame_.data[0] == EN_TOUCH_STATE_MULTI_TOUCH)
          {
            tp->state = EN_TOUCH_STATE_MULTI_TOUCH;
            return true;
          }
          if (this->is_valid_position_(this->frame_.data[0]))
          {
            tp->x = this->frame_.data[0];
            tp->state = EN_TOUCH_STATE_RELEASE;
            return true;
          }
          ESP_LOGV(TAG, "Ignoring one-byte touch payload 0x%02X", this->frame_.data[0]);
          return false;

        case 2:
          // Known pattern: LEN=2, DATA[0]=0, DATA[1]=position means touch-down.
          if (this->frame_.data[0] == 0 && this->is_valid_position_(this->frame_.data[1]))
          {
            tp->x = this->frame_.data[1];
            tp->state = EN_TOUCH_STATE_PRESS;
            return true;
          }
          if (this->frame_.data[0] == EN_TOUCH_STATE_RELEASE &&
              this->frame_.data[1] == EN_TOUCH_STATE_MULTI_TOUCH)
          {
            tp->state = EN_TOUCH_STATE_MULTI_TOUCH;
            return true;
          }
          if (this->is_valid_position_(this->frame_.data[0]) &&
              this->is_valid_position_(this->frame_.data[1]))
          {
            tp->x = this->frame_.data[1];
            tp->state = EN_TOUCH_STATE_RELEASE;
            gesture->from = this->frame_.data[1];
            gesture->to = this->frame_.data[0];
            gesture->distance = gesture->to > gesture->from ? gesture->to - gesture->from : gesture->from - gesture->to;
            gesture->state = EN_TOUCH_STATE_DASH;
            return true;
          }
          ESP_LOGV(TAG, "Ignoring two-byte touch payload 0x%02X 0x%02X",
                   this->frame_.data[0], this->frame_.data[1]);
          return false;

        case 3:
          if (this->frame_.data[0] == EN_TOUCH_STATE_SWIPE_RIGHT &&
              this->is_valid_position_(this->frame_.data[1]))
          {
            tp->x = this->frame_.data[1];
            tp->state = EN_TOUCH_STATE_SWIPE_RIGHT;
            gesture->from = this->frame_.data[1];
            gesture->to = this->is_valid_position_(this->frame_.data[2]) ? this->frame_.data[2] : 0;
            gesture->distance = gesture->to > gesture->from ? gesture->to - gesture->from : gesture->from - gesture->to;
            gesture->raw_mid = this->frame_.data[2];
            gesture->state = EN_TOUCH_STATE_SWIPE_RIGHT;
            return true;
          }
          if (this->frame_.data[0] == EN_TOUCH_STATE_SWIPE_LEFT &&
              this->is_valid_position_(this->frame_.data[1]))
          {
            tp->x = this->frame_.data[1];
            tp->state = EN_TOUCH_STATE_SWIPE_LEFT;
            gesture->from = this->frame_.data[1];
            gesture->to = this->is_valid_position_(this->frame_.data[2]) ? this->frame_.data[2] : 0;
            gesture->distance = gesture->to > gesture->from ? gesture->to - gesture->from : gesture->from - gesture->to;
            gesture->raw_mid = this->frame_.data[2];
            gesture->state = EN_TOUCH_STATE_SWIPE_LEFT;
            return true;
          }
          if (this->frame_.data[0] == EN_TOUCH_STATE_SWIPE &&
              this->is_valid_position_(this->frame_.data[2]))
          {
            if (this->frame_.data[1] == EN_TOUCH_STATE_SWIPE_RIGHT)
            {
              tp->x = this->frame_.data[2];
              tp->state = EN_TOUCH_STATE_SWIPE_RIGHT;
              gesture->from = this->frame_.data[2];
              gesture->state = EN_TOUCH_STATE_SWIPE_RIGHT;
              return true;
            }
            if (this->frame_.data[1] == EN_TOUCH_STATE_SWIPE_LEFT)
            {
              tp->x = this->frame_.data[2];
              tp->state = EN_TOUCH_STATE_SWIPE_LEFT;
              gesture->from = this->frame_.data[2];
              gesture->state = EN_TOUCH_STATE_SWIPE_LEFT;
              return true;
            }
          }
          ESP_LOGV(TAG, "Ignoring three-byte touch payload 0x%02X 0x%02X 0x%02X",
                   this->frame_.data[0], this->frame_.data[1], this->frame_.data[2]);
          return false;

        default:
          ESP_LOGV(TAG, "Ignoring touch frame with unsupported data length %u", this->frame_.length);
          return false;
      }
    }

    bool TxUltimateTouch::is_valid_position_(uint8_t x) const
    {
      return x != 0 && x < 0x80;
    }

    TouchFrame TxUltimateTouch::make_touch_frame_() const
    {
      TouchFrame frame;
      frame.opcode = this->frame_.opcode;
      frame.length = this->frame_.length;
      frame.data0 = this->frame_.length > 0 ? this->frame_.data[0] : 0;
      frame.data1 = this->frame_.length > 1 ? this->frame_.data[1] : 0;
      frame.data2 = this->frame_.length > 2 ? this->frame_.data[2] : 0;
      frame.data3 = this->frame_.length > 3 ? this->frame_.data[3] : 0;
      frame.data4 = this->frame_.length > 4 ? this->frame_.data[4] : 0;
      frame.data5 = this->frame_.length > 5 ? this->frame_.data[5] : 0;
      frame.data6 = this->frame_.length > 6 ? this->frame_.data[6] : 0;
      frame.data7 = this->frame_.length > 7 ? this->frame_.data[7] : 0;
      frame.crc = this->frame_.crc;
      return frame;
    }

    void TxUltimateTouch::send_unknown_frame_()
    {
      this->trigger_unknown_frame_.trigger(this->make_touch_frame_());
    }

    void TxUltimateTouch::process_frame_()
    {
      ESP_LOGV(TAG, "Frame opcode=0x%02X length=%u crc=0x%04X",
               this->frame_.opcode, this->frame_.length, this->frame_.crc);

      if (!this->validate_crc_())
      {
        this->send_unknown_frame_();
        return;
      }

      if (this->frame_.opcode != EN_OPT_CODE_TOUCH)
      {
        ESP_LOGD(TAG, "Ignoring unsupported opcode 0x%02X", this->frame_.opcode);
        this->send_unknown_frame_();
        return;
      }

      TouchPoint tp;
      TouchGesture gesture;
      if (this->decode_touch_frame_(&tp, &gesture))
      {
        if (gesture.state != 0)
          this->send_gesture_(gesture);
        this->send_touch_(tp);
      }
      else
      {
        this->send_unknown_frame_();
      }
    }

    void TxUltimateTouch::send_touch_(TouchPoint tp)
    {
      this->trigger_touch_event_.trigger(tp);
      switch (tp.state)
      {
        case EN_TOUCH_STATE_RELEASE:
        {
          const bool chip_long_press = tp.x >= this->long_press_x_offset_;
          const uint32_t duration = this->touch_active_ ? millis() - this->touch_started_at_ : 0;
          this->touch_active_ = false;
          this->send_long_press_time_(duration);

          if (chip_long_press)
          {
            tp.x -= this->long_press_x_offset_;
          }

          if (chip_long_press)
          {
            ESP_LOGV(TAG, "Long touch released (x=%u)", static_cast<unsigned>(tp.x));
            trigger_long_touch_release_.trigger(tp);
          }
          else
          {
            ESP_LOGV(TAG, "Touch released (x=%u)", static_cast<unsigned>(tp.x));
            trigger_release_.trigger(tp);
          }
          break;
        }

        case EN_TOUCH_STATE_PRESS:
          this->touch_active_ = true;
          this->touch_started_at_ = millis();
          ESP_LOGV(TAG, "Touch pressed (x=%u)", static_cast<unsigned>(tp.x));
          trigger_touch_.trigger(tp);
          break;

        case EN_TOUCH_STATE_SWIPE_LEFT:
          this->touch_active_ = false;
          ESP_LOGV(TAG, "Swipe left (x=%u)", static_cast<unsigned>(tp.x));
          trigger_swipe_left_.trigger(tp);
          break;

        case EN_TOUCH_STATE_SWIPE_RIGHT:
          this->touch_active_ = false;
          ESP_LOGV(TAG, "Swipe right (x=%u)", static_cast<unsigned>(tp.x));
          trigger_swipe_right_.trigger(tp);
          break;

        case EN_TOUCH_STATE_MULTI_TOUCH:
          this->touch_active_ = false;
          ESP_LOGV(TAG, "Multi touch released");
          trigger_multi_touch_release_.trigger(tp);
          break;

        default:
          ESP_LOGV(TAG, "Ignoring decoded touch point with state %u", static_cast<unsigned>(tp.state));
          break;
      }
    }

    void TxUltimateTouch::send_long_press_time_(uint32_t duration)
    {
      if (duration == 0)
        return;

      ESP_LOGV(TAG, "Touch duration %u ms", static_cast<unsigned>(duration));
      for (auto *trigger : this->long_press_time_triggers_)
        trigger->process(duration);
    }

    void TxUltimateTouch::send_gesture_(TouchGesture gesture)
    {
      switch (gesture.state)
      {
        case EN_TOUCH_STATE_DASH:
          ESP_LOGV(TAG, "Dash from %u to %u, distance %u",
                   static_cast<unsigned>(gesture.from),
                   static_cast<unsigned>(gesture.to),
                   static_cast<unsigned>(gesture.distance));
          if (gesture.to > gesture.from)
            trigger_dash_right_.trigger(gesture);
          else if (gesture.to < gesture.from)
            trigger_dash_left_.trigger(gesture);
          trigger_dash_.trigger(gesture);
          break;

        case EN_TOUCH_STATE_SWIPE_LEFT:
        case EN_TOUCH_STATE_SWIPE_RIGHT:
          ESP_LOGV(TAG, "Swipe gesture from %u to %u",
                   static_cast<unsigned>(gesture.from), static_cast<unsigned>(gesture.to));
          trigger_swipe_.trigger(gesture);
          break;

        default:
          break;
      }
    }
  } // namespace tx_ultimate_touch
} // namespace esphome
