Custom Components
=================

* *horse_run_effect*
  Addressable light wipe effect with configurable fade window and finish callback.

* *tx_ultimate_touch*  
  https://github.com/abangtor/sonoff-tx-ultimate-for-esphome  
  https://github.com/SmartHome-yourself/sonoff-tx-ultimate-for-esphome

  Supports controller-reported long touch releases through `on_long_touch_release`
  and separate release-duration bands through `on_long_press_time`:

  ```yaml
  tx_ultimate_touch:
    on_long_press_time:
      - above: 5s
        then:
          - switch.turn_on: relay_1
      - above: 2s
        below: 5s
        then:
          - switch.turn_on: relay_2
  ```
