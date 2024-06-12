# ESP32-Thermostat
ESP32 Thermostat

An example of how to create a smart thermostat using WeAct ESP32, DHT11 sensor and a relay.

Hardware Components
WeAct ESP32 Board
DHT11 Temperature and Humidity Sensor
Relay Module
Jumper Wires
Power Supply (5V for ESP32 and relay)
Not necessary (some DHT11 come with everything): Resistor (4.7kΩ or 10kΩ for the DHT11 pull-up resistor)

Circuit Assembly
ESP32 to DHT11 Connections:

VCC (DHT11) to 3.3V (ESP32)
GND (DHT11) to GND (ESP32)
Data (DHT11) to GPIO 2 (ESP32) (with a pull-up resistor between data and VCC)

ESP32 to Relay Connections:

VCC (Relay) to 5V (ESP32)
GND (Relay) to GND (ESP32)
IN (Relay) to GPIO 3 (ESP32)

Software Setup
Arduino IDE Configuration:

Install the ESP32 board in Arduino IDE.
Install the DHT sensor library and Adafruit Unified Sensor library.
