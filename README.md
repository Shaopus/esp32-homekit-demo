# esp32-homekit-demo

[esp-homekit](https://github.com/maximkulkin/esp-homekit) 是 `Apple HomeKit` 配件服务器库，可以在 `esp8266` 和 `esp32` 上使用，以下是 [esp-homekit](https://github.com/maximkulkin/esp-homekit)  在 `esp32` 上的各个demo，由 [esp-homekit-demo](https://github.com/maximkulkin/esp-homekit-demo) 修改而来。



## 移植

- 移植步骤请参考 [esp32-homekit-led](https://github.com/Shaopus/esp32-homekit-led) 



## 例子

- [01-accessary_names](./examples/01-accessory_names)
- [02-led](./examples/02-led)



## 参考

This project uses the Apple HomeKit accessory server library [esp-homekit](https://github.com/maximkulkin/esp-homekit) from [@MaximKulkin](https://github.com/maximkulkin) for [esp-idf](https://github.com/espressif/esp-idf).

Although already forbidden by the sources and subsequent licensing, it is not allowed to use or distribute this software for a commercial purpose.



[![img](https://camo.githubusercontent.com/86810bc4e8b59fd6be0583d910d7d1c0b4ede582/68747470733a2f2f66726565706e67696d672e636f6d2f7468756d622f6170706c655f6c6f676f2f32353336362d372d6170706c652d6c6f676f2d66696c652e706e67)](https://camo.githubusercontent.com/86810bc4e8b59fd6be0583d910d7d1c0b4ede582/68747470733a2f2f66726565706e67696d672e636f6d2f7468756d622f6170706c655f6c6f676f2f32353336362d372d6170706c652d6c6f676f2d66696c652e706e67)

###### HomeKit Accessory Protocol (HAP) is Apple’s proprietary protocol that enables third-party accessories in the home (e.g., lights, thermostats and door locks) and Apple products to communicate with each other. HAP supports two transports, IP and Bluetooth LE. The information provided in the HomeKit Accessory Protocol Specification (Non-Commercial Version) describes how to implement HAP in an accessory that you create for non-commercial use and that will not be distributed or sold.

###### The HomeKit Accessory Protocol Specification (Non-Commercial Version) can be downloaded from the [HomeKit Apple Developer page.](https://developer.apple.com/homekit/)

###### Copyright © 2019 Apple Inc. All rights reserved.