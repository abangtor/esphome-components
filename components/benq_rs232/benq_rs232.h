#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/uart/uart_component.h"

namespace esphome
{
namespace benq_rs232
{

struct Reply
{
  std::string reply = std::string();
};

class BenqRs232 :
  public uart::UARTDevice,
  public Component
{
  protected:
    Trigger<Reply> touch_reply_;

  public:
    Trigger<Reply>* get_reply_trigger() { return &this->touch_trigger_; }
    
    void set_uart_component(esphome::uart::UARTComponent *uart_component)
    {
        this->set_uart_parent(uart_component);
    }

    void setup() override;
    void loop() override;
    void dump_config() override;

  protected:
    void send_touch_(TouchPoint tp);
    void handle_touch(int bytes[]);

    TouchPoint get_touch_point(int bytes[]);
    bool is_valid_data(int bytes[]);
    int get_x_touch_position(int bytes[]);
    int get_touch_state(int bytes[]);

};

template<typename... Ts> class BenqSendCommand : 
  public Action<Ts...>,
  public Parented<UARTComponent>
{
  protected:
    bool                                       is_static_  {false};
    std::function<std::vector<uint8_t>(Ts...)> cmd_func_   {};
    std::vector<uint8_t>                       cmd_static_ {};
    
  public:
    void set_cmd_template(std::function<std::vector<uint8_t>(Ts...)> func)
    {
      this->cmd_func_  = func;
      this->is_static_ = false;
    }
    void set_cmd_static(const std::vector<uint8_t> &cmd) 
    {
      this->cmd_static_ = cmd;
      this->is_static_  = true;
    }

    void play(Ts... x) override 
    {
      if( this->is_static_ )
      {
        this->parent_->write_array(this->cmd_static_);
      }
      else 
      {
        auto val = this->cmd_func_(x...);
        this->parent_->write_array(val);
      }
    }

};

}
}