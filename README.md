# 基本使用

## 编译

mico make tc1-mqtt@MK3031@moc

## 编译下载并运行

mico make tc1-mqtt@MK3031@moc download JTAG=jlink_swd run

# 功能说明

## 主要修改

已经有人做了TC1的修改固件，但是需要激活码，而且功能较多，代码也已经不开源了，对于我自己的需求来说，不需要通过APP配置的功能（HASS 使用Homekit直接在iPhone中控制就可以），也不需要定时等功能（NodeRed中自己做自动化），因此基于MiCo的基础例程，逐步增加功能的方式编写了这个固件，有需要的可以参考。

## 已经实现的功能

1. MQTT状态同步和开关控制
2. 默认上电所有继电器开启
3. Wifi密码和mqtt配置为默认写死，对应配置在："user_config.h"
4. 基本功能稳定

## 未实现功能：

1. 功率上报
2. 动态配置（计划不使用APP，配网、MQTT服务在webserver中配置）

## 重要参考

项目新建和导入

http://developer.mxchip.com/handbooks/102

拆机和硬件连接

https://github.com/a2633063/zTC1

## homeassistant 配置

> sensor:
>
>   \- platform: mqtt # TC1功率传感器
>
> ​    name: 'tc1_power'
>
> ​    state_topic: 'homeassistant/sensor/tc1/power/state'
>
> ​    unit_of_measurement: 'W'
>
> ​    icon: 'mdi:gauge'
>
> ​    value_template: '{{ value_json.power }}' #{"power":"3.5"}
>
> 
>
> switch:
>
>   \- platform: mqtt
>
> ​    name: 'tc1_1'
>
> ​    state_topic: 'homeassistant/switch/tc1/state'
>
> ​    command_topic: 'device/tc1/set'
>
> ​    payload_on: 'S1_ON'
>
> ​    payload_off: 'S1_OFF'
>
>   \- platform: mqtt
>
> ​    name: 'tc1_2'
>
> ​    state_topic: 'homeassistant/switch/tc1/state'
>
> ​    command_topic: 'device/tc1/set'
>
> ​    payload_on: 'S2_ON'
>
> ​    payload_off: 'S2_OFF'
>
>   \- platform: mqtt
>
> ​    name: 'tc1_3'
>
> ​    state_topic: 'homeassistant/switch/tc1/state'
>
> ​    command_topic: 'device/tc1/set'
>
> ​    payload_on: 'S3_ON'
>
> ​    payload_off: 'S3_OFF'
>
>   \- platform: mqtt
>
> ​    name: 'tc1_4'
>
> ​    state_topic: 'homeassistant/switch/tc1/state'
>
> ​    command_topic: 'device/tc1/set'
>
> ​    payload_on: 'S4_ON'
>
> ​    payload_off: 'S4_OFF'
>
>   \- platform: mqtt
>
> ​    name: 'tc1_5'
>
> ​    state_topic: 'homeassistant/switch/tc1/state'
>
> ​    command_topic: 'device/tc1/set'
>
> ​    payload_on: 'S5_ON'
>
> ​    payload_off: 'S5_OFF'
>
>   \- platform: mqtt
>
> ​    name: 'tc1_6'
>
> ​    state_topic: 'homeassistant/switch/tc1/state'
>
> ​    command_topic: 'device/tc1/set'
>
> ​    payload_on: 'S6_ON'
>
> ​    payload_off: 'S6_OFF'