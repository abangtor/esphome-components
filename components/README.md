Custom Components
=================

* *benq_rs232*  
  Not finished

* *panasonic_ac*  
  https://github.com/DomiStyle/esphome-panasonic-ac

* *rest_server*  
  web_server component with only restfull API enabled

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

* *wifi*  
  ESPHome wifi component copy

* *wifi_now*  
  https://github.com/motwok/esphome/tree/wifi_now_component/esphome/components/wifi_now
  https://deploy-preview-775--esphome.netlify.app/components/wifi_now.html
