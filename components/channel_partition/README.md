# Channel Partition Light

`channel_partition` wraps one or more addressable lights and exposes selected
source color channels as a logical monochrome addressable strip.

This is useful for WS2811 strips where each chip drives three separate white LED
segments instead of one RGB LED. The source light can be any ESPHome
addressable light implementation; this component only reads and writes its
addressable light buffer.

```yaml
external_components:
  - source:
      type: local
      path: components
    components:
      - channel_partition

light:
  - platform: neopixelbus
    id: parent_leds
    type: RGB
    variant: WS2811
    pin: GPIO2
    num_leds: 41
    internal: true

  - platform: channel_partition
    id: mono_leds
    name: "Mono LEDs"
    segments:
      - id: parent_leds
        from: 0
        to: 40
        channels: RGB
```

The example above exposes `41 * 3 = 123` logical pixels:

- logical pixel 0 -> source LED 0 red channel
- logical pixel 1 -> source LED 0 green channel
- logical pixel 2 -> source LED 0 blue channel
- logical pixel 3 -> source LED 1 red channel

Set `reversed: true` on a segment to reverse that flattened logical order.
`channels` may contain `R`, `G`, `B`, and `W` once each, in the physical order
you want exposed.

Use `num_leds` instead of `to` when a package template already receives a count:

```yaml
segments:
  - id: parent_leds
    from: 0
    num_leds: 41
    channels: RGB
```
