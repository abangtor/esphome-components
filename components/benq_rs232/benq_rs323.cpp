#include "esphome/core/log.h"
#include "tx_ultimate_touch.h"

namespace esphome
{
    namespace benq_rs232
    {
        static const char* TAG = "benq_rs232";

        void BenqRs232::setup()
        {
            ESP_LOGI("log", "%s", "BenQ RS232 is initialized");
        }

        void BenqRs232::loop()
        {
            bool found = false;

            int bytes[15] = {};
            int byte = -1;
            int i = 0;

            while (this->available())
            {
                byte = this->read();
                if( byte == '\r' )
                {
                    handle_touch(bytes);
                    i = 0;
                }

                bytes[i] = byte;

                i++;

                if( byte != 0 )
                {
                    found = true;
                }
            };

            if (found)
            {
                handle_touch(bytes);
            }
        }

        void BenqRs232::handle_touch(int bytes[])
        {
            ESP_LOGV("UART-Log", "------------");
            for (int i = 0; i < 15; i++)
            {
                ESP_LOGV("UART-Log", "%i", bytes[i]);
            }

            if (is_valid_data(bytes))
            {
                send_touch_(get_touch_point(bytes));
            }
        }

        void BenqRs232::dump_config()
        {
            ESP_LOGCONFIG(TAG, "BenQ RS232");
        }

        void BenqRs232::send_touch_(TouchPoint tp)
        {
            switch (tp.state)
            {
            case TOUCH_STATE_RELEASE:
                if (tp.x >= 17)
                {
                    tp.x = tp.x - 16;
                    ESP_LOGD(TAG, "Long Press Release (x=%d)", tp.x);
                    this->long_touch_release_trigger_.trigger(tp);
                }
                else
                {
                    ESP_LOGD(TAG, "Release (x=%d)", tp.x);
                    this->release_trigger_.trigger(tp);
                }
                break;

            case TOUCH_STATE_PRESS:
                ESP_LOGD(TAG, "Press (x=%d)", tp.x);
                this->touch_trigger_.trigger(tp);
                break;

            case TOUCH_STATE_SWIPE_LEFT:
                ESP_LOGD(TAG, "Swipe Left (x=%d)", tp.x);
                this->swipe_trigger_left_.trigger(tp);
                break;

            case TOUCH_STATE_SWIPE_RIGHT:
                ESP_LOGD(TAG, "Swipe Right (x=%d)", tp.x);
                this->swipe_trigger_right_.trigger(tp);
                break;

            case TOUCH_STATE_ALL_FIELDS:
                ESP_LOGD(TAG, "Full Touch Release");
                this->full_touch_release_trigger_.trigger(tp);
                break;

            default:
                break;
            }
        }

        bool BenqRs232::is_valid_data(int bytes[])
        {
            bool valid = true;

            if (!(bytes[0] == 170 && bytes[1] == 85 && bytes[2] == 1 && bytes[3] == 2))
            {
                return false;
            }

            int state = get_touch_state(bytes);
            if (state != TOUCH_STATE_PRESS &&
                state != TOUCH_STATE_RELEASE &&
                state != TOUCH_STATE_SWIPE_LEFT &&
                state != TOUCH_STATE_SWIPE_RIGHT &&
                state != TOUCH_STATE_ALL_FIELDS)
            {
                return false;
            }

            if (bytes[6] < 0 && state != TOUCH_STATE_ALL_FIELDS)
            {
                return false;
            }

            return true;
        }

        int BenqRs232::get_x_touch_position(int bytes[])
        {
            int state = bytes[4];
            switch (state)
            {
            case TOUCH_STATE_RELEASE:
                return bytes[5];
                break;

            case TOUCH_STATE_ALL_FIELDS:
                return bytes[5];
                break;

            case TOUCH_STATE_SWIPE_LEFT:
                return bytes[5];
                break;

            case TOUCH_STATE_SWIPE_RIGHT:
                return bytes[5];
                break;

            default:
                return bytes[6];
                break;
            }
        }

        int BenqRs232::get_touch_state(int bytes[])
        {
            int state = bytes[4];

            if (state == TOUCH_STATE_PRESS && bytes[5] != 0)
            {
                state = TOUCH_STATE_RELEASE;
            }

            if (state == TOUCH_STATE_RELEASE && bytes[5] == TOUCH_STATE_ALL_FIELDS)
            {
                state = TOUCH_STATE_ALL_FIELDS;
            }

            if (state == TOUCH_STATE_SWIPE)
            {
                if (bytes[5] == TOUCH_STATE_SWIPE_RIGHT)
                {
                    state = TOUCH_STATE_SWIPE_RIGHT;
                }
                else if (bytes[5] == TOUCH_STATE_SWIPE_LEFT)
                {
                    state = TOUCH_STATE_SWIPE_LEFT;
                }
            }

            return state;
        }

        TouchPoint BenqRs232::get_touch_point(int bytes[])
        {
            TouchPoint tp;

            tp.x = get_x_touch_position(bytes);
            tp.state = get_touch_state(bytes);

            return tp;
        }

    }
}