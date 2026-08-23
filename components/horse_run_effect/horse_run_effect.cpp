#include "horse_run_effect.h"

namespace esphome::horse_run_effect {

void HorseRunEffect::start() {
  this->pos_ = 0;
  this->end_ = false;
  this->initial_run_ = true;
  this->last_run_ = 0;
}

void HorseRunEffect::apply(light::AddressableLight &it, const Color &current_color) {
  const uint32_t now = millis();
  if (!this->initial_run_ && now - this->last_run_ < this->update_interval_)
    return;

  this->last_run_ = now;
  const Color target = this->target_black_ ? Color::BLACK : current_color;
  if (this->apply_direction_(it, target, this->initial_run_))
    this->finished_trigger_.trigger();

  this->initial_run_ = false;
  it.schedule_show();
}

bool HorseRunEffect::color_matches_(const Color &left, const Color &right) {
  return left.r == right.r && left.g == right.g && left.b == right.b && left.w == right.w;
}

uint8_t HorseRunEffect::blend_channel_(const uint8_t current, const uint8_t target, const uint8_t step,
                                       const uint8_t fade_steps) {
  const int16_t diff = static_cast<int16_t>(target) - current;
  return current + ((diff * step) / fade_steps);
}

Color HorseRunEffect::blend_color_(const Color &current, const Color &target, const uint8_t step,
                                   const uint8_t fade_steps) {
  if (step >= fade_steps)
    return target;

  return Color(HorseRunEffect::blend_channel_(current.r, target.r, step, fade_steps),
               HorseRunEffect::blend_channel_(current.g, target.g, step, fade_steps),
               HorseRunEffect::blend_channel_(current.b, target.b, step, fade_steps),
               HorseRunEffect::blend_channel_(current.w, target.w, step, fade_steps));
}

bool HorseRunEffect::apply_direction_(light::AddressableLight &it, const Color &target, const bool initial_run) {
  if (this->end_)
    return false;

  const uint8_t fade_steps = this->fade_steps_ == 0 ? 1 : this->fade_steps_;
  const uint32_t size = it.size();
  if (size == 0) {
    this->end_ = true;
    return true;
  }

  if (initial_run) {
    this->pos_ = 0;
    this->end_ = false;
    bool found = false;
    for (uint32_t i = 0; i < size; i++) {
      const uint32_t led = this->reverse_ ? (size - i - 1) : i;
      if (!this->color_matches_(it[led].get(), target)) {
        this->pos_ = i;
        found = true;
        break;
      }
    }
    if (!found)
      this->pos_ = size + fade_steps - 1;
  }

  for (uint8_t step = 1; step <= fade_steps; step++) {
    if ((this->pos_ + 1) < step)
      continue;

    const uint32_t led = this->pos_ + 1 - step;
    if (led >= size)
      continue;

    const uint32_t target_led = this->reverse_ ? (size - led - 1) : led;
    it[target_led] = this->blend_color_(it[target_led].get(), target, step, fade_steps);
  }

  const uint32_t end_pos = size + fade_steps - 1;
  if (this->pos_ < end_pos)
    this->pos_++;
  if (end_pos <= this->pos_) {
    this->end_ = true;
    it.all() = target;
    return true;
  }
  return false;
}

}  // namespace esphome::horse_run_effect
