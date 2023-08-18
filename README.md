# E-Paper-Display-for-Kostal-Solar-Inverter

I developed a small system to show basic information of the Kostal Pico IQ Solar Inverter. I use an ESP32 and a Waveshare 2.7 inch e-paper HAT b/w display. 

You have to add your Wifi information and the local IP address and port of your Kostal Inverter in the respective lines of code.
## Wiring
Busy 27

RST 33

DC 25

CS 26

CLK 18

BIN 23

GND GND

VCC 3.3V

## Dependencies
I use the library [GxEPD by Jean-Marc Zingg](https://github.com/ZinggJM/GxEPD).
