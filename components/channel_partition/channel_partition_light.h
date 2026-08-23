#pragma once

#include <algorithm>
#include <utility>
#include <vector>

#include "esphome/core/component.h"
#include "esphome/core/color.h"
#include "esphome/components/light/addressable_light.h"

namespace esphome::channel_partition {

enum ChannelPartitionChannel : uint8_t {
  CHANNEL_RED = 0,
  CHANNEL_GREEN = 1,
  CHANNEL_BLUE = 2,
  CHANNEL_WHITE = 3,
};

class ChannelPartitionSegment {
 public:
  ChannelPartitionSegment(light::LightState *src, int32_t src_offset, int32_t src_size,
                          std::vector<uint8_t> channels, bool reversed)
      : src_(static_cast<light::AddressableLight *>(src->get_output())),
        src_offset_(src_offset),
        src_size_(src_size),
        channels_(std::move(channels)),
        reversed_(reversed) {}

  light::AddressableLight *get_src() const { return this->src_; }
  int32_t get_src_offset() const { return this->src_offset_; }
  int32_t get_src_size() const { return this->src_size_; }
  int32_t get_size() const { return this->src_size_ * this->channel_count(); }
  int32_t get_dst_offset() const { return this->dst_offset_; }
  void set_dst_offset(int32_t dst_offset) { this->dst_offset_ = dst_offset; }
  int32_t channel_count() const { return this->channels_.size(); }
  uint8_t channel_at(int32_t index) const { return this->channels_[index]; }
  bool is_reversed() const { return this->reversed_; }

 protected:
  light::AddressableLight *src_;
  int32_t src_offset_;
  int32_t src_size_;
  int32_t dst_offset_{0};
  std::vector<uint8_t> channels_;
  bool reversed_;
};

struct ChannelPartitionPixel {
  uint8_t red{0};
  uint8_t green{0};
  uint8_t blue{0};
  uint8_t white{0};
  uint8_t effect_data{0};
};

class ChannelPartitionLightOutput final : public light::AddressableLight {
 public:
  explicit ChannelPartitionLightOutput(std::vector<ChannelPartitionSegment> segments)
      : segments_(std::move(segments)) {
    int32_t off = 0;
    for (auto &seg : this->segments_) {
      seg.set_dst_offset(off);
      off += seg.get_size();
    }
    this->pixels_.resize(off);
  }

  int32_t size() const override { return this->pixels_.size(); }

  void clear_effect_data() override {
    for (auto &pixel : this->pixels_) {
      pixel.effect_data = 0;
    }
  }

  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::BRIGHTNESS});
    return traits;
  }

  void write_state(light::LightState *state) override {
    for (const auto &seg : this->segments_) {
      for (int32_t dst_off = 0; dst_off < seg.get_size(); dst_off++) {
        const int32_t flat_src_off = seg.is_reversed() ? seg.get_size() - dst_off - 1 : dst_off;
        const int32_t src_led = seg.get_src_offset() + (flat_src_off / seg.channel_count());
        const uint8_t channel = seg.channel_at(flat_src_off % seg.channel_count());
        auto src_view = (*seg.get_src())[src_led];
        src_view.raw_set_color_correction(&this->correction_);
        const uint8_t value = this->mono_value_(this->pixels_[seg.get_dst_offset() + dst_off]);

        switch (channel) {
          case CHANNEL_RED:
            src_view.set_red(value);
            break;
          case CHANNEL_GREEN:
            src_view.set_green(value);
            break;
          case CHANNEL_BLUE:
            src_view.set_blue(value);
            break;
          case CHANNEL_WHITE:
            src_view.set_white(value);
            break;
          default:
            break;
        }
      }
      seg.get_src()->schedule_show();
    }
    this->mark_shown_();
  }

 protected:
  light::ESPColorView get_view_internal(int32_t index) const override {
    auto &pixel = this->pixels_[index];
    return {&pixel.red, &pixel.green, &pixel.blue, &pixel.white, &pixel.effect_data, &this->correction_};
  }

  static uint8_t mono_value_(const ChannelPartitionPixel &pixel) {
    return std::max(std::max(pixel.red, pixel.green), std::max(pixel.blue, pixel.white));
  }

  mutable std::vector<ChannelPartitionPixel> pixels_;
  std::vector<ChannelPartitionSegment> segments_;
};

}  // namespace esphome::channel_partition
