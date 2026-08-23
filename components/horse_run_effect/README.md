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
            - script.execute: on_horse_run_end_on

      - horse_run:
          name: "Horse Run On Invers"
          update_interval: 5ms
          fade_steps: 5
          reverse: true
          target: current
          on_finished:
            - script.execute: on_horse_run_end_on

      - horse_run:
          name: "Horse Run Off"
          update_interval: 5ms
          fade_steps: 5
          reverse: false
          target: black
          on_finished:
            - script.execute: on_horse_run_end_off

      - horse_run:
          name: "Horse Run Off Invers"
          update_interval: 5ms
          fade_steps: 5
          reverse: true
          target: black
          on_finished:
            - script.execute: on_horse_run_end_off
```

`target` can be `current` or `black`.
