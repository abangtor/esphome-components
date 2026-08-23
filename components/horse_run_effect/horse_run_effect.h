#pragma once

#include "esphome/components/light/addressable_light_effect.h"
#include "esphome/core/automation.h"

namespace esphome::horse_run_effect {

class HorseRunEffect : public light::AddressableLightEffect {
 public:
  explicit HorseRunEffect(const char *name) : light::AddressableLightEffect(name) {}

  void start() override;
  void apply(light::AddressableLight &it, const Color &current_color) override;

  void set_update_interval(uint32_t update_interval) { this->update_interval_ = update_interval; }
  void set_fade_steps(uint8_t fade_steps) { this->fade_steps_ = fade_steps; }
  void set_reverse(bool reverse) { this->reverse_ = reverse; }
  void set_target_black(bool target_black) { this->target_black_ = target_black; }
  Trigger<> *get_finished_trigger() { return &this->finished_trigger_; }

 protected:
  static bool color_matches_(const Color &left, const Color &right);
  static uint8_t blend_channel_(uint8_t current, uint8_t target, uint8_t step, uint8_t fade_steps);
  static Color blend_color_(const Color &current, const Color &target, uint8_t step, uint8_t fade_steps);

  bool apply_direction_(light::AddressableLight &it, const Color &target, bool initial_run);

  uint32_t update_interval_{25};
  uint8_t fade_steps_{5};
  bool reverse_{false};
  bool target_black_{false};
  uint32_t last_run_{0};
  uint32_t pos_{0};
  bool end_{false};
  bool initial_run_{true};
  Trigger<> finished_trigger_;
};

}  // namespace esphome::horse_run_effect
