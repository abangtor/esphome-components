// tx_ultimate_easy.h

#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/uart/uart_component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/light/addressable_light.h"
#include <array>

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
      EN_TOUCH_STATE_SWIPE_LEFT  = 0x0D
    };

    // UART Constants
    enum
    {
      EN_UART_RECEIVED_BYTES_SIZE =   15,
      EN_MAGIC_WORD_BYTE_1        = 0xAA,
      EN_MAGIC_WORD_BYTE_2        = 0x55,
      EN_DEVICE_VERSION           = 0x01,
      EN_OPT_CODE                 = 0x02
    };

    struct TouchPoint
    {
      int8_t x   = -1;
      int8_t state = -1;
    };

    static const char* TAG = "tx_ultimate_touch";

    typedef std::array<int, EN_UART_RECEIVED_BYTES_SIZE> typeData;

    class TxUltimateTouch :
      public uart::UARTDevice,
      public Component
    {
    public:
      Trigger<TouchPoint>* get_trigger_touch_event()         { return &trigger_touch_event_; }
      Trigger<TouchPoint>* get_trigger_touch()               { return &trigger_touch_; }
      Trigger<TouchPoint>* get_trigger_release()             { return &trigger_release_; }
      Trigger<TouchPoint>* get_trigger_swipe_left()          { return &trigger_swipe_left_; }
      Trigger<TouchPoint>* get_trigger_swipe_right()         { return &trigger_swipe_right_; }
      Trigger<TouchPoint>* get_trigger_multi_touch_release() { return &trigger_multi_touch_release_; }
      Trigger<TouchPoint>* get_trigger_long_touch_release()  { return &trigger_long_touch_release_; }

      void set_uart_component(esphome::uart::UARTComponent* uart_component)
      {
        set_uart_parent(uart_component);
      }

      void setup() override;
      void loop() override;
      void dump_config() override;

    protected:
      void send_touch_(TouchPoint tp);
      void handle_touch(const typeData& bytes);

      TouchPoint get_touch_point(const typeData& bytes);
      bool is_valid_data        (const typeData& bytes);
      int get_touch_position_x  (const typeData& bytes);
      int get_touch_state       (const typeData& bytes);

      Trigger<TouchPoint> trigger_touch_event_;
      Trigger<TouchPoint> trigger_touch_;
      Trigger<TouchPoint> trigger_release_;
      Trigger<TouchPoint> trigger_swipe_left_;
      Trigger<TouchPoint> trigger_swipe_right_;
      Trigger<TouchPoint> trigger_multi_touch_release_;
      Trigger<TouchPoint> trigger_long_touch_release_;

    }; // class TxUltimateTouch
  } // namespace tx_ultimate_touch
} // namespace esphome