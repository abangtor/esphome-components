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

- `benq_rs232` - BenQ projector RS232 helpers.
- `channel_partition` - Exposes selected RGB/RGBW channels from an addressable strip as logical monochrome addressable pixels.
- `panasonic_ac` - Panasonic air conditioner climate component.
- `rest_server` - Reduced web server component focused on REST access.
- `tx_ultimate_touch` - Sonoff TX Ultimate touch panel support.
- `web_handler` - Lightweight custom web request handler.
- `wifi_now` - ESP-NOW communication component.
- `wifi` - Local ESPHome WiFi override; include only when a project explicitly needs it.

The repository follows ESPHome's external component layout:

```text
components/<component_name>/
```
