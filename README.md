# ESPHome Components

Custom ESPHome external components used by Torsten's ESPHome configurations.

## Usage

```yaml
external_components:
  - source: github://abangtor/esphome-components@main
    components:
      - channel_partition
```

List every component required by the device under `components`.

## Components

- `channel_partition` - Exposes selected RGB/RGBW channels from an addressable strip as logical monochrome addressable pixels.
- `panasonic_ac` - Panasonic air conditioner climate component.
- `tx_ultimate_touch` - Sonoff TX Ultimate touch panel support.
- `web_handler` - Lightweight custom web request handler.

The repository follows ESPHome's external component layout:

```text
components/<component_name>/
```
