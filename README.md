#tc1-mqtt

mico make tc1-mqtt@MK3031@moc
mico make tc1-mqtt@MK3031@moc download JTAG=jlink_swd run

2019.10.4
已经实现的功能：MQTT状态同步和开关控制
默认上电所有继电器开启

Wifi密码和mqtt配置为默认写死
对应配置在：
"user_config.h"

未实现功能：
功率上报
动态配置




sensor:
  - platform: mqtt # TC1功率传感器
    name: 'tc1_power'
    state_topic: 'homeassistant/sensor/tc1/power/state'
    unit_of_measurement: 'W'
    icon: 'mdi:gauge'
    value_template: '{{ value_json.power }}' #{"power":"3.5"}

switch:
  - platform: mqtt
    name: 'tc1_1'
    state_topic: 'homeassistant/switch/tc1/state'
    command_topic: 'device/tc1/set'
    payload_on: 'S1_ON'
    payload_off: 'S1_OFF'
  - platform: mqtt
    name: 'tc1_2'
    state_topic: 'homeassistant/switch/tc1/state'
    command_topic: 'device/tc1/set'
    payload_on: 'S2_ON'
    payload_off: 'S2_OFF'
  - platform: mqtt
    name: 'tc1_3'
    state_topic: 'homeassistant/switch/tc1/state'
    command_topic: 'device/tc1/set'
    payload_on: 'S3_ON'
    payload_off: 'S3_OFF'
  - platform: mqtt
    name: 'tc1_4'
    state_topic: 'homeassistant/switch/tc1/state'
    command_topic: 'device/tc1/set'
    payload_on: 'S4_ON'
    payload_off: 'S4_OFF'
  - platform: mqtt
    name: 'tc1_5'
    state_topic: 'homeassistant/switch/tc1/state'
    command_topic: 'device/tc1/set'
    payload_on: 'S5_ON'
    payload_off: 'S5_OFF'
  - platform: mqtt
    name: 'tc1_6'
    state_topic: 'homeassistant/switch/tc1/state'
    command_topic: 'device/tc1/set'
    payload_on: 'S6_ON'
    payload_off: 'S6_OFF'