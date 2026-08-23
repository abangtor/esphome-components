Custom Components
=================

* *panasonic_ac*  
  https://github.com/DomiStyle/esphome-panasonic-ac

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

* *web_handler*  
  https://github.com/ssieb/esphome_components/tree/master/components/web_handler
