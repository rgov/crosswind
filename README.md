# Crosswind - Controller for the Wahoo KICKR Headwind

The [Wahoo KICKR Headwind][headwind] is a $300 "smart fan" for indoor cycling. It has a variable speed setting that can either be controlled manually from a companion smartphone app via Bluetooth, or synchronized to an ANT+ speed or heartrate sensor.

Unfortunately, Wahoo did not make it possible to synchronize the fan speed with Bluetooth-only sensors. This omission is especially exasperating because the smartphone app can connect to such sensors, and hence could easily modulate the fan speed according to their readings.

Crosswind fills this gap. It is a microcontroller firmware for Arduino-compatible boards (like the [M5Stack Atom Lite][atom-lite]) that controls the Headwind's fan speed in sync with a Bluetooth cycling speed sensor.


  [headwind]: https://www.wahoofitness.com/devices/indoor-cycling/accessories/kickr-headwind-buy-us
  [atom-lite]: https://shop.m5stack.com/products/atom-lite-esp32-development-kit
