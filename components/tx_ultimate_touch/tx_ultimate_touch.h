// tx_ultimate_touch.h

#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/uart/uart_component.h"
#include <array>
#include <cstdint>
#include <limits>
#include <vector>

namespace esphome
{
  namespace tx_ultimate_touch
  {
    // Touch State Constants
    enum
    {
      EN_TOUCH_STATE_RELEASE     = 0x01,
      EN_TOUCH_STATE_PRESS       = 0x02,
      EN_TOUCH_STATE_SWIPE       = 0x03,
      EN_TOUCH_STATE_MULTI_TOUCH = 0x0B,
      EN_TOUCH_STATE_SWIPE_RIGHT = 0x0C,
      EN_TOUCH_STATE_SWIPE_LEFT  = 0x0D,
      EN_TOUCH_STATE_DASH        = 0x0E
    };

    // UART Constants
    enum
    {
      EN_MAGIC_WORD_BYTE_1        = 0xAA,
      EN_MAGIC_WORD_BYTE_2        = 0x55,
      EN_DEVICE_VERSION           = 0x01,
      EN_OPT_CODE_TOUCH           = 0x02,
      EN_MAX_DATA_LENGTH          =   64
    };

    struct TouchPoint
    {
      uint8_t x = 0;
      uint8_t state = 0;
    };

    struct TouchGesture
    {
      uint8_t from = 0;
      uint8_t to = 0;
      uint8_t distance = 0;
      uint8_t raw_mid = 0;
      uint8_t state = 0;
    };

    struct TouchFrame
    {
      uint8_t opcode = 0;
      uint8_t length = 0;
      uint8_t data0 = 0;
      uint8_t data1 = 0;
      uint8_t data2 = 0;
      uint8_t data3 = 0;
      uint8_t data4 = 0;
      uint8_t data5 = 0;
      uint8_t data6 = 0;
      uint8_t data7 = 0;
      uint16_t crc = 0;
    };

    class LongPressTimeTrigger;

    class TxUltimateTouch :
      public uart::UARTDevice,
      public Component
    {
    public:
      Trigger<TouchPoint>* get_trigger_touch_event()         { return &trigger_touch_event_; }
      Trigger<TouchPoint>* get_trigger_touch()               { return &trigger_touch_; }
      Trigger<TouchPoint>* get_trigger_release()             { return &trigger_release_; }
      Trigger<TouchGesture>* get_trigger_dash()              { return &trigger_dash_; }
      Trigger<TouchGesture>* get_trigger_dash_left()         { return &trigger_dash_left_; }
      Trigger<TouchGesture>* get_trigger_dash_right()        { return &trigger_dash_right_; }
      Trigger<TouchGesture>* get_trigger_swipe()             { return &trigger_swipe_; }
      Trigger<TouchPoint>* get_trigger_swipe_left()          { return &trigger_swipe_left_; }
      Trigger<TouchPoint>* get_trigger_swipe_right()         { return &trigger_swipe_right_; }
      Trigger<TouchPoint>* get_trigger_multi_touch_release() { return &trigger_multi_touch_release_; }
      Trigger<TouchPoint>* get_trigger_long_touch_release()  { return &trigger_long_touch_release_; }
      Trigger<TouchFrame>* get_trigger_unknown_frame()       { return &trigger_unknown_frame_; }
      void set_long_press_x_offset(uint8_t offset) { this->long_press_x_offset_ = offset; }
      void set_validate_crc(bool validate_crc) { this->validate_crc_enabled_ = validate_crc; }
      void add_long_press_time_trigger(LongPressTimeTrigger *trigger) { this->long_press_time_triggers_.push_back(trigger); }

      void setup() override;
      void loop() override;
      void dump_config() override;

    protected:
      enum class ParserState : uint8_t
      {
        WAIT_HEADER_1 = 0,
        WAIT_HEADER_2,
        VERSION,
        OPCODE,
        LENGTH,
        DATA,
        CRC_HIGH,
        CRC_LOW,
      };

      struct Frame
      {
        uint8_t version = 0;
        uint8_t opcode = 0;
        uint8_t length = 0;
        std::array<uint8_t, EN_MAX_DATA_LENGTH> data{};
        uint16_t crc = 0;
      };

      bool parse_byte_(uint8_t byte);
      void reset_parser_();
      void process_frame_();
      bool validate_crc_() const;
      uint16_t calculate_crc_() const;
      bool decode_touch_frame_(TouchPoint *tp, TouchGesture *gesture) const;
      bool is_valid_position_(uint8_t x) const;
      TouchFrame make_touch_frame_() const;
      void send_unknown_frame_();
      void send_touch_(TouchPoint tp);
      void send_gesture_(TouchGesture gesture);
      void send_long_press_time_(uint32_t duration);

      ParserState parser_state_{ParserState::WAIT_HEADER_1};
      Frame frame_{};
      uint8_t data_read_{0};
      uint8_t long_press_x_offset_{16};
      uint32_t touch_started_at_{0};
      bool touch_active_{false};
      bool validate_crc_enabled_{true};
      std::vector<LongPressTimeTrigger *> long_press_time_triggers_;

      Trigger<TouchPoint> trigger_touch_event_;
      Trigger<TouchPoint> trigger_touch_;
      Trigger<TouchPoint> trigger_release_;
      Trigger<TouchGesture> trigger_dash_;
      Trigger<TouchGesture> trigger_dash_left_;
      Trigger<TouchGesture> trigger_dash_right_;
      Trigger<TouchGesture> trigger_swipe_;
      Trigger<TouchPoint> trigger_swipe_left_;
      Trigger<TouchPoint> trigger_swipe_right_;
      Trigger<TouchPoint> trigger_multi_touch_release_;
      Trigger<TouchPoint> trigger_long_touch_release_;
      Trigger<TouchFrame> trigger_unknown_frame_;

    }; // class TxUltimateTouch

    class LongPressTimeTrigger : public Trigger<uint32_t>
    {
    public:
      explicit LongPressTimeTrigger(TxUltimateTouch *parent) { parent->add_long_press_time_trigger(this); }

      void set_min(uint32_t min) { this->min_ = min; }
      void set_max(uint32_t max) { this->max_ = max; }

      void process(uint32_t duration)
      {
        if (duration > this->min_ && duration < this->max_)
          this->trigger(duration);
      }

    protected:
      uint32_t min_{0};
      uint32_t max_{std::numeric_limits<uint32_t>::max()};
    };
  } // namespace tx_ultimate_touch
} // namespace esphome
