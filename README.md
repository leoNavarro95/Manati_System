# Manati System
**Manati System** is an Automatic Water Level System based on Esp32 MCU

## Features
This **system** is designed to ensure that your water tanks are never empty. The ultrasonic sensor is used to measure the water level in a tank. On the other hand, a flow meter is used to detect that the water pump is working correctly. The ESP32 microcontroller is used as the brain of the system. This MCU has WIFI connectivity and this makes it possible to implement an [embedded web application](https://github.com/leoNavarro95/Esp32-Web-Tank-System "Esp32-Web-Tank-System"). 

## How does the system work?

The system will measure the level of the tank and from two preset parameters for the high level and the low level, it will make the decision to turn the pump on or off. In other words, if, for example, 40% of the total tank capacity is taken as low level and 90% as high level, the system will command the pump to turn on (through the relay) when the level is lower than 40% until it reaches 90% where it will be turned off. Parallel to this process, while the pump is on, the system will check that there is flow at its outlet (that the pump is “pulling” water) and in case there is no flow at the outlet of the pump, it will send it to stop, this is to prevent it from burning, as a pump that is turned on without pumping water for a relatively short time burns out, as it relies on water for its own lubrication and cooling.

In addition to all this, the LCD screen will show the data of the% of water in the tank all the time, the water flow and will notify in case of an error such as when the pump does not pump. As a support to the screen, the system will have a buzzer in the form of an audible signal, which will produce sounds when events such as: pump start, pump stop and error occur.

___

## Tutorial:
* [Tank water level controller with ultrasonic sensor and Esp32](https://www.sysadminsdecuba.com/2020/03/controlador-de-nivel-de-agua-en-tanque-con-sensor-ultrasonico-y-arduino/ "Tutorial") __Sorry It's in Spanish__

___

## List of materials for the project:
* MCU dev board: DOIT ESP32 DevKit V1
![ESP32 Logo](https://i0.wp.com/www.sysadminsdecuba.com/wp-content/uploads/2020/03/esp32.png)

* Ultrasonic Waterproof Sensor:  JSN SR04T 
![Ultrasonic Sensor](https://i1.wp.com/www.sysadminsdecuba.com/wp-content/uploads/2020/03/sensor.png)

* One channel relay module
![Relay Module](https://i2.wp.com/www.sysadminsdecuba.com/wp-content/uploads/2020/03/rele.png?resize=308%2C278&ssl=1)

* Flow meter: Model Yf-s201
![flow meter](https://i1.wp.com/www.sysadminsdecuba.com/wp-content/uploads/2020/03/flujometro.png?resize=314%2C226&ssl=1)

* LCD screen 16x2
![LCD screen](https://i1.wp.com/www.sysadminsdecuba.com/wp-content/uploads/2020/03/lcd.png?w=338&ssl=1)

* Buzzer
![buzzer](https://i0.wp.com/www.sysadminsdecuba.com/wp-content/uploads/2020/03/buzzer.png?w=300&ssl=1)


> This file is, currently, in development
