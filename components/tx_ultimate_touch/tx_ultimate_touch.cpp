// tx_ultimate_easy.cpp

#include "esphome/core/log.h"
#include "tx_ultimate_touch.h"
#include <string>

namespace esphome
{
  namespace tx_ultimate_touch
  {
    void TxUltimateTouch::setup()
    {
      ESP_LOGI("log", "%s", "Tx Ultimate Touch is initialized");
    }

    void TxUltimateTouch::loop()
    {
      typeData uart_received_bytes{};
      bool found = false;
      int byte   = -1;
      int i      =  0;

      while( available() )
      {
        byte = read();
        if(byte == EN_MAGIC_WORD_BYTE_1) 
        {
          handle_touch(uart_received_bytes);
          i = 0;
        }
        if(i < EN_UART_RECEIVED_BYTES_SIZE) 
        {
          uart_received_bytes[i] = byte;
          i++;
        }
        if(byte != 0x00)
          found = true;
      }
      if (found) handle_touch(uart_received_bytes);
    }

    void TxUltimateTouch::handle_touch(const typeData& uart_received_bytes)
    {
      ESP_LOGV(TAG, "------------");
      for (int i = 5; i < uart_received_bytes[4] + 5; i++)
        ESP_LOGV(TAG, "UART - Log - Byte[%i]: %02x", i, uart_received_bytes[i]);
      if (is_valid_data(uart_received_bytes))
        send_touch_(get_touch_point(uart_received_bytes));
    }

    void TxUltimateTouch::dump_config()
    {
      ESP_LOGCONFIG(TAG, "Tx Ultimate Touch");
    }

    void TxUltimateTouch::send_touch_(TouchPoint tp)
    {
      this->trigger_touch_event_.trigger(tp);
      switch (tp.state)
      {
        case EN_TOUCH_STATE_RELEASE:
          if (tp.x >= 17) 
          {
            tp.x -= 16;
            ESP_LOGV(TAG, "Long touch - Released (x=%d)", tp.x);
            trigger_long_touch_release_.trigger(tp);
          }
          else
          {
            ESP_LOGV(TAG, "Touch - Released (x=%d)", tp.x);
            trigger_release_.trigger(tp);
          }
          break;

        case EN_TOUCH_STATE_PRESS:
          ESP_LOGV(TAG, "Touch - Pressed (x=%d)", tp.x);
          trigger_touch_.trigger(tp);
          break;

        case EN_TOUCH_STATE_SWIPE_LEFT:
          ESP_LOGV(TAG, "Swipe - Left (x=%d)", tp.x);
          trigger_swipe_left_.trigger(tp);
          break;

        case EN_TOUCH_STATE_SWIPE_RIGHT:
          ESP_LOGV(TAG, "Swipe - Right (x=%d)", tp.x);
          trigger_swipe_right_.trigger(tp);
          break;

        case EN_TOUCH_STATE_MULTI_TOUCH:
          ESP_LOGV(TAG, "Multi touch - Released");
          trigger_multi_touch_release_.trigger(tp);
          break;

        default:
          break;
      }
    }

    bool TxUltimateTouch::is_valid_data(const typeData& uart_received_bytes)
    {
      if(   EN_MAGIC_WORD_BYTE_1 != uart_received_bytes[0]
         || EN_MAGIC_WORD_BYTE_2 != uart_received_bytes[1]
         || EN_DEVICE_VERSION    != uart_received_bytes[2]
         || EN_OPT_CODE          != uart_received_bytes[3])
        return false;

      int state = get_touch_state(uart_received_bytes);
      return(   EN_TOUCH_STATE_PRESS       == state
             || EN_TOUCH_STATE_RELEASE     == state
             || EN_TOUCH_STATE_SWIPE_LEFT  == state
             || EN_TOUCH_STATE_SWIPE_RIGHT == state
             || EN_TOUCH_STATE_MULTI_TOUCH == state) &&
           (uart_received_bytes[6] >= 0 || EN_TOUCH_STATE_MULTI_TOUCH == state);
    }

    int TxUltimateTouch::get_touch_position_x(const typeData& uart_received_bytes)
    {
      switch (uart_received_bytes[4]) 
      {
        case EN_TOUCH_STATE_RELEASE:
        case EN_TOUCH_STATE_MULTI_TOUCH:
        case EN_TOUCH_STATE_SWIPE_LEFT:
        case EN_TOUCH_STATE_SWIPE_RIGHT:
          return uart_received_bytes[5];

        default:
          return uart_received_bytes[6];
      }
    }

    int TxUltimateTouch::get_touch_state(const typeData& uart_received_bytes)
    {
      int state = uart_received_bytes[4];

      if(   EN_TOUCH_STATE_PRESS   == state
         && uart_received_bytes[5] != 0)
        state = EN_TOUCH_STATE_RELEASE;

      if(   EN_TOUCH_STATE_RELEASE     == state
         && EN_TOUCH_STATE_MULTI_TOUCH == uart_received_bytes[5])
        state = EN_TOUCH_STATE_MULTI_TOUCH;

      if (EN_TOUCH_STATE_SWIPE == state)
      {
        state = (EN_TOUCH_STATE_SWIPE_RIGHT == uart_received_bytes[5]) ? EN_TOUCH_STATE_SWIPE_RIGHT : 
                (EN_TOUCH_STATE_SWIPE_LEFT  == uart_received_bytes[5]) ? EN_TOUCH_STATE_SWIPE_LEFT : state;
      }
      return state;
    }

    TouchPoint TxUltimateTouch::get_touch_point(const typeData& uart_received_bytes)
    {
      TouchPoint tp;
      tp.x     = get_touch_position_x(uart_received_bytes);
      tp.state = get_touch_state(uart_received_bytes);
      return tp;
    }

  } // namespace tx_ultimate_touch
} // namespace esphome