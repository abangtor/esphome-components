# Horse Run Effect

Addressable light effect that advances across the light one logical pixel at a
time while fading a configurable window behind the runner.

```yaml
external_components:
  - source: github://abangtor/esphome-components@main
    components:
      - horse_run_effect

horse_run_effect:

light:
  - platform: ...
    effects:
      - horse_run:
          name: "Horse Run On"
          update_interval: 5ms
          fade_steps: 5
          reverse: false
          target: current
          on_finished:
            - light.turn_on:
                id: my_light
                effect: "None"
```

`target` can be `current` or `black`.
